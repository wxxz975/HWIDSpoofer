#include "Disk.h"

#include <ntstrsafe.h>

#include "Logger.hpp"

#include "Utils.h"


namespace Disk
{
	extern "C" POBJECT_TYPE * IoDriverObjectType;
	/*this method need hook*/
	VOID FuckDiskDiskpatch()
	{
		PVOID base = Utils::GetModuleBase("classpnp.sys");
		if (!base) {
			err("failed to get ndis.sys base!\n");
			return;
		}
		/*
			ClassMpdevInternalDeviceControl
			fffff804`4a4653b0 4053            push    rbx
			fffff804`4a4653b2 4883ec20        sub     rsp,20h
			fffff804`4a4653b6 488b4140        mov     rax,qword ptr [rcx+40h]
			fffff804`4a4653ba 488bda          mov     rbx,rdx
			fffff804`4a4653bd 4c8bc1          mov     r8,rcx
			fffff804`4a4653c0 80b84602000000  cmp     byte ptr [rax+246h],0
			fffff804`4a4653c7 7457            je      CLASSPNP!ClassMpdevInternalDeviceControl+0x70 (fffff804`4a465420)
			fffff804`4a4653c9 488b0d30dc0000  mov     rcx,qword ptr [CLASSPNP!WPP_GLOBAL_Control (fffff804`4a473000)]
		*/
		PVOID scan = Utils::FindPatternImage(base, "\x40\x53\x48\x83\xEC\x00\x48\x8B\x41\x00\x48\x8B\xDA\x4C\x8B\xC1\x80\xB8\x00\x00\x00\x00\x00\x74\x57", "xxxxx?xxx?xxxxxxxx?????xx");

		UNICODE_STRING targetName;
		RtlInitUnicodeString(&targetName, L"\\Driver\\Disk");

		PDRIVER_OBJECT driverObject = nullptr;
		auto status = ObReferenceObjectByName(&targetName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, 0, *IoDriverObjectType, KernelMode, nullptr, reinterpret_cast<VOID**>(&driverObject));
		if (!NT_SUCCESS(status)) {
			err("failed to get Object from ObReferenceObjectByName!\n");
			return;
		}

		*reinterpret_cast<PVOID*>(&driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]) = scan;

