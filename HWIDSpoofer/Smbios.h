#pragma once
#include <ntifs.h>
#include "HWIDChanger.h"

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

	typedef PBOOT_ENVIRONMENT_INFORMATION(__fastcall* ExpBootEnvironmentInformation)(VOID);



	// ------------------------------------------------
	// SMBIOS / gnu-efi
	// ------------------------------------------------
	typedef struct
	{
		UINT8   Type;
		UINT8   Length;
		UINT8   Handle[2];
	} SMBIOS_HEADER;



	typedef UINT8   SMBIOS_STRING;

	// reference: https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.6.0WIP50.pdf
	typedef struct
	{
		SMBIOS_HEADER   Hdr;
		SMBIOS_STRING   Vendor;
		SMBIOS_STRING   BiosVersion;
		UINT8           BiosSegment[2]; // BIOS Starting Address Segment
		SMBIOS_STRING   BiosReleaseDate;
		UINT8           BiosRomSize;
		UINT8           BiosCharacteristics[8];
		// ....
	} SMBIOS_TYPE0; // BIOS Information

	typedef struct
	{
		SMBIOS_HEADER   Hdr;
		SMBIOS_STRING   Manufacturer;
		SMBIOS_STRING   ProductName;
		SMBIOS_STRING   Version;
		SMBIOS_STRING   SerialNumber;
		GUID			UUID[16];
		//
		// always byte copy this data to prevent alignment faults!
		//
		UINT8           WakeUpType;
		// .... 
	} SMBIOS_TYPE1; // System Information

	typedef struct
	{
		SMBIOS_HEADER   Hdr;
		SMBIOS_STRING   Manufacturer;
		SMBIOS_STRING   Product;
		SMBIOS_STRING   Version;
		SMBIOS_STRING   SerialNumber;
		SMBIOS_STRING	AssetTag;
		// ... 
	} SMBIOS_TYPE2; // Baseboard (or Module) Information

	typedef struct
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
	} SMBIOS_TYPE3;	// System Enclosure or Chassis

	typedef struct
	{
		SMBIOS_HEADER   Hdr;
		UINT8           SocketDesignation;
		UINT8           ProcessorType;
		UINT8           ProcessorFamily;
		SMBIOS_STRING   ProcessorManufacturer; // ->
		UINT8           ProcessorId[8];			// ->
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
		SMBIOS_STRING	SerialNumber;		// ->
		SMBIOS_STRING	AssetTag;
		SMBIOS_STRING	PartNumber;
		// ....
	} SMBIOS_TYPE4; // Processor Information

	/// Memory Controller Information (Type 5)

	enum {
		SMBIOSTYPE0 = 0x0,
		SMBIOSTYPE1 = 0x1,
		SMBIOSTYPE2 = 0x2,
		SMBIOSTYPE3 = 0x3,
		SMBIOSTYPE4 = 0x4
	};

	typedef union
	{
		SMBIOS_HEADER* Hdr;
		SMBIOS_TYPE0* Type0;
		SMBIOS_TYPE1* Type1;
		SMBIOS_TYPE2* Type2;
		SMBIOS_TYPE3* Type3;
		SMBIOS_TYPE4* Type4;
		UINT8* Raw;
	} SMBIOS_STRUCTURE_POINTER;

	typedef struct
	{
		UINT8   AnchorString[4];
		UINT8   EntryPointStructureChecksum;
		UINT8   EntryPointLength;
		UINT8   MajorVersion;
		UINT8   MinorVersion;
		UINT16  MaxStructureSize;
		UINT8   EntryPointRevision;
		UINT8   FormattedArea[5];
		UINT8   IntermediateAnchorString[5];
		UINT8   IntermediateChecksum;
		UINT16  TableLength;
		UINT32  TableAddress;
		UINT16  NumberOfSmbiosStructures;
		UINT8   SmbiosBcdRevision;
	} SMBIOS_STRUCTURE_TABLE;

	typedef struct _RAW_SMBIOS
	{
		UINT8	Used20CallingMethod;
		UINT8	MajorVersion;
		UINT8	MinorVersion;
		UINT8	DmiRevision;
		UINT32	Size;
		UINT8* SMBIOSTableData;
	} RAW_SMBIOS;




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

		ExpBootEnvironmentInformation m_ExpBootEnvironmentInformation = nullptr;
	};
}


