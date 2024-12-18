#pragma once
#include "HWIDChanger.h"

#include "Disk.h"
#include "Nic.h"
#include "Smbios.h"

#include "Gpu.h"

class HWIDSpoofer
{
public:
	HWIDSpoofer() = default;
	~HWIDSpoofer() = default;

	bool Initialize();

	bool Execute();

private:
	
	Disk::DiskManager m_diskMgr;
	Nic::NICManager m_nicMgr;
	Smbios::SmbiosManager m_smbiosMgr;
	
	Gpu::GpuManager m_gpuMgr;
};