		ObfDereferenceObject(driverObject);
	}


	bool DiskManager::Initialize()
	{
		m_storportBase = Utils::GetModuleBase("storport.sys");
		if (m_storportBase == nullptr) {
			err("Failed to find m_storportBase!");
			return false;
		}

		/*
			storport!RaidUnitRegisterInterfaces:
			fffff805`66ebbbd4 48895c2418      mov     qword ptr [rsp+18h],rbx
			fffff805`66ebbbd9 55              push    rbp
			fffff805`66ebbbda 56              push    rsi
			fffff805`66ebbbdb 57              push    rdi
			fffff805`66ebbbdc 4883ec50        sub     rsp,50h
			fffff805`66ebbbe0 8b4160          mov     eax,dword ptr [rcx+60h]
			fffff805`66ebbbe3 4c8d0de6fd0300  lea     r9,[storport!`string' (fffff805`66efb9d0)]
			fffff805`66ebbbea 488b5118        mov     rdx,qword ptr [rcx+18h]
			fffff805`66ebbbee 488bf9          mov     rdi,rcx
			fffff805`66ebbbf1 440fb6c0        movzx   r8d,al
			fffff805`66ebbbf5 33ed            xor     ebp,ebp
		*/
		auto RaidUnitRegisterInterfacesPtr = Utils::FindPatternImage(m_storportBase,
			"\x48\x89\x5c\x24\x00\x55\x56\x57\x48\x83\xec\x00\x8b\x41\x00\x4c\x8d",
			"xxxx?xxxxxx?xx?xx");
		if (RaidUnitRegisterInterfacesPtr == nullptr) {
			err("Failed to find m_RaidUnitRegisterInterfaces!");
			return false;
		}
		m_RaidUnitRegisterInterfaces = Utils::reinterpret<RaidUnitRegisterInterfaces>(RaidUnitRegisterInterfacesPtr, 0);

		m_diskBase = Utils::GetModuleBase("disk.sys");
		if (m_diskBase == nullptr) {
			err("Failed to find m_diskBase!");
			return false;
		}

		/*
		disk!DiskEnableDisableFailurePrediction:
		fffff805`67be00f4 4c8bdc          mov     r11,rsp
		fffff805`67be00f7 49895b10        mov     qword ptr [r11+10h],rbx
		fffff805`67be00fb 49897b18        mov     qword ptr [r11+18h],rdi
		fffff805`67be00ff 55              push    rbp
		fffff805`67be0100 498d6ba1        lea     rbp,[r11-5Fh]
		fffff805`67be0104 4881ec90000000  sub     rsp,90h
		fffff805`67be010b 488b054e8fffff  mov     rax,qword ptr [disk!_security_cookie (fffff805`67bd9060)]
		fffff805`67be0112 4833c4          xor     rax,rsp
		*/
		auto DiskEnableDisableFailurePredictionPtr = Utils::FindPatternImage(m_diskBase,
			"\x4c\x8b\xdc\x49\x89\x5b\x00\x49\x89\x7b\x00\x55\x49\x8d\x6b\xa1\x48\x81\xec",
			"xxxxxx?xxx?xxxxxxxx");
		if (DiskEnableDisableFailurePredictionPtr == nullptr) {
			err("Failed to find m_DiskEnableDisableFailurePrediction!");
			return false;
		}
		m_DiskEnableDisableFailurePrediction = Utils::reinterpret<DiskEnableDisableFailurePrediction>(DiskEnableDisableFailurePredictionPtr, 0);

		return true;
	}

	bool DiskManager::Execute()
	{
		if (m_storportBase == nullptr
			|| m_RaidUnitRegisterInterfaces == nullptr
			|| m_diskBase == nullptr || m_DiskEnableDisableFailurePrediction == nullptr)
		{
			log("Please init DiskManager Module first!");
			ShowAllAddress();
			return false;
		}

		if (!DisableSmart()) {
			err("Failed to DisableSmart!");
			return false;
		}
		
		if (!ChangeDiskSerials()) {
			err("Failed to ChangeDiskSerials!");
			return false;
		}

		return true;
	}

	void DiskManager::ShowAllAddress()
	{
		log("\n m_storportBase:%llx\n m_RaidUnitRegisterInterfaces:%llx\n m_diskBase:%llx\n m_DiskEnableDisableFailurePrediction:%llx\n",
			m_storportBase, m_RaidUnitRegisterInterfaces, m_diskBase, m_DiskEnableDisableFailurePrediction
		);
	}

	bool DiskManager::ChangeDiskSerials() const
	{
		auto status = STATUS_NOT_FOUND;
		for (auto i = 0; i < 2; i++)
		{
			const auto* raidFormat = L"\\Device\\RaidPort%d";
			wchar_t raidBuffer[18];
			RtlStringCbPrintfW(raidBuffer, 18 * sizeof(wchar_t), raidFormat, i);

			auto* device = GetRaidDevice(raidBuffer);
			if (!device)
				continue;
			
			const auto loopStatus = DiskLoop(device, m_RaidUnitRegisterInterfaces);
			if (NT_SUCCESS(loopStatus))
				status = loopStatus;
		}

		log("ChangeDiskSerials Status:%s\n", NT_SUCCESS(status) ? "Success" : "Fail");

		return NT_SUCCESS(status);
	}

	bool DiskManager::DisableSmart() const
	{
		UNICODE_STRING driverDisk;
		RtlInitUnicodeString(&driverDisk, L"\\Driver\\Disk");

		PDRIVER_OBJECT driverObject = nullptr;
		auto status = ObReferenceObjectByName(&driverDisk, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, 0, *IoDriverObjectType, KernelMode, nullptr, reinterpret_cast<PVOID*>(&driverObject));
		if (!NT_SUCCESS(status)) {
			err("Failed to get disk driver object!\n");
			return false;
		}

		PDEVICE_OBJECT deviceObjectList[64];
		RtlZeroMemory(deviceObjectList, sizeof(deviceObjectList));

		ULONG numberOfDeviceObjects = 0;
		status = IoEnumerateDeviceObjectList(driverObject, deviceObjectList, sizeof(deviceObjectList), &numberOfDeviceObjects);
		if (!NT_SUCCESS(status)) {
			ObDereferenceObject(driverObject);
			err("Failed to enumerate disk driver device object list!\n");
			return false;
		}

		for (ULONG i = 0; i < numberOfDeviceObjects; ++i) {
			auto* deviceObject = deviceObjectList[i];
			m_DiskEnableDisableFailurePrediction(deviceObject->DeviceExtension, false);
			ObDereferenceObject(deviceObject);
		}

		ObDereferenceObject(driverObject);
		log("Success to DisableSmart!\n");

		return true;
	}


	/**
	 * \brief Get pointer to device object of desired raid port of given name
	 * \param deviceName Name of the raid port (path ex. \\Device\\RaidPort0)
	 * \return Pointer to device object
	 */
	PDEVICE_OBJECT DiskManager::GetRaidDevice(const wchar_t* deviceName) const
	{
		UNICODE_STRING raidPort;
		RtlInitUnicodeString(&raidPort, deviceName);

		PFILE_OBJECT fileObject = nullptr;
		PDEVICE_OBJECT deviceObject = nullptr;
		auto status = IoGetDeviceObjectPointer(&raidPort, FILE_READ_DATA, &fileObject, &deviceObject);
		if (!NT_SUCCESS(status)) {
			return nullptr;
		}

		return deviceObject->DriverObject->DeviceObject; // not sure about this
	}

	/**
	 * \brief Loop through all devices in the array and change serials of any
	 * device of type FILE_DEVICE_DISK
	 * \param deviceArray Pointer to first device
	 * \param registerInterfaces Function from storport.sys to reset registry entries
	 * \return
	 */
	NTSTATUS DiskManager::DiskLoop(PDEVICE_OBJECT deviceArray, RaidUnitRegisterInterfaces registerInterfaces) const
	{
		auto status = STATUS_NOT_FOUND;

		while (deviceArray->NextDevice)
		{
			if (deviceArray->DeviceType == FILE_DEVICE_DISK)
			{
				auto* extension = static_cast<PRAID_UNIT_EXTENSION>(deviceArray->DeviceExtension);
				if (!extension)
					continue;
				//Telemetry
				const auto length = extension->_Identity.Identity.SerialNumber.Length;
				if (!length)
					continue;

				char original[256];
				RtlCopyMemory(original, extension->_Identity.Identity.SerialNumber.Buffer, length);
				//memcpy(original, extension->_Identity.Identity.SerialNumber.Buffer, length);
				original[length] = '\0';

				auto* buffer = static_cast<char*>(ExAllocatePoolWithTag(NonPagedPool, length, POOL_TAG));
				if (buffer == NULL) {
					continue;
				}
				buffer[length] = '\0';

				Utils::RandomText(buffer, length - 1);

				RtlInitString(&extension->_Identity.Identity.SerialNumber, buffer);

				log("Changed disk serial %s to %s.\n", original, buffer);

				status = STATUS_SUCCESS;
				ExFreePool(buffer);
				/*
				 * On some devices DiskEnableDisableFailurePrediction will fail
				 * Setting the bits directly will not fail and should work on any device
				 */
				DisableSmartBit(extension);

				registerInterfaces(extension);
			}

			deviceArray = deviceArray->NextDevice;
		}

		return status;
	}

	/**
	 * \brief Null smart mask in raid extension to disable S.M.A.R.T functionality
	 * \param extension Pointer to disk's raid unit extension
	 */
	void DiskManager::DisableSmartBit(PRAID_UNIT_EXTENSION extension) const 
	{
		extension->_Smart.TelemetryExtension.Flags = 0;
	}

};

