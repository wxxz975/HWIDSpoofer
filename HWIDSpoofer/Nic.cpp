#include "Nic.h"

#include "Utils.h"
#include "Logger.hpp"
#include "Shared.h"

namespace Nic
{
	static DWORD SEED = 0x7899;

	/*
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


	*/




	/**** NIC ****/
	/*
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
	}*/

	static PVOID SafeCopy(PVOID src, DWORD size) {
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

	static PWCHAR TrimGUID(PWCHAR guid, DWORD max) {
		DWORD i = 0;
		PWCHAR start = guid;

		--max;
		for (; i < max && *start != L'{'; ++i, ++start);
		for (; i < max && guid[i++] != L'}';);

		guid[i] = 0;
		return start;
	}


	/*

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
	*/




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
		m_ndisGlobalFilterList = Utils::translateAddress<PVOID*>(ndisGlobalFilterListCalled, 7);
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
		
		/*
		ndis!ndisAddDevice+0xeb:
		fffff804`2d8dc5ef 4c8b3d8a7afbff  mov     r15,qword ptr [ndis!ndisMiniDriverList (fffff804`2d894080)]
		fffff804`2d8dc5f6 4d85ff          test    r15,r15
		fffff804`2d8dc5f9 7409            je      ndis!ndisAddDevice+0x100 (fffff804`2d8dc604)
		fffff804`2d8dc5fb 4d3bfd          cmp     r15,r13
		fffff804`2d8dc5fe 0f856b710000    jne     ndis!ndisAddDevice+0x726b (fffff804`2d8e376f)
		fffff804`2d8dc604 8ad0            mov     dl,al
		fffff804`2d8dc606 488d0dab84fbff  lea     rcx,[ndis!ndisMiniDriverListLock (fffff804`2d894ab8)]
		fffff804`2d8dc60d 4c8b15cc72fcff  mov     r10,qword ptr [ndis!_imp_KeReleaseSpinLock (fffff804`2d8a38e0)]
		*/
		PVOID ndisMiniDriverListCalled = Utils::FindPatternImage(m_ndisBase,
			"\x4c\x8b\x3d\x00\x00\x00\x00\x4d\x85\xff\x74\x00\x4d\x3b\xfd\x0f\x85",
			"xxx????xxxx?xxxxx");
		if (ndisMiniDriverListCalled == nullptr) {
			log("Failed to find m_ndisMiniDriverList!");
			return false;
		}
		m_ndisMiniDriverList = Utils::translateAddress<PVOID>(ndisGlobalFilterListCalled, 7);
		

		ShowAllAddress();
		return true;
	}

	bool NICManager::Execute()
	{
		if (m_ndisBase == nullptr || m_ndisGlobalFilterList == nullptr || m_ndisDummyIrpHandler == nullptr || m_ndisMiniDriverList == nullptr)
		{
			err("Please init NIC Module first!\n");
			ShowAllAddress();
			return false;
		}

		if (!ChangeMacAddress()) {
			err("Failed to Execute ChangeMacAddress!\n");
			return false;
		}


		return true;
	}

	void NICManager::ShowAllAddress()
	{
		log("\n m_ndisBase:%llx \n m_ndisGlobalFilterList:%llx \n m_ndisDummyIrpHandler:%llx\n m_ndisMiniDriverList:%llx\n",
			m_ndisBase, m_ndisGlobalFilterList, m_ndisDummyIrpHandler, m_ndisMiniDriverList);
	}

	static VOID ShowMacAddress(const PIF_PHYSICAL_ADDRESS_LH addr) {

		log("MacAddress:");
		for (int idx = 0; idx < addr->Length; ++idx) {
			log("%02X", addr->Address[idx]);
			if (idx < addr->Length - 1)
				log("-");
		}
		log("\n");
	}


	DWORD Random(PDWORD seed) {
		DWORD s = *seed * 1103515245 + 12345;
		*seed = s;
		return (s / 65536) % 32768;
	}

	DWORD Hash(PBYTE buffer, DWORD length) {
		if (!length) {
			return 0;
		}

		DWORD h = (*buffer ^ 0x4B9ACE3F) * 0x1040193;
		for (DWORD i = 1; i < length; ++i) {
			h = (buffer[i] ^ h) * 0x1040193;
		}
		return h;
	}

	bool NICManager::ChangeMacAddress() const
	{
		for (PNDIS_FILTER_BLOCK filter = *reinterpret_cast<PNDIS_FILTER_BLOCK*>(m_ndisGlobalFilterList); 
			filter; 
			filter = filter->NextFilter) 
		{
			PFILE_OBJECT file = nullptr;
			PDEVICE_OBJECT device = nullptr;
			PDRIVER_OBJECT driver = nullptr;

			PWCHAR instanceName = reinterpret_cast<PWCHAR>(SafeCopy(filter->FilterInstanceName->Buffer, MAX_PATH));
			if (instanceName == nullptr)  continue;

			WCHAR adapter[MAX_PATH] = { 0 };
			swprintf(adapter, L"\\Device\\%ws", TrimGUID(instanceName, MAX_PATH / 2));
			ExFreePool(instanceName);
			log("found NIC %ws\n", adapter);

			UNICODE_STRING deviceName = { 0 };
			RtlInitUnicodeString(&deviceName, adapter);

			
			NTSTATUS status = IoGetDeviceObjectPointer(&deviceName, FILE_READ_DATA, &file, &device);
			if (!NT_SUCCESS(status)) {
				err("Failed to find the device:%ws\n", deviceName.Buffer);
				continue;
			}
			
			driver = device->DriverObject;
			if (driver == nullptr) {
				err("Failed to find the driver Object!\n");
				continue;
			}

			driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = reinterpret_cast<PDRIVER_DISPATCH>(m_ndisDummyIrpHandler);
			ObDereferenceObject(file);
			log("Success Hook The nic:%ws\n", adapter);
		}
		return true;
		
	}

};
