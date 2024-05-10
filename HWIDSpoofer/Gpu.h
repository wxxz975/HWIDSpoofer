#pragma once
#include <ntifs.h>
#include "HWIDChanger.h"


/*
	nvidia-smi --query-gpu=index,name,serial,uuid,vbios_version,driver_version,pstate --format=csv
	这个命令将查询显卡的以下信息：

	index: 显卡的索引号
	name: 显卡的名称
	serial: 显卡的序列号
	uuid: 显卡的唯一标识符（UUID）
	vbios_version: 显卡的 VBIOS 版本
	driver_version: 显卡驱动程序的版本
	pstate: 显卡的性能状态
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
		bool HookDeviceControl();
		bool UnhookDeviceControl() const;

	private:
		static NTSTATUS DeviceIOControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);

	private:
		static PDRIVER_DISPATCH m_original_device_ctl;
	};

	
}