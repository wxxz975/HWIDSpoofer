#include "Nic.h"

#include "Utils.h"
#include "Logger.hpp"
#include "Shared.h"

namespace Nic
{
	static DWORD SEED = 0;

	PDRIVER_DISPATCH NsiControlOriginal = 0;

	struct {
		DWORD Length;
		NIC_DRIVER Drivers[0xFF];
	} NICs = { 0 };



	

	
	


	NTSTATUS NsiControl(PDEVICE_OBJECT device, PIRP irp) {
		PIO_STACK_LOCATION ioc = IoGetCurrentIrpStackLocation(irp);
		switch (ioc->Parameters.DeviceIoControl.IoControlCode) {
		case IOCTL_NSI_PROXY_ARP: {
			DWORD length = ioc->Parameters.DeviceIoControl.OutputBufferLength;
			NTSTATUS ret = NsiControlOriginal(device, irp);

			PNSI_PARAMS params = (PNSI_PARAMS)irp->UserBuffer;
			if (params && NSI_PARAMS_ARP == params->Type) {
				memset(irp->UserBuffer, 0, length);

				log("handled ARP table\n");
			}

			return ret;
		}
		}

		return NsiControlOriginal(device, irp);
	}







	/**** NIC ****/

	NTSTATUS NICIoc(PDEVICE_OBJECT device, PIRP irp, PVOID context) {
		if (context) {
			IOC_REQUEST request = *(PIOC_REQUEST)context;
			ExFreePool(context);

			if (irp->MdlAddress) {
				Utils::SpoofBuffer(SEED, (PBYTE)MmGetSystemAddressForMdl(irp->MdlAddress), 6);

				log("handled NICIoc\n");
			}

			if (request.OldRoutine && irp->StackCount > 1) {
				return request.OldRoutine(device, irp, request.OldContext);
			}
		}

		return STATUS_SUCCESS;
	}
	PVOID SafeCopy(PVOID src, DWORD size) {
		PCHAR buffer = (PCHAR)ExAllocatePool(NonPagedPool, size);
		if (buffer) {
			MM_COPY_ADDRESS addr = { 0 };
			addr.VirtualAddress = src;

			SIZE_T read = 0;
			if (NT_SUCCESS(MmCopyMemory(buffer, addr, size, MM_COPY_MEMORY_VIRTUAL, &read)) && read == size) {
				return buffer;
			}

			ExFreePool(buffer);
		}
		else {
			err("! failed to allocate pool of size %d !\n", size);
		}

		return 0;
	}
	PWCHAR TrimGUID(PWCHAR guid, DWORD max) {
		DWORD i = 0;
		PWCHAR start = guid;

		--max;
		for (; i < max && *start != L'{'; ++i, ++start);
		for (; i < max && guid[i++] != L'}';);

		guid[i] = 0;
		return start;
	}




	VOID ChangeIoc(PIO_STACK_LOCATION ioc, PIRP irp, PIO_COMPLETION_ROUTINE routine) {
		PIOC_REQUEST request = (PIOC_REQUEST)ExAllocatePool(NonPagedPool, sizeof(IOC_REQUEST));
		if (!request) {
			err("! failed to allocate IOC_REQUEST !\n");
			return;
		}

		request->Buffer = irp->AssociatedIrp.SystemBuffer;
		request->BufferLength = ioc->Parameters.DeviceIoControl.OutputBufferLength;
		request->OldContext = ioc->Context;
		request->OldRoutine = ioc->CompletionRoutine;

		ioc->Control = SL_INVOKE_ON_SUCCESS;
		ioc->Context = request;
		ioc->CompletionRoutine = routine;
	}

