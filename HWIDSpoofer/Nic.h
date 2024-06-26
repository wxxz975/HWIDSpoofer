#pragma once

#include <ntifs.h>
#include <ntddndis.h> // 这个必须放ntifs.h后面

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
		UCHAR Header[8]; // 0x8
		struct _NDIS_FILTER_BLOCK* NextFilter; // 0x8
		PVOID FilterDriver;	// 0x8
		PVOID FilterModuleContext; // 0x8
		PVOID Miniport; // 0x8
		PKSTRING FilterInstanceName; // 0x8
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
		NDIS_OBJECT_HEADER Header;		// 这里存在内存对齐，占用8字节
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

		/// <summary>
		/// 两种思路。
		/// 一个是通过查找m_ndisGlobalFilterList 过滤器链表查找到每个驱动设备的实例名称，然后使用IoGetDeviceObjectPointer获取设备指针
		/// 然后使用进行Hook DeviceControl 或则直接修改Mac地址(直接修改Mac地址目前存在值异常)
		/// 第二个方法是直接找ndisMiniDriverList驱动链表，通过遍历驱动链表，直接进行Hook
		/// </summary>
		/// <returns></returns>
		bool ChangeMacAddress() const;

	private:
		PVOID m_ndisBase = nullptr;

		/// <summary>
		/// ndisGlobalFilterList是一个全局的链表变量,用于维护系统中已注册的 NDIS 过滤器驱动程序的列表。
		/// </summary>
		PVOID m_ndisGlobalFilterList = nullptr;

		PVOID m_ndisDummyIrpHandler = nullptr;

		PVOID m_ndisMiniDriverList = nullptr;

		PVOID m_originalDispatch = nullptr;
	};

}



