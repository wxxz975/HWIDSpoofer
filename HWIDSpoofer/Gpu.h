#pragma once
#include <ntifs.h>
#include "HWIDChanger.h"


/*
	nvidia-smi --query-gpu=index,name,serial,uuid,vbios_version,driver_version,pstate --format=csv
	
*/
namespace Gpu
{
	class GpuManager: public HWIDChanger
	{
	public:
		GpuManager();
		~GpuManager();
		typedef uintptr_t(*GpuMgrGetGpuFromId)(int);

		bool Initialize() override;
		bool Execute()override;
		void ShowAllAddress() override;
		
	private:
		bool HookDeviceControl();
		bool UnhookDeviceControl() const;

	private:
		static NTSTATUS DeviceIOControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);

	private:
		static PDRIVER_DISPATCH m_original_device_ctl;
		PVOID m_nvlddmkmBase = nullptr;
		PVOID m_gpuObject = nullptr;

		GpuMgrGetGpuFromId m_gpuMgrGetGpuFromId = nullptr;
		UINT32 m_UuidValidOffset = 0;
		UINT32 m_gpuObjectOffset = 0;
	};

	
}