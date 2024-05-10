#pragma once
#include <ntifs.h>
#include "HWIDChanger.h"


/*
	nvidia-smi --query-gpu=index,name,serial,uuid,vbios_version,driver_version,pstate --format=csv
	��������ѯ�Կ���������Ϣ��

	index: �Կ���������
	name: �Կ�������
	serial: �Կ������к�
	uuid: �Կ���Ψһ��ʶ����UUID��
	vbios_version: �Կ��� VBIOS �汾
	driver_version: �Կ���������İ汾
	pstate: �Կ�������״̬
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