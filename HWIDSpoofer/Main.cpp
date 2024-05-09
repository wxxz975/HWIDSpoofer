#include <ntifs.h>

#include "Disk.h"
#include "Nic.h"
#include "Smbios.h"

#include "Logger.hpp"

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDrvObj, PUNICODE_STRING pRegistryPath)
{
	UNREFERENCED_PARAMETER(pDrvObj);
	UNREFERENCED_PARAMETER(pRegistryPath);
	//Disk::DisableSmart();
	//Disk::ChangeDiskSerials();
	//Nic::SpoofNIC();
	//Smbios::ChangeSmbiosSerials();
	DiskManager diskMgr;
	
	if (!diskMgr.Initialize()) {
		log("Failed to init diskMgr!"); 
	}
	diskMgr.ShowAllAddress();

	NICManager nicMgr;
	if (!nicMgr.Initialize()) {
		log("Failed to init nicMgr!");
	}
	nicMgr.ShowAllAddress();

	SmbiosManager smbiosMgr;
	if (!smbiosMgr.Initialize()) {
		log("Failed to init smbiosMgr!");
	}
	smbiosMgr.ShowAllAddress();

	return STATUS_SUCCESS;
}