	NTSTATUS NICControl(PDEVICE_OBJECT device, PIRP irp) {
		for (DWORD i = 0; i < NICs.Length; ++i) {
			PNIC_DRIVER driver = &NICs.Drivers[i];

			if (driver->Original && driver->DriverObject == device->DriverObject) {
				PIO_STACK_LOCATION ioc = IoGetCurrentIrpStackLocation(irp);
				switch (ioc->Parameters.DeviceIoControl.IoControlCode) {
				case IOCTL_NDIS_QUERY_GLOBAL_STATS: {
					switch (*(PDWORD)irp->AssociatedIrp.SystemBuffer) {
					case OID_802_3_PERMANENT_ADDRESS:
					case OID_802_3_CURRENT_ADDRESS:
					case OID_802_5_PERMANENT_ADDRESS:
					case OID_802_5_CURRENT_ADDRESS:
						ChangeIoc(ioc, irp, NICIoc);
						break;
					}

					break;
				}
				}

				return driver->Original(device, irp);
			}
		}

		return STATUS_SUCCESS;
	}
	/// <summary>
	/// 将地址转化为实际地址
	/// </summary>
	/// <param name="Instruction">指令开始的地址</param>
	/// <param name="OffsetOffset">操作指令符占用几个字节</param>
	/// <param name="InstructionSize">整个操作占用几个字节</param>
	/// <returns></returns>
	PVOID ResolveRelativeAddress(_In_ PVOID Instruction, _In_ ULONG OffsetOffset, _In_ ULONG InstructionSize)
	{
		ULONG_PTR Instr = (ULONG_PTR)Instruction;
		LONG RipOffset = 0;

		RipOffset = *(PULONG)((PBYTE)Instr + OffsetOffset);

		PVOID ResolvedAddr = (PVOID)((PBYTE)Instr + InstructionSize + RipOffset);

		return ResolvedAddr;
	}

#define _DBG
#define __HOOK_DISPATCH__


