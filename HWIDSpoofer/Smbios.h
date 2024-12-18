#pragma once
#include <ntifs.h>
#include "HWIDChanger.h"

//
// Lastest Reference: https://www.dmtf.org/standards/smbios
// //


namespace Smbios
{
	NTSTATUS ChangeSmbiosSerials();


	typedef struct _GUID {
		unsigned long  Data1;		// 4
		unsigned short Data2;		// 2
		unsigned short Data3;		// 2
		unsigned char  Data4[8];	// 8
	} GUID;

	typedef struct _BOOT_ENVIRONMENT_INFORMATION {
		GUID  BootIdentifier; // need this
		FIRMWARE_TYPE FirmwareType;
		// ....
	} BOOT_ENVIRONMENT_INFORMATION, * PBOOT_ENVIRONMENT_INFORMATION;


	enum SMBIOS_INFO_TYPE: UINT8
	{
		BIOS_INFO = 0,	// bios information
		SYSTEM_INFO = 1,	// system information
		BASEBOARD_INFO = 2,
		SYSTEM_ENCLOSURE = 3,	
		PROCESSOR_INFO = 4,	// processor information
		CACHE_INFO = 7,		// cache information
		SYSTEM_SLOTS = 9,
		PHYSICAL_MEMORY_ARRAY = 16,
		MEMORY_DEVICE = 17, // memory device
		MEMORY_ARRAY_MAPPED_ADDR = 19,
		SYSTEM_BOOT_INFO = 32   // boot information
	};

	// ------------------------------------------------
	// SMBIOS / gnu-efi
	// ------------------------------------------------
	typedef struct BIOS_HEADER
	{
		SMBIOS_INFO_TYPE	Type;
		UINT8				Length;
		UINT8				Handle[2];
	} SMBIOS_HEADER;

	typedef UINT8   SMBIOS_STRING;

	// reference: https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.6.0WIP50.pdf
	typedef struct _SMBIOS_TYPE0
	{
		SMBIOS_HEADER   Hdr;
		SMBIOS_STRING   Vendor;
		SMBIOS_STRING   BiosVersion;
		UINT8           BiosSegment[2]; // BIOS Starting Address Segment
		SMBIOS_STRING   BiosReleaseDate;
		UINT8           BiosRomSize;
		UINT8           BiosCharacteristics[8];
		// ....
	} BIOSInfo,*PBIOSInfo; // BIOS Information


	typedef struct _SMBIOS_TYPE1 {
		SMBIOS_HEADER	Header;
		SMBIOS_STRING	Manufacturer;
		SMBIOS_STRING	ProductName;
		SMBIOS_STRING	Version;
		SMBIOS_STRING	SerialNumber;
		UINT8			UUID[16];
		UINT8			WakeUpType;
		SMBIOS_STRING	SKUNumber;
		UINT8			Family;
	} SystemInfo, * PSystemInfo;

	typedef struct _SMBIOS_TYPE2 {
		SMBIOS_HEADER	Header;
		SMBIOS_STRING	Manufacturer;
		SMBIOS_STRING	ProductName;
		SMBIOS_STRING	Version;
		SMBIOS_STRING	SerialNumber;
		SMBIOS_STRING	AssetTag;
		UINT8			FeatureFlags;
		SMBIOS_STRING	LocationInChassis;
		UINT16			ChassisHandle;
		UINT8			BoardType;
		UINT8			NumObjHandle;
		UINT16*			pObjHandle;
	} BoardInfo, *PBoardInfo;

	typedef struct _SMBIOS_TYPE3
	{
		SMBIOS_HEADER   Hdr;
		SMBIOS_STRING   Manufacturer;
		UINT8           Type;
		SMBIOS_STRING   Version;
		SMBIOS_STRING   SerialNumber;
		SMBIOS_STRING   AssetTag;
		UINT8           BootupState;
		UINT8           PowerSupplyState;
		UINT8           ThermalState;
		UINT8           SecurityStatus;
		UINT8           OemDefined[4];
	} SystemEnc, *PSystemEnc;	// System Enclosure or Chassis

	typedef struct _SMBIOS_TYPE4
	{
		SMBIOS_HEADER   Hdr;
		SMBIOS_STRING   SocketDesignation;
		UINT8           ProcessorType;
		UINT8           ProcessorFamily;
		SMBIOS_STRING   ProcessorManufacturer; // ->
		UINT8           ProcessorId[8];			// -> change this 
		SMBIOS_STRING   ProcessorVersion;
		UINT8           Voltage;
		UINT8           ExternalClock[2];
		UINT8           MaxSpeed[2];
		UINT8           CurrentSpeed[2];
		UINT8           Status;
		UINT8           ProcessorUpgrade;
		UINT8           L1CacheHandle[2];
		UINT8           L2CacheHandle[2];
		UINT8           L3CacheHandle[2];
		SMBIOS_STRING	SerialNumber;		// -> change this
		SMBIOS_STRING	AssetTag;
		SMBIOS_STRING	PartNumber;
		// ....
	} ProcessorInfo, *PProcessorInfo; // Processor Information

	
	typedef struct _SMBIOS_TYPE17
	{
		SMBIOS_HEADER   Hdr;
		UINT8			PhysicalMemoryArrayHandle[2];
		UINT8			MemoryErrorInformationHandle[2];
		UINT8			TotalWidth[2];
		UINT8			DataWidth[2];
		UINT8			Size[2];
		UINT8			FormFactor;
		UINT8			DeviceSet;
		SMBIOS_STRING	DeviceLocator;
		SMBIOS_STRING	BankLocator;
		UINT8			MemoryType;
		UINT8			TypeDetail[2];
		UINT8			Speed[2];
		SMBIOS_STRING	Manufacturer;
		SMBIOS_STRING	SerialNumber;
	}MemoryDevice, *PMemoryDevice;
	

	/*
		typedef struct _SMBIOS_TYPE45 {
			SMBIOS_HEADER   Hdr;
			STRING FirmwareComponentName;
			STRING FirmwareVersion;
			BYTE VersionFormat;
			BYTE FirmwareID;
			BYTE FirmwareIDFormat;
			BYTE ReleaseDate;
			BYTE Manufacturer;
		} FirmwareInventoryInfo, * PFirmwareInventoryInfo;
	*/

	class SmbiosManager : public HWIDChanger
	{
	public:
		SmbiosManager() = default;
		~SmbiosManager() = default;

		bool Initialize() override;

		bool Execute() override;

		void ShowAllAddress() override;
	private:
		bool ChangeSmbiosSerials();
		bool ChangeBootInfo() const;

	private:
		NTSTATUS ProcessTable(SMBIOS_HEADER* header);
		NTSTATUS LoopTables(void* mapped, ULONG size);
		char* GetString(SMBIOS_HEADER* header, SMBIOS_STRING string);
	private:
		PVOID m_ntoskrnlBase = nullptr;
		PPHYSICAL_ADDRESS m_WmipSMBiosTablePhysicalAddress = nullptr;
		PULONG m_WmipSMBiosTableLengthAddr = nullptr;
		ULONG m_WmipSMBiosTableLength = 0;

		PBOOT_ENVIRONMENT_INFORMATION m_ExpBootEnvironmentInformation = nullptr;
	};
}


