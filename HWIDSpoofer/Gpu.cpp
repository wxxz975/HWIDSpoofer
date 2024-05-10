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
		//UnhookDeviceControl();
	}
	bool GpuManager::Initialize()
	{
		return true;
	}
	bool GpuManager::Execute()
	{
		return HookDeviceControl();
	}
	void GpuManager::ShowAllAddress()
	{

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
		log("Hook Gpu OriginalDeviceIoControl:%llx to DeviceIoControl:%llx\n", m_original_device_ctl, DeviceIOControl);

		
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

					//ExFreePoolWithTag(buffer, tag); //暂时保留查看
				}
			}
			
			return status;
		}*/

		return m_original_device_ctl(DeviceObject, irp);
	}
}