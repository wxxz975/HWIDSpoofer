#pragma once
#include <ntifs.h>
#include "Shared.h"
#include "HWIDChanger.h"
namespace Disk
{
	PDEVICE_OBJECT GetRaidDevice(const wchar_t* deviceName);
	NTSTATUS DiskLoop(PDEVICE_OBJECT deviceArray, RaidUnitRegisterInterfaces registerInterfaces);
	NTSTATUS ChangeDiskSerials();
	NTSTATUS DisableSmart();
	void DisableSmartBit(PRAID_UNIT_EXTENSION extension);

	//VOID FuckDiskDiskpatch();
	
	class DiskManager : public HWIDChanger
	{
		/// <summary>
		/// 2024/5/8 Valid the 22h2 works fine
		/// </summary>
	public:
		DiskManager() = default;
		~DiskManager() = default;

		bool Initialize() override;

		bool Execute() override;

		void ShowAllAddress() override;

	private:
		bool ChangeDiskSerials();
		bool DisableSmart();

	private:
		PVOID m_storportBase = nullptr;
		RaidUnitRegisterInterfaces m_RaidUnitRegisterInterfaces = nullptr;

		PVOID m_diskBase = nullptr;
		DiskEnableDisableFailurePrediction m_DiskEnableDisableFailurePrediction = nullptr;
	};
	
};


