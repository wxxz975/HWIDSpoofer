#pragma once

#include <ntifs.h>
#include <ntddndis.h> // 这个必须放ntifs.h后面

#include "HWIDChanger.h"

#define IOCTL_NSI_PROXY_ARP (0x0012001B)
#define NSI_PARAMS_ARP (11)


namespace Nic
{

	typedef struct _NIC_DRIVER {
		PDRIVER_OBJECT DriverObject;
		PDRIVER_DISPATCH Original;
	} NIC_DRIVER, * PNIC_DRIVER;



	typedef struct _NSI_PARAMS {
		char _padding_0[0x18];
		ULONG Type; // 0x18
	} NSI_PARAMS, * PNSI_PARAMS;



	typedef struct _KSTRING {
		char _padding_0[0x10];
		WCHAR Buffer[1]; // 0x10 at least
	} KSTRING, * PKSTRING;

	
	/*
			ndis!_IF_PHYSICAL_ADDRESS_LH
			+0x000 Length           : Uint2B
			+0x002 Address          : [32] UChar

	typedef struct _IF_PHYSICAL_ADDRESS_LH
	{
		USHORT Length;
		UCHAR  Address[32];
	}IF_PHYSICAL_ADDRESS_LH, * PIF_PHYSICAL_ADDRESS_LH;*/

	// dt ndis!_NDIS_IF_BLOCK
	typedef struct _NDIS_IF_BLOCK {
		char _padding_0[0x464];
		IF_PHYSICAL_ADDRESS_LH ifPhysAddress; // 0x464
		IF_PHYSICAL_ADDRESS_LH PermanentPhysAddress; // 0x486
	} NDIS_IF_BLOCK, * PNDIS_IF_BLOCK;


	/*
		+0x000 Header           : _NDIS_OBJECT_HEADER
	   +0x008 NextFilter       : Ptr64 _NDIS_FILTER_BLOCK
	   +0x010 FilterDriver     : Ptr64 _NDIS_FILTER_DRIVER_BLOCK
	   +0x018 FilterModuleContext : Ptr64 Void
	   +0x020 Miniport         : Ptr64 _NDIS_MINIPORT_BLOCK
	   +0x028 FilterInstanceName : wistd::unique_ptr<Rtl::KString,wistd::default_delete<Rtl::KString> >
	   +0x030 FilterFriendlyName : Ptr64 _UNICODE_STRING
	   +0x038 Flags            : Uint4B
	*/
	// dt ndis!_NDIS_FILTER_BLOCK

	typedef struct _NDIS_FILTER_BLOCK {
		UCHAR Header[8];
		struct _NDIS_FILTER_BLOCK* NextFilter; // 0x8
		UCHAR _padding_1[0x18];	//0x10
		PKSTRING FilterInstanceName; // 0x28
		UCHAR _padding_2[0x290];
		PNDIS_IF_BLOCK IfBlock;
	} NDIS_FILTER_BLOCK, * PNDIS_FILTER_BLOCK;


	typedef struct _IOC_REQUEST {
		PVOID Buffer;
		ULONG BufferLength;
		PVOID OldContext;
		PIO_COMPLETION_ROUTINE OldRoutine;
	} IOC_REQUEST, * PIOC_REQUEST;

	typedef struct _NDIS_OBJECT_HEADER
	{
		UCHAR	Type;
		UCHAR	Revision;
		USHORT	Size;
	}NDIS_OBJECT_HEADER, * PNDIS_OBJECT_HEADER;

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


	NTSTATUS SpoofNIC();
	
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

		/// <summary>
		/// ndisGlobalFilterList是一个全局的链表变量,用于维护系统中已注册的 NDIS 过滤器驱动程序的列表。
		/// </summary>
		PVOID m_ndisGlobalFilterList = nullptr;

		PVOID m_ndisDummyIrpHandler = nullptr;
	};

}