	NTSTATUS SpoofNIC() {
		UNICODE_STRING pDriverName = { 0 };
		RtlInitUnicodeString(&pDriverName, L"\\Driver\\nsiproxy");

		//SwapControl(&pDriverName, NsiControl, NsiControlOriginal);


		PVOID base = Utils::GetModuleBase("ndis.sys");
		if (!base) {
			err("Failed to get ndis.sys !\n");
			return STATUS_SUCCESS;
		}

		log("ndis.sys Base:0x%llp\n", base);


		/*
			build number:22h2
			// 上一层 ndisReferenceFilterByHandle + 0x2b
			ndis!ndisReferenceFilterByHandle+0x2b ->ndis!ndisGlobalFilterList

			fffff805`650194b3 488bf9          mov     rdi,rcx
			fffff805`650194b6 33db            xor     ebx,ebx
			fffff805`650194b8 488d0d09b60700  lea     rcx,[ndis!ndisGlobalFilterListLock (fffff805`65094ac8)]
			fffff805`650194bf 4c8b1522a40800  mov     r10,qword ptr [ndis!_imp_KeAcquireSpinLockRaiseToDpc (fffff805`650a38e8)]
			fffff805`650194c6 e8054440fa      call    nt!KeAcquireSpinLockRaiseToDpc (fffff805`5f41d8d0)
			fffff805`650194cb 408af0          mov     sil,al
			fffff805`650194ce 488b0543b00700  mov     rax,qword ptr [ndis!ndisGlobalFilterList (fffff805`65094518)]
		*/
		PVOID ndisGlobalFilterListPattern = Utils::FindPatternImage(base,
			"\x48\x8b\xf9\x33\xdb\x48\x8d\x0d\x00\x00\x00\x00\x4c\x8b\x15\x00\x00\x00\x00\xe8\x00\x00\x00\x00\x40\x8a\xf0", 
			"xxxxxxxx????xxx????x????xxx");
		
		if (!ndisGlobalFilterListPattern)
		{
			err("Failed to find the pattern ndisGlobalFilterListPattern\n");
			return STATUS_UNSUCCESSFUL;
		}

		log("ndisGlobalFilterList orginal:%llp\n", ndisGlobalFilterListPattern);

		PVOID ndisGlobalFilterListCall = Utils::reinterpret<PVOID>(ndisGlobalFilterListPattern, 27);
		if (!ndisGlobalFilterListCall)
		{
			err("Failed to find the ndisGlobalFilterListCall\n");
			return STATUS_UNSUCCESSFUL;
		}
		PNDIS_FILTER_BLOCK ndisGlobalFilterList = Utils::translateAddress<PNDIS_FILTER_BLOCK>(ndisGlobalFilterListCall, 7);

		
		if (ndisGlobalFilterList == nullptr)
		{
			err("Failed to find ndisGlobalFilterList !\n");
			return STATUS_SUCCESS;
		}
		log("ndisGlobalFilterList:0x%llp\n", ndisGlobalFilterList);
		
		/*
		// 这是个结构体
		PDWORD ndisFilter_IfBlock = (PDWORD)Utils::FindPatternImage(base,
			"\x48\x85\xff\x0F\x84\x00\x00\x00\x00\x4C\x8B\xA7\x00\x00\x00\x00", "xxxxx????xxx????");

	#ifdef _DBG
		Log::Print("ndisFilter_IfBlock Base:0x%llp\n", ndisFilter_IfBlock);
	#endif

		return ;
		if (ndisFilter_IfBlock == NULL) {
			Log::Print("! failed to find ndisFilter_IfBlock !\n");
			return;
		}
		*/
		/*
		fffff803`65bfa000 4885ff          test    rdi,rdi
		fffff803`65bfa003 0f8421360400    je      ndis!ndisNsiEnumerateAllInterfaceInformation+0x43d3a (fffff803`65c3d62a)
		fffff803`65bfa009 4c8ba7b8020000  mov     r12,qword ptr [rdi+2B8h] <- 这个偏移0x2b8
		*/
		// 这玩意儿实际上就是结构体里面的偏移 _NDIS_FILTER_BLOCK->ifBlock
		/*
			ndis!_NDIS_FILTER_BLOCK
		   +0x000 Header           : _NDIS_OBJECT_HEADER
		   +0x008 NextFilter       : Ptr64 _NDIS_FILTER_BLOCK
		   +0x010 FilterDriver     : Ptr64 _NDIS_FILTER_DRIVER_BLOCK
		   +0x018 FilterModuleContext : Ptr64 Void
		   +0x020 Miniport         : Ptr64 _NDIS_MINIPORT_BLOCK
		   +0x028 FilterInstanceName : wistd::unique_ptr<Rtl::KString,wistd::default_delete<Rtl::KString> >
		   +0x030 FilterFriendlyName : Ptr64 _UNICODE_STRING
		   +0x038 Flags            : Uint4B
		   +0x03c StackFlags       : Uint4B
		   +0x040 State            : _NDIS_FILTER_STATE
		   +0x048 Ref              : _REFERENCE_EX
		   +0x060 FakeStatus       : Int4B
		   +0x068 NextGlobalFilter : Ptr64 _NDIS_FILTER_BLOCK
		   +0x070 LowerFilter      : Ptr64 _NDIS_FILTER_BLOCK
		   +0x078 HigherFilter     : Ptr64 _NDIS_FILTER_BLOCK
		   +0x080 AsyncOpContext   : Ptr64 _FILTER_PAUSE_RESTART_CONTEXT
		   +0x088 NumOfPauseRestartRequests : Int4B
		   +0x090 Lock             : Uint8B
		   +0x098 LockThread       : Ptr64 Void
		   +0x0a0 OidRequestList   : _LIST_ENTRY
		   +0x0b0 PendingOidRequest : Ptr64 _NDIS_OID_REQUEST
		   +0x0b8 PendingOidWatchdog : wil::unique_any_t<wil::details::unique_storage<wil::details::resource_policy<NDISWATCHDOG__ *,void (__cdecl*)(NDISWATCHDOG__ *),&ndisFreeWatchdog,wistd::integral_constant<unsigned __int64,1>,NDISWATCHDOG__ *,__int64,-1,std::nullptr_t> > >
		   +0x0c0 StackExpansionFallback : _NDIS_FILTER_BLOCK::<unnamed-type-StackExpansionFallback>
		   +0x120 StatusIndicationsQueued : UChar
		   +0x124 DroppedReceiveNbls : Int4B
		   +0x128 DroppedSendNbls  : Int4B
		   +0x12c DroppedStatusIndications : Uint4B
		   +0x130 DroppedUncloneableStatusIndications : Uint4B
		   +0x138 PnPRef           : _REFERENCE_EX
		   +0x150 MediaType        : _NDIS_MEDIUM
		   +0x154 PhysicalMediaType : _NDIS_PHYSICAL_MEDIUM
		   +0x158 MediaConnectState : _NET_IF_MEDIA_CONNECT_STATE
		   +0x15c MediaDuplexState : _NET_IF_MEDIA_DUPLEX_STATE
		   +0x160 XmitLinkSpeed    : Uint8B
		   +0x168 RcvLinkSpeed     : Uint8B
		   +0x170 PauseFunctions   : _NDIS_SUPPORTED_PAUSE_FUNCTIONS
		   +0x174 AutoNegotiationFlags : Uint4B
		   +0x178 XState           : UChar
		   +0x179 Reserved1        : UChar
		   +0x17a Reserved2        : UChar
		   +0x17b Reserved3        : UChar
		   +0x17c MediaConnectStateIndicateUp : _NET_IF_MEDIA_CONNECT_STATE
		   +0x180 MediaDuplexStateIndicateUp : _NET_IF_MEDIA_DUPLEX_STATE
		   +0x188 XmitLinkSpeedIndicateUp : Uint8B
		   +0x190 RcvLinkSpeedIndicateUp : Uint8B
		   +0x198 PauseFunctionsIndicateUp : _NDIS_SUPPORTED_PAUSE_FUNCTIONS
		   +0x19c AutoNegotiationFlagsIndicateUp : Uint4B
		   +0x1a0 NextRequestHandle : Ptr64 Void
		   +0x1a8 IterativeDataPathTracker : Ptr64 [4] _NDIS_ITERATIVE_DATA_PATH_TRACKER_ENTRY
		   +0x1b0 NextSendNetBufferListsHandler : Ptr64     void 
		   +0x1b8 NextSendNetBufferListsContext : Ptr64 Void
		   +0x1c0 NextSendNetBufferListsTracker : Ptr64 NDIS_NBL_TRACKER_HANDLE__
		   +0x1c8 NextSendNetBufferListsObject : Ptr64 _NDIS_OBJECT_HEADER
		   +0x1d0 NextSendNetBufferListsCompleteHandler : Ptr64     void 
		   +0x1d8 NextSendNetBufferListsCompleteContext : Ptr64 Void
		   +0x1e0 NextSendNetBufferListsCompleteTracker : Ptr64 NDIS_NBL_TRACKER_HANDLE__
		   +0x1e8 NextSendNetBufferListsCompleteObject : Ptr64 _NDIS_OBJECT_HEADER
		   +0x1f0 NextIndicateReceiveNetBufferListsHandler : Ptr64     void 
		   +0x1f8 NextIndicateReceiveNetBufferListsContext : Ptr64 Void
		   +0x200 NextIndicateReceiveNetBufferListsTracker : Ptr64 NDIS_NBL_TRACKER_HANDLE__
		   +0x208 NextIndicateReceiveNetBufferListsObject : Ptr64 _NDIS_OBJECT_HEADER
		   +0x210 NextReturnNetBufferListsHandler : Ptr64     void 
		   +0x218 NextReturnNetBufferListsContext : Ptr64 Void
		   +0x220 NextReturnNetBufferListsTracker : Ptr64 NDIS_NBL_TRACKER_HANDLE__
		   +0x228 NextReturnNetBufferListsObject : Ptr64 _NDIS_OBJECT_HEADER
		   +0x230 NextCancelSendNetBufferListsHandler : Ptr64     void 
		   +0x238 NextCancelSendNetBufferListsContext : Ptr64 Void
		   +0x240 Characteristics  : _NDIS_FILTER_PARTIAL_CHARACTERISTICS
		   +0x270 FilterSendNetBufferListsHandler : Ptr64     void 
		   +0x278 FilterIndicateReceiveNetBufferListsHandler : Ptr64     void 
		   +0x280 FilterCancelSendNetBufferListsHandler : Ptr64     void 
		   +0x288 TcpOffloadReceiveReturnHandler : Ptr64     int 
		   +0x290 NblTracker       : Ptr64 NDIS_NBL_TRACKER_HANDLE__
		   +0x298 Offload          : Ptr64 _NDIS_FILTER_TASK_OFFLOAD
		   +0x2a0 InterfaceGuid    : _GUID
		   +0x2b0 IfIndex          : Uint4B
		   +0x2b8 IfBlock          : Ptr64 _NDIS_IF_BLOCK
		   +0x2c0 NextDirectRequestHandle : Ptr64 Void
		   +0x2c8 DirectOidRequestCount : Uint4B
		   +0x2d0 WOLPatternList   : _SINGLE_LIST_ENTRY
		   +0x2d8 PMProtocolOffloadList : _SINGLE_LIST_ENTRY
		   +0x2e0 PMCurrentParameters : _NDIS_PM_PARAMETERS
		   +0x2f8 Bind             : Ptr64 NDIS_BIND_FILTER_LINK
		   +0x300 HwTimestampCapabilities : Ptr64 _NDIS_TIMESTAMP_CAPABILITIES
		   +0x308 HwTimestampCurrentConfig : Ptr64 _NDIS_TIMESTAMP_CAPABILITIES
		   +0x310 VerifierContext  : Ptr64 Void
		   +0x318 PktMonComp       : _PKTMON_COMPONENT_CONTEXT
		   +0x350 PktMonEdgeLower  : _PKTMON_EDGE_CONTEXT
		   +0x378 PktMonEdgeUpper  : _PKTMON_EDGE_CONTEXT

		*/
		DWORD ndisFilter_IfBlock_offset = 0x2b8; // win10 22h2



#ifdef _DBG
		log("ndisGlobalFilterList Base:0x%llp, ndisFilter_IfBlock_offset: 0x%lx\n", ndisGlobalFilterList, ndisFilter_IfBlock_offset);
#endif

		/*
		 *  ndisDummyIrpHandler
		 *
		 *  fffff804`87a35da0 488bc4          mov     rax,rsp
			fffff804`87a35da3 48895808        mov     qword ptr [rax+8],rbx
			fffff804`87a35da7 48896810        mov     qword ptr [rax+10h],rbp
			fffff804`87a35dab 48897018        mov     qword ptr [rax+18h],rsi
			fffff804`87a35daf 48897820        mov     qword ptr [rax+20h],rdi
			fffff804`87a35db3 4157            push    r15
			fffff804`87a35db5 4883ec40        sub     rsp,40h
			fffff804`87a35db9 488b5940        mov     rbx,qword ptr [rcx+40h]


		 * M$ was so kind to have this function in there
		 */
		PVOID ndisDummyIrpHandler = Utils::FindPatternImage(base,
			"\x48\x8b\xc4\x48\x89\x58\x00\x48\x89\x68\x00\x48\x89\x70\x00\x48\x89\x78\x00\x41\x57\x48\x83\xec",
			"xxxxxx?xxx?xxx?xxx?xxxxx");
		if (!ndisDummyIrpHandler) {
			err("failed to get ndisDummyIrpHandler!\n");
			return STATUS_SUCCESS;
		}
		log("ndisDummyIrpHandler Base: 0x%llp!\n", ndisDummyIrpHandler);


		//return ;

		DWORD count = 0;
		for (PNDIS_FILTER_BLOCK filter = *(PNDIS_FILTER_BLOCK*)ndisGlobalFilterList; filter; filter = filter->NextFilter) {

			log("filter Addr:0x%llp\n", filter);
			PNDIS_IF_BLOCK block = *(PNDIS_IF_BLOCK*)((PBYTE)filter + ndisFilter_IfBlock_offset);
			if (block == NULL)
			{
				if (filter == filter->NextFilter) break;
				continue;
			}

			log("block:0x%llp\n", block);

			PWCHAR InstanceName = (PWCHAR)SafeCopy(filter->FilterInstanceName->Buffer, MAX_PATH);
			if (InstanceName == NULL) {
				err("failed to copy buffer. Line: %d\n", __LINE__);
				if (filter == filter->NextFilter) break;
				continue;
			}
			log("InstanceName: %ws\n", InstanceName);

			WCHAR adapter[MAX_PATH] = { 0 };
			swprintf(adapter, L"\\Device\\%ws", TrimGUID(InstanceName, MAX_PATH / 2));
			ExFreePool(InstanceName);

			log("found NIC %ws\n", adapter);

			UNICODE_STRING name = { 0 };
			RtlInitUnicodeString(&name, adapter);

			PFILE_OBJECT file = 0;
			PDEVICE_OBJECT device = 0;

			NTSTATUS status = IoGetDeviceObjectPointer(&name, FILE_READ_DATA, &file, &device);
			if (!NT_SUCCESS(status)) {
				err("! failed to get %wZ: %p !Line:%d\n", &name, status, __LINE__);
				if (filter == filter->NextFilter) break;
				continue;
			}
			log("Success to GetDeviceObjectPointer: 0x%llp, Name:%ws\n", device, name.Buffer);

			PDRIVER_OBJECT driver = device->DriverObject;
			if (driver == NULL)
			{
				err("failed to get the Driver Object\n");
				if (filter == filter->NextFilter) break;
				continue;
			}
			log("Success to Get driver Object: 0x%llp !\n", driver);



			BOOL exists = FALSE;
			for (DWORD i = 0; i < NICs.Length; ++i) {
				if (NICs.Drivers[i].DriverObject == driver) {
					exists = TRUE;
					log("Success to find driver:0x%llp [%d]!\n", NICs.Drivers[i].DriverObject, i);
					break;
				}
			}

			if (exists) {
				log("%wZ already swapped\n", &driver->DriverName);
			}
			else {
				PNIC_DRIVER nic = &NICs.Drivers[NICs.Length];
				nic->DriverObject = driver;

#ifdef __HOOK_DISPATCH_CUSTOM__
				Utils::AppendSwap(&driver->DriverName, &driver->MajorFunction[IRP_MJ_DEVICE_CONTROL], NICControl, nic->Original);
#else 
				Utils::AppendSwap(&driver->DriverName, &driver->MajorFunction[IRP_MJ_DEVICE_CONTROL], ndisDummyIrpHandler, nic->Original);
#endif
				++NICs.Length;
			}

			// Indirectly dereferences device object
			ObDereferenceObject(file);


			// Current MAC
			PIF_PHYSICAL_ADDRESS_LH addr = &block->ifPhysAddress;
			Utils::SpoofBuffer(SEED, addr->Address, addr->Length);
			addr = &block->PermanentPhysAddress;
			Utils::SpoofBuffer(SEED, addr->Address, addr->Length);

			++count;

		}

		log("handled %d MACs\n", count);

		return STATUS_SUCCESS;
	}



