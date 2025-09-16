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
		

		bool Initialize() override;
		bool Execute()override;
		void ShowAllAddress() override;
		
	private:
		PVOID m_nvlddmkmBase = nullptr;

		typedef uintptr_t(*GpuMgrGetGpuFromId)(int);
		GpuMgrGetGpuFromId m_gpuMgrGetGpuFromId = nullptr;
		UINT32 m_UuidValidOffset = 0;
	};

	
}