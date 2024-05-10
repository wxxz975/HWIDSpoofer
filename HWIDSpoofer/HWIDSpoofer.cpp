#include "HWIDSpoofer.h"


#include "Logger.hpp"

bool HWIDSpoofer::Initialize()
{
	
	if (!m_diskMgr.Initialize()) {
		err("Failed to init diskMgr!");
		return false;
	}
	m_diskMgr.ShowAllAddress();


	if (!m_nicMgr.Initialize()) {
		err("Failed to init nicMgr!");
		return false;
	}
	m_nicMgr.ShowAllAddress();


	if (!m_smbiosMgr.Initialize()) {
		err("Failed to init smbiosMgr!");
		return false;
	}
	m_smbiosMgr.ShowAllAddress();
	/*
	if (!m_gpuMgr.Initialize()) {
		err("Failed to init gpuMgr!");
		return false;
	}*/

	return true;
}

bool HWIDSpoofer::Execute()
{
	
	if (!m_nicMgr.Execute()){
		err("Failed to Execute m_nicMgr\n");
		return false;
	}
	if (!m_diskMgr.Execute()) {
		err("Failed to Execute m_diskMgr\n");
		return false;
	}

	if (!m_smbiosMgr.Execute()) {
		err("Failed to Execute m_smbiosMgr\n");
		return false;
	}
	/*
	if(!m_gpuMgr.Execute()) {
		err("Failed to Execute m_gpuMgr!\n");
		return false;
	}
	*/
	return true;
}
