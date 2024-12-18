#include "Gpu.h"

#include "Shared.h"
#include "Logger.hpp"
#include "Utils.h"
#include <wchar.h>
namespace Gpu
{
	extern "C" POBJECT_TYPE * IoDriverObjectType;

	PDRIVER_DISPATCH GpuManager::m_original_device_ctl = nullptr;

	GpuManager::GpuManager()
	{
	}

	GpuManager::~GpuManager()
	{
		
	}
	bool GpuManager::Initialize()
	{
		GUID guid;
		m_nvlddmkmBase = Utils::GetModuleBase("nvlddmkm.sys");
		if (!m_nvlddmkmBase) {
			err("Failed to get the m_nvlddmkmBase!\n");
			return false;
		}
		log("Found nvlddmkmBase:%llx\n", m_nvlddmkmBase);

		/*
			works for 560.92 → 561.09
		*/
		m_UuidValidOffset = 0xb2e; // 0xb2d
		m_gpuObjectOffset = 0x124700;
		uintptr_t Addr = (uintptr_t)m_nvlddmkmBase;
		auto gpu = ((uintptr_t(*)(int))(Addr + m_gpuObjectOffset))(0);
		for (int i = 0; i < sizeof UUID; ++i) {
			*(PBYTE)(gpu + m_UuidValidOffset + i) = __rdtsc();
		}
		return true;
		// 48 8B 05 ? ? ? ? 4C 8B F2 48 8B B8
		/*m_gpuObject = Utils::FindPatternImage(m_nvlddmkmBase,
			"\x48\x8b\x05\x00\x00\x00\x00\x4c\x8b\xf2\x48\x8b\xB8",
			"xxx????xxxxxx");*/
		/*
		* 
		*  E8 ?? ?? ?? ?? 48 8B D8 48 85 C0 0F 84 ?? ?? ?? ?? 44 8B 80 ?? ?? ?? ?? 48 8D 15
		m_gpuObject = Utils::FindPatternImage(m_nvlddmkmBase, 
			"\xE8\x00\x00\x00\x00\x48\x8B\xD8\x48\x85\xC0\x0F\x84\x00\x00\x00\x00\x44\x8B\x80\x00\x00\x00\x00\x48\x8D\x15",
			"x????xxxxxxxx????xxx????xxx");
		if (!m_gpuObject) {
			log("Failed to get the gpu object!\n");
			return false;
		}

		uintptr_t Addr = (uintptr_t)m_gpuObject;
		uintptr_t scanAddr = (uintptr_t)m_gpuObject;
		UINT8 offs = 0x3B;
		bool offsFound = false;
		for (int i = 0; i < 64; i++)
		{
			if (!Addr || *(PBYTE)(Addr + offs + i) == 0xE8) { // call
				offsFound = true;
				break;
			}
		}
		if (!offsFound)
		{
			log("Could not find pattern.\n");
			log("Failed to handle NVIDIA GPU UUID(s)!\n");
			return STATUS_UNSUCCESSFUL;
		}

		uintptr_t(*GpuMgrGetGpuFromId)(int) = decltype(GpuMgrGetGpuFromId)(*(int*)(Addr + 1) + 5 + Addr);
		Addr += offs;
		Addr += *(int*)(Addr + 1) + 5;
		log("GpuMgrGetGpuFromId: %llx\n", Addr);

		for (int i = 0; i < 256; i++) {
			if (*(PBYTE)scanAddr == 0x80 && *(PBYTE)(scanAddr + 1) == 0xBB) {
				if (*(PBYTE)(scanAddr + 4) == 0x00 && *(PBYTE)(scanAddr + 5) == 0x00 && *(PBYTE)(scanAddr + 6) == 0x00) {
					m_UuidValidOffset = *(PBYTE)(scanAddr + 3) << 8 | *(PBYTE)(scanAddr + 2);
					log("Found m_UuidValidOffset at offset: 0x%x\n", m_UuidValidOffset);
					break;
				}
			}
			scanAddr++;
		}

		// Could not find GPU::gpuUuid.isInitialized offset
		if (!m_UuidValidOffset) {
			log("Failed to find offset.\n");
			return false;
		}

		return true;*/
	}
	bool GpuManager::Execute()
	{
		UUID newUUID = {0};
		// Max number of GPUs supported is 32.
		for (int i = 0; i < 32; i++) {
			uintptr_t ProbedGPU = m_gpuMgrGetGpuFromId(i);

			// Does not exist?
			if (!ProbedGPU) continue;

			// Is GPU UUID not initialized?
			if (!*(bool*)(ProbedGPU + m_UuidValidOffset)) continue;

			// Update each byte of the existing UUID with the new UUID
			const PBYTE newUUIDBytes = reinterpret_cast<const PBYTE>(&newUUID);
			for (int j = 0; j < sizeof(GUID); j++)
				*(PBYTE)(ProbedGPU + m_UuidValidOffset + 1 + j) = newUUIDBytes[j];

			log("Spoofed GPU %d.\n", i);
		}
		return true;

		//return HookDeviceControl();
	}
	void GpuManager::ShowAllAddress()
	{
		log("m_nvlddmkmBase:%llx, m_gpuObject:%llx\n", m_nvlddmkmBase, m_gpuObject);
	}

#define IOCTL_NVIDIA_SMIL (0x8DE0008)
#define IOCTL_NVIDIA_SMIL_MAX (512)


