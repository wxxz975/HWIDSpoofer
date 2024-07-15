#include <ntifs.h>

#include "Logger.hpp"
#include "HWIDSpoofer.h"


extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDrvObj, PUNICODE_STRING pRegistryPath)
{
	UNREFERENCED_PARAMETER(pDrvObj);
	UNREFERENCED_PARAMETER(pRegistryPath);

	HWIDSpoofer spoofer;

	if (spoofer.Initialize()) {
		spoofer.Execute();
		log("HWIDSpoofer Success!\n");
	}

	return STATUS_SUCCESS;
}