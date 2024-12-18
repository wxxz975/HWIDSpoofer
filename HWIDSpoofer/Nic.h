#pragma once

#include <ntifs.h>
#include <ntddndis.h> // ��������ntifs.h����

#include "HWIDChanger.h"

#define IOCTL_NSI_PROXY_ARP (0x0012001B)
#define NSI_PARAMS_ARP (11)


namespace Nic
{
	/*
		win32k!_GUID
	   +0x000 Data1            : Uint4B
	   +0x004 Data2            : Uint2B
	   +0x006 Data3            : Uint2B
	   +0x008 Data4            : [8] UChar
	*/
	typedef struct _GUID {
		UINT32 data1;
		USHORT data2;
		USHORT data3;
		UCHAR  data4[8];
	}GUID, *PGUID;


	typedef struct _KSTRING {
		char _padding_0[0x10];
		WCHAR Buffer[1]; // 0x10 at least
	} KSTRING, * PKSTRING;

	typedef struct _NDIS_OBJECT_HEADER
	{
		UCHAR	Type;
		UCHAR	Revision;
		USHORT	Size;
	}NDIS_OBJECT_HEADER, * PNDIS_OBJECT_HEADER;
	
	// dt ndis!_NDIS_IF_BLOCK
	typedef struct _NDIS_IF_BLOCK {
		char _padding_0[0x464];
		IF_PHYSICAL_ADDRESS_LH ifPhysAddress;
		IF_PHYSICAL_ADDRESS_LH PermanentPhysAddress; 
	} NDIS_IF_BLOCK, * PNDIS_IF_BLOCK;


	/*
		ndis!_NDIS_FILTER_BLOCK
		   +0x000 Header           : _NDIS_OBJECT_HEADER
		   +0x008 NextFilter       : Ptr64 _NDIS_FILTER_BLOCK
		   +0x010 FilterDriver     : Ptr64 _NDIS_FILTER_DRIVER_BLOCK
		   +0x018 FilterModuleContext : Ptr64 Void
		   +0x020 Miniport         : Ptr64 _NDIS_MINIPORT_BLOCK
		   +0x028 FilterInstanceName : wistd::unique_ptr<Rtl::KString,wistd::default_delete<Rtl::KString> > 0x30
		   ...

	*/
	// dt ndis!_NDIS_FILTER_BLOCK

	typedef struct _NDIS_FILTER_BLOCK {
		UCHAR Header[8]; // 0x0
		struct _NDIS_FILTER_BLOCK* NextFilter; // 0x8
		PVOID FilterDriver;	// 0x10
		PVOID FilterModuleContext; // 0x18
		PVOID Miniport; // 0x20
		PKSTRING FilterInstanceName; // 0x28
		UCHAR _padding_2[0x288]; 
		PNDIS_IF_BLOCK* IfBlock;
	} NDIS_FILTER_BLOCK, * PNDIS_FILTER_BLOCK;


	typedef struct _IOC_REQUEST {
		PVOID Buffer;
		ULONG BufferLength;
		PVOID OldContext;
		PIO_COMPLETION_ROUTINE OldRoutine;
	} IOC_REQUEST, * PIOC_REQUEST;

	

	/*
		ndis!_NDIS_M_DRIVER_BLOCK
		+0x000 Header           : _NDIS_OBJECT_HEADER
		   +0x008 NextDriver       : Ptr64 _NDIS_M_DRIVER_BLOCK
		   +0x010 MiniportQueue    : Ptr64 _NDIS_MINIPORT_BLOCK
		   +0x018 MajorNdisVersion : UChar
		   +0x019 MinorNdisVersion : UChar
		   +0x01a Flags            : Uint2B
		   +0x020 NdisDriverInfo   : Ptr64 _NDIS_WRAPPER_HANDLE
		   +0x028 DriverObject     : Ptr64 _DRIVER_OBJECT
		   +0x030 ServiceRegPath   : _UNICODE_STRING
		*/
	typedef struct _NDIS_M_DRIVER_BLOCK {
		NDIS_OBJECT_HEADER Header;		
		struct _NDIS_M_DRIVER_BLOCK* NextDriver;
		struct _NDIS_M_DRIVER_BLOCK* MiniportQueue;
		UCHAR MajorNdisVersion;
		UCHAR MinorNdisVersion;
		USHORT Flags;
		struct NDIS_WRAPPER_HANDLE* NdisDriverInfo;
		PDRIVER_OBJECT DriverObject;
		UNICODE_STRING ServiceRegPath;
	}NDIS_M_DRIVER_BLOCK, * PNDIS_M_DRIVER_BLOCK;


	//NTSTATUS SpoofNIC();
	
	class NICManager : public HWIDChanger
	{
	public:
		NICManager() = default;
		~NICManager() = default;

		bool Initialize() override;

		bool Execute() override;

		void ShowAllAddress() override;

	private:

		bool ChangeMacAddress() const;

	private:
		PVOID m_ndisBase = nullptr;

		PVOID m_ndisGlobalFilterList = nullptr;

		PVOID m_ndisDummyIrpHandler = nullptr;

		PVOID m_ndisMiniDriverList = nullptr;

		PVOID m_originalDispatch = nullptr;
	};

}