	bool NICManager::Initialize()
	{
		m_ndisBase = Utils::GetModuleBase("ndis.sys");
		if (m_ndisBase == nullptr) {
			log("Failed to find m_ndisBase!");
			return false;
		}
		/*
			ndis!ndisReferenceFilterByHandle:
			fffff805`674194a0 48895c2408      mov     qword ptr [rsp+8],rbx
			fffff805`674194a5 4889742418      mov     qword ptr [rsp+18h],rsi
			fffff805`674194aa 88542410        mov     byte ptr [rsp+10h],dl
			fffff805`674194ae 57              push    rdi
			fffff805`674194af 4883ec20        sub     rsp,20h
			fffff805`674194b3 488bf9          mov     rdi,rcx
			fffff805`674194b6 33db            xor     ebx,ebx   // 24

			fffff805`674194b8 488d0d09b60700  lea     rcx,[ndis!ndisGlobalFilterListLock (fffff805`67494ac8)]
			fffff805`674194bf 4c8b1522a40800  mov     r10,qword ptr [ndis!_imp_KeAcquireSpinLockRaiseToDpc (fffff805`674a38e8)]
			fffff805`674194c6 e8054440fa      call    nt!KeAcquireSpinLockRaiseToDpc (fffff805`6181d8d0)
			fffff805`674194cb 408af0          mov     sil,al
			fffff805`674194ce 488b0543b00700  mov     rax,qword ptr [ndis!ndisGlobalFilterList (fffff805`67494518)]
		*/
		PVOID ndisReferenceFilterByHandle =
			Utils::FindPatternImage(m_ndisBase,
				"\x48\x89\x5c\x24\x00\x48\x89\x74\x24\x00\x88\x54\x24\x00\x57",
				"xxxx?xxxx?xxx?x");
		if (ndisReferenceFilterByHandle == nullptr) {
			log("Failed to find ndisReferenceFilterByHandle!");
			return false;
		}

		PVOID ndisGlobalFilterListCalled = Utils::reinterpret<PVOID>(ndisReferenceFilterByHandle, 46);
		m_ndisGlobalFilterList = Utils::translateAddress<PVOID>(ndisGlobalFilterListCalled, 7);
		if (m_ndisGlobalFilterList == nullptr) {
			log("Failed to find m_ndisGlobalFilterList!");
			return false;
		}

		/*
			ndis!ndisDummyIrpHandler:
			fffff805`673b7c40 488bc4          mov     rax,rsp
			fffff805`673b7c43 48895808        mov     qword ptr [rax+8],rbx
			fffff805`673b7c47 48896810        mov     qword ptr [rax+10h],rbp
			fffff805`673b7c4b 48897018        mov     qword ptr [rax+18h],rsi
			fffff805`673b7c4f 48897820        mov     qword ptr [rax+20h],rdi
			fffff805`673b7c53 4157            push    r15
			fffff805`673b7c55 4883ec40        sub     rsp,40h
			fffff805`673b7c59 488b5940        mov     rbx,qword ptr [rcx+40h]
		*/
		m_ndisDummyIrpHandler = Utils::FindPatternImage(m_ndisBase,
			"\x48\x8b\xc4\x48\x89\x58\x00\x48\x89\x68\x00\x48\x89\x70\x00\x48\x89\x78\x00\x41\x57\x48\x83\xec",
			"xxxxxx?xxx?xxx?xxx?xxxxx");
		if (m_ndisDummyIrpHandler == nullptr) {
			log("Failed to find m_ndisDummyIrpHandler!");
			return false;
		}


		return true;
	}

	bool NICManager::Execute()
	{
		if (m_ndisBase == nullptr || m_ndisGlobalFilterList == nullptr || m_ndisDummyIrpHandler == nullptr)
		{
			log("Please init NIC Module first!");
			ShowAllAddress();
			return false;
		}



		return false;
	}

	void NICManager::ShowAllAddress()
	{
		log("\n m_ndisBase:%llx \n m_ndisGlobalFilterList:%llx \n m_ndisDummyIrpHandler:%llx",
			m_ndisBase, m_ndisGlobalFilterList, m_ndisDummyIrpHandler);
	}

};