	bool GpuManager::HookDeviceControl()
	{
		const wchar_t* deviceNameStr = L"\\Driver\\nvlddmkm";
		UNICODE_STRING deviceName;
		RtlInitUnicodeString(&deviceName, deviceNameStr);

		PDRIVER_OBJECT driver_object = 0;
		NTSTATUS status = ObReferenceObjectByName(&deviceName, OBJ_CASE_INSENSITIVE, 0, 0, *IoDriverObjectType, KernelMode, 0, (void**)&driver_object);
		if (!NT_SUCCESS(status)) {
			err("Can not find the gpu driver object!\n");
			return false;
		}

		m_original_device_ctl = driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL];
		driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceIOControl;
		
		ObDereferenceObject(driver_object);
		
		return true;
	}
	bool GpuManager::UnhookDeviceControl() const
	{
		const wchar_t* deviceNameStr = L"\\Driver\\nvlddmkm";
		UNICODE_STRING deviceName;
		RtlInitUnicodeString(&deviceName, deviceNameStr);

		PDRIVER_OBJECT driver_object = 0;
		NTSTATUS status = ObReferenceObjectByName(&deviceName, OBJ_CASE_INSENSITIVE, 0, 0, *IoDriverObjectType, KernelMode, 0, (void**)&driver_object);
		if (!NT_SUCCESS(status))
			return false;


		if(m_original_device_ctl) {
			driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = m_original_device_ctl;
			log("Unhooked DeviceIoControl!\n");
		}

		ObDereferenceObject(driver_object);
		//ndisMiniportList
		return true;
	}

	static BOOL FindStr(const char* dst, int size, const char* str) {
		int len = strlen(str);
		for (int i = 0; i < size - len; ++i) {
			if (strcmp(dst + i, str) == 0)
				return true;
		}

		return false;
	}
	
	NTSTATUS GpuManager::DeviceIOControl(PDEVICE_OBJECT DeviceObject, PIRP irp)
	{
		//m_original_device_ctl(DeviceObject, Irp);
		PIO_STACK_LOCATION ioc = IoGetCurrentIrpStackLocation(irp);
		char customize_gpu_serial[100]{ 0 };

		NTSTATUS status = m_original_device_ctl(DeviceObject, irp);
		
		if (NT_SUCCESS(status)) {
			char* original_buffer = (char*)irp->UserBuffer;
			const int length = IOCTL_NVIDIA_SMIL_MAX;
			void* buffer = ExAllocatePool(NonPagedPool, length);
			if (buffer) {
				MM_COPY_ADDRESS addr{ 0 };
				addr.VirtualAddress = irp->UserBuffer;
				SIZE_T copy_size = 0;
				if (NT_SUCCESS(MmCopyMemory(buffer, addr, length, MM_COPY_MEMORY_VIRTUAL, &copy_size))
					&& copy_size == length)
				{
					log("Current code:%ul, Buffer:%llx\n", ioc->Parameters.DeviceIoControl.IoControlCode, buffer);
					char * str1 = "\xaf\xa3\xde\x5a";
					char* str2 = "\x77\x3b";
					char* str3 = "\xbf\xc4";
					char* str4 = "\x13\x81";
					char* str5 = "\xe6\x11\x9c\x50\xf3\x8d";

					if (FindStr((char*)buffer, length, str1)
						&& FindStr((char*)buffer, length, str2)
						&& FindStr((char*)buffer, length, str3)
						&& FindStr((char*)buffer, length, str4)
						&& FindStr((char*)buffer, length, str5)) {

						log("Found uuid code:%ul\n", ioc->Parameters.DeviceIoControl.IoControlCode);
					}
				}
				//ExFreePool(buffer);
			}
		}
		return status;
		////////////////////////////////////////////////////
		/*
		if (ioc->Parameters.DeviceIoControl.IoControlCode == IOCTL_NVIDIA_SMIL)
		{
			NTSTATUS status = m_original_device_ctl(DeviceObject, irp);

			char* original_buffer = (char*)irp->UserBuffer;
			const int length = IOCTL_NVIDIA_SMIL_MAX;

			if (original_buffer)
			{
				const unsigned long tag = 'Gput';
				void* buffer = ExAllocatePoolWithTag(NonPagedPool, length, tag);
				if (buffer)
				{
					MM_COPY_ADDRESS addr{ 0 };
					addr.VirtualAddress = irp->UserBuffer;

					SIZE_T copy_size = 0;
					if (NT_SUCCESS(MmCopyMemory(buffer, addr, length, MM_COPY_MEMORY_VIRTUAL, &copy_size))
						&& copy_size == length)
					{

						log("DeviceIoControl Return Buffer:%llx, Size:%d\n", buffer, length);
						
						
						const char* gpu = "GPU-";
						const size_t len = strlen(gpu);

						for (int i = 0; i < length - len; i++)
						{
							char* ptr = (char*)buffer + i;
							if (0 == memcmp(ptr, gpu, strlen(gpu)))
							{
								RtlCopyMemory(original_buffer + i + len, customize_gpu_serial, 100);
								Utils::RandomizeString(original_buffer + i + len);
								break;
							}
						}
					}

					//ExFreePoolWithTag(buffer, tag); //��ʱ����鿴
				}
			}
			
			return status;
		}*/

		return m_original_device_ctl(DeviceObject, irp);
	}

