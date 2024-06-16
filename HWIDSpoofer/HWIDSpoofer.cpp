#include "HWIDSpoofer.h"


#include "Disk.h"
#include "Nic.h"
#include "Smbios.h"
#include "Gpu.h"

#include "Logger.hpp"

bool HWIDSpoofer::Initialize()
{	
	if (!m_diskMgr.Initialize()) {
		err("Error Init DiskManager\n");
		return false;
	}
	m_diskMgr.ShowAllAddress();


	if (!m_nicMgr.Initialize()) {
		err("Error Init NICManager\n");
		return false;
	}
	m_nicMgr.ShowAllAddress();

	if (!m_smbiosMgr.Initialize()) {
		err("Error Init SmbiosManager\n");
		return false;
	}
	m_smbiosMgr.ShowAllAddress();

	return true;
}

bool HWIDSpoofer::Execute()
{

	if (!m_diskMgr.Execute()) {
		err("Error on Execute DiskManager\n");
		return false;
	}

	if (!m_nicMgr.Execute()) {
		err("Error on Execute NICManager\n");
		return false;
	}

	
	if (!m_smbiosMgr.Execute()) {
		err("Error on Execute SmbiosManager\n");
		return false;
	}

	return true;
}
