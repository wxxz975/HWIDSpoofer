#pragma once
#include <ntifs.h>
#include "Shared.h"
#include "HWIDChanger.h"
namespace Disk
{
	typedef struct _TELEMETRY_UNIT_EXTENSION
	{
		/*
				+0x000 Flags            : <anonymous-tag>
				+0x000 DeviceHealthEventsLogged : Pos 0, 1 Bit
				+0x000 FailedFirstSMARTCommand : Pos 1, 1 Bit
				+0x000 FailedFirstDeviceStatisticsLogCommand : Pos 2, 1 Bit
				+0x000 FailedFirstNvmeCloudSSDCommand : Pos 3, 1 Bit
				+0x000 FailedFirstGetSupportedLogPagesCommand : Pos 4, 1 Bit
				+0x000 SmartPredictFailure : Pos 5, 1 Bit
				+0x000 Reserved         : Pos 6, 26 Bits
				+0x004 NvmeCriticalWarning : UChar
				+0x005 DeviceHealthPollCount : UChar
			 */
			int Flags;
		} TELEMETRY_UNIT_EXTENSION, * PTELEMETRY_UNIT_EXTENSION;

		// ------------------------------------------------
		// storport.sys
		// ------------------------------------------------
		typedef struct _STOR_SCSI_IDENTITY
		{
			char Space[0x8]; // +0x008 SerialNumber     : _STRING
			STRING SerialNumber;
		} STOR_SCSI_IDENTITY, * PSTOR_SCSI_IDENTITY;

		// lkd> dt storport!_RAID_UNIT_EXTENSION -b
		typedef struct _RAID_UNIT_EXTENSION
		{
			union
		{
			struct
			{
				char padding[0x68]; // +0x068 Identity         : _STOR_SCSI_IDENTITY
				STOR_SCSI_IDENTITY Identity;
			} _Identity;

			struct
			{
				char padding[0x7c8]; // +0x7c8 TelemetryExtension : _TELEMETRY_UNIT_EXTENSION
				TELEMETRY_UNIT_EXTENSION TelemetryExtension;
			} _Smart;
		};
	} RAID_UNIT_EXTENSION, * PRAID_UNIT_EXTENSION;


	/// ------------------------------------------------
	/// storport!RaidUnitRegisterInterfaces
	/// ------------------------------------------------
	typedef __int64(__fastcall* RaidUnitRegisterInterfaces)(PRAID_UNIT_EXTENSION a1);

	// ------------------------------------------------
	// disk.sys
	// ------------------------------------------------
	typedef NTSTATUS(__fastcall* DiskEnableDisableFailurePrediction)(void* a1, bool a2);
	

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
		bool ChangeDiskSerials() const;
		bool DisableSmart() const;
		void DisableSmartBit(PRAID_UNIT_EXTENSION extension) const;

	private:
		PDEVICE_OBJECT GetRaidDevice(const wchar_t* deviceName) const;
		NTSTATUS DiskLoop(PDEVICE_OBJECT deviceArray, RaidUnitRegisterInterfaces registerInterfaces) const;
	
	private:
		PVOID m_storportBase = nullptr;
		RaidUnitRegisterInterfaces m_RaidUnitRegisterInterfaces = nullptr;

		PVOID m_diskBase = nullptr;
		DiskEnableDisableFailurePrediction m_DiskEnableDisableFailurePrediction = nullptr;
	};
	
};