//	namespace nvidia_ctx
//	{
//#define NV_MAX_DEVICES 32
//
//		uint64_t g_system{};
//
//		FORCEINLINE BOOLEAN init(void)
//		{
//			uint64_t nvlddmkm_base = kernel::find_driver(_x("nvlddmkm.sys"));
//
//			g_system = *(uint64_t*)(nvlddmkm_base + 0xB93D40); // 48 8B 05 ? ? ? ? 4C 8B F2 44 8B E9
//
//			return (g_system != 0);
//		}
//
//		FORCEINLINE uint64_t gpu_data(uint32_t gpu_instance)
//		{
//			uint64_t gpu_sys = *(uint64_t*)(g_system + 0x1C0);
//			uint64_t gpu_mgr = *(uint64_t*)(gpu_sys + 0x3CAD0);
//
//			if (!gpu_mgr)
//				return 0;
//
//			gpu_sys += 0x3C8D0;
//
//			uint64_t gpu_device{};
//
//			while (1)
//			{
//				uint32_t found_instance = *(uint32_t*)(gpu_sys + 0x8);
//
//				if (found_instance == gpu_instance)
//				{
//					uint64_t device = *(uint64_t*)gpu_sys;
//
//					if (device != 0)
//						gpu_device = device;
//
//					break;
//				}
//
//				gpu_sys += 0x10;
//			}
//
//			return gpu_device;
//		}
//
//		FORCEINLINE uint64_t next_gpu(uint32_t device_mask, uint32_t* start_index)
//		{
//			if (*start_index >= NV_MAX_DEVICES)
//				return 0;
//
//			for (uint32_t i = *start_index; i < NV_MAX_DEVICES; ++i)
//			{
//				if (device_mask & (1U << i))
//				{
//					*start_index = i + 1;
//					return gpu_data(i);
//				}
//			}
//
//			*start_index = NV_MAX_DEVICES;
//
//			return 0;
//		}
//
//		FORCEINLINE BOOLEAN change_uuid(uint64_t gpu_object)
//		{
//			if (*(uint8_t*)(gpu_object + 0x848))
//			{
//				uint8_t* uuid_data = (uint8_t*)(gpu_object + 0x849);
//
//				// randomize your GPU UUID here
//				uuid_data[0] = wp::rand_between<uint8_t>(0x01, 0xFF);
//				uuid_data[1] = wp::rand_between<uint8_t>(0x01, 0xFF);
//				uuid_data[2] = wp::rand_between<uint8_t>(0x01, 0xFF);
//				uuid_data[3] = wp::rand_between<uint8_t>(0x01, 0xFF);
//				uuid_data[4] = wp::rand_between<uint8_t>(0x01, 0xFF);
//				uuid_data[5] = wp::rand_between<uint8_t>(0x01, 0xFF);
//				uuid_data[6] = wp::rand_between<uint8_t>(0x01, 0xFF);
//				uuid_data[7] = wp::rand_between<uint8_t>(0x01, 0xFF);
//				uuid_data[8] = wp::rand_between<uint8_t>(0x01, 0xFF);
//				uuid_data[9] = wp::rand_between<uint8_t>(0x01, 0xFF);
//				uuid_data[10] = wp::rand_between<uint8_t>(0x01, 0xFF);
//				uuid_data[11] = wp::rand_between<uint8_t>(0x01, 0xFF);
//				uuid_data[12] = wp::rand_between<uint8_t>(0x01, 0xFF);
//				uuid_data[13] = wp::rand_between<uint8_t>(0x01, 0xFF);
//				uuid_data[14] = wp::rand_between<uint8_t>(0x01, 0xFF);
//				uuid_data[15] = wp::rand_between<uint8_t>(0x01, 0xFF);
//
//				return TRUE;
//			}
//			else
//			{
//				DBG_LOG("device hasn't been initialized yet!");
//			}
//
//			return FALSE;
//		}
//
//		FORCEINLINE BOOLEAN spoof_gpu(void)
//		{
//			BOOLEAN status = FALSE;
//			uint64_t gpu_sys = *(uint64_t*)(g_system + 0x1C0);
//
//			if (!gpu_sys)
//				return status;
//
//			uint32_t gpu_index{},
//				gpu_mask = *(uint32_t*)(gpu_sys + 0x754);
//
//			// loops through all available GPU's (limited to NV_MAX_DEVICES)
//			while (1)
//			{
//				uint64_t gpu_object = next_gpu(gpu_mask, &gpu_index);
//
//				if (!gpu_object)
//					break;
//
//				if (change_uuid(gpu_object))
//					status = TRUE;
//				else
//					status = FALSE;
//			}
//
//			return status;
//		}
//	}
//
//	// usage;
//	NTSTATUS driver_entry(...)
//	{
//		// ...
//		if (!nvidia_ctx::init())
//		{
//			DBG_LOG("failed initializing nvidia context!"); // most likely wrong offset or no nvidia GPU/drivers installed
//
//			return STATUS_UNSUCCESSFUL;
//		}
//
//		if (!nvidia_ctx::spoof_gpu())
//		{
//			DBG_LOG("failed spoofing gpu!");
//
//			return STATUS_UNSUCCESSFUL;
//		}
//	}

}