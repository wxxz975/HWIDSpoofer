#include "Smbios.h"

#include "Logger.hpp"
#include "Utils.h"

#define __LOOP_TABLE__
namespace Smbios
{
	/*
	// reference: DSP0134_3.6.0WIP50.pdf
	NTSTATUS ProcessTable(SMBIOS_HEADER* header)
	{
		if (!header->Length)
			return STATUS_UNSUCCESSFUL;

		if (header->Type == 0)
		{
			auto* type0 = reinterpret_cast<SMBIOS_TYPE0*>(header);

			auto* vendor = GetString(header, type0->Vendor);
			RandomizeString(vendor);
		}

		if (header->Type == 1)
		{
			auto* type1 = reinterpret_cast<SMBIOS_TYPE1*>(header);

			auto* manufacturer = GetString(header, type1->Manufacturer);
			RandomizeString(manufacturer);

			auto* productName = GetString(header, type1->ProductName);
			RandomizeString(productName);

			auto* serialNumber = GetString(header, type1->SerialNumber);
			RandomizeString(serialNumber);
		}

		if (header->Type == 2)
		{
			auto* type2 = reinterpret_cast<SMBIOS_TYPE2*>(header);

			auto* manufacturer = GetString(header, type2->Manufacturer);
			RandomizeString(manufacturer);

			auto* productName = GetString(header, type2->ProductName);
			RandomizeString(productName);

			auto* serialNumber = GetString(header, type2->SerialNumber);
			RandomizeString(serialNumber);
		}

		if (header->Type == 3)
		{
			auto* type3 = reinterpret_cast<SMBIOS_TYPE3*>(header);

			auto* manufacturer = GetString(header, type3->Manufacturer);
			RandomizeString(manufacturer);

			auto* serialNumber = GetString(header, type3->SerialNumber);
			RandomizeString(serialNumber);
		}

		return STATUS_SUCCESS;
	}

	NTSTATUS LoopTables(void* mapped, ULONG size)
	{
		auto* endAddress = static_cast<char*>(mapped) + size;
		while (true)
		{
			auto* header = static_cast<SMBIOS_HEADER*>(mapped);
			if (header->Type == 127 && header->Length == 4)
				break;

			ProcessTable(header);
			auto* end = static_cast<char*>(mapped) + header->Length;
			while (0 != (*end | *(end + 1))) end++;
			end += 2;
			if (end >= endAddress)
				break;

			mapped = end;
		}

		return STATUS_SUCCESS;
	}*/


	NTSTATUS ChangeSmbiosSerials()
	{
		auto* base = Utils::GetModuleBase("ntoskrnl.exe");
		if (!base)
		{
			err("Failed to find ntoskrnl.sys base!\n");
			return STATUS_UNSUCCESSFUL;
		}
		log("ntoskrnl.exe base:%llp\n", base);


		/*  //加上这一块会把Mac地址变为乱码
		PVOID ExpBootEnvironmentInformationPtr = Utils::FindPatternImage(base, "\x0F\x10\x05\x00\x00\x00\x00\x0F\x11\x00\x8B", "xxx????xx?x");

		Log::Print("ExpBootEnvironmentInformation ptr: %llp\n", ExpBootEnvironmentInformationPtr);
		if (ExpBootEnvironmentInformationPtr) {
			auto* ExpBootEnvironmentInformation = (*(PLONG)((PBYTE)ExpBootEnvironmentInformationPtr + 3) + 7 + (PBYTE)ExpBootEnvironmentInformationPtr);
			Log::Print("ExpBootEnvironmentInformation: %llp\n", ExpBootEnvironmentInformation);
			ULONG64 time = 0;

			KeQuerySystemTime(&time);
			Utils::SpoofBuffer(time, ExpBootEnvironmentInformation, 16);
		}*/


		/*
		fffff800`1d733111 895c2420        mov     dword ptr [rsp+20h],ebx
		fffff800`1d733115 488b0decdf3f00  mov     rcx,qword ptr [nt!WmipSMBiosTablePhysicalAddress (fffff800`1db31108)]
		fffff800`1d73311c 4885c9          test    rcx,rcx
		fffff800`1d73311f 742c            je      nt!WmipFindSMBiosStructure+0x85 (fffff800`1d73314d)
		fffff800`1d733121 8b151ddf3f00    mov     edx,dword ptr [nt!WmipSMBiosTableLength (fffff800`1db31044)]
		fffff800`1d733127 448d4304        lea     r8d,[rbx+4]
		*/
		auto* WmipSMBiosTablePhysicalAddressCall = static_cast<PPHYSICAL_ADDRESS>(Utils::FindPatternImage(base, "\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x74\x00\x8b\x15", "xxx????xxxx?xx")); // WmipFindSMBiosStructure -> WmipSMBiosTablePhysicalAddress
		if (!WmipSMBiosTablePhysicalAddressCall)
		{
			err("Failed to find SMBIOS physical address!\n");
			return STATUS_UNSUCCESSFUL;
		}

		
		auto* WmipSMBiosTablePhysicalAddress = Utils::translateAddress<PPHYSICAL_ADDRESS>(WmipSMBiosTablePhysicalAddressCall, 7);
		if (!WmipSMBiosTablePhysicalAddress)
		{
			err("Physical address is null!\n");
			return STATUS_UNSUCCESSFUL;
		}
		log("WmipSMBiosTablePhysicalAddress:0x%llp\n", WmipSMBiosTablePhysicalAddress);


		/*
		* WmipFindSMBiosStructure + 0x98
		fffff802`0fb3c160 8b15dede3f00    mov     edx,dword ptr [nt!WmipSMBiosTableLength (fffff802`0ff3a044)]
		fffff802`0fb3c166 4803d1          add     rdx,rcx
		fffff802`0fb3c169 c7442420010000c0 mov     dword ptr [rsp+20h],0C0000001h
		fffff802`0fb3c171 483bca          cmp     rcx,rdx
		fffff802`0fb3c174 7339            jae     nt!WmipFindSMBiosStructure+0xe7 (fffff802`0fb3c1af)
		fffff802`0fb3c176 443821          cmp     byte ptr [rcx],r12b
		*/
		auto* WmipSMBiosTableLengthPtr = Utils::FindPatternImage(base, "\x8B\x15\x00\x00\x00\x00\x48\x03\xD1\xC7\x44\x24\x00\x00\x00\x00\x00\x48\x3B\xCA\x73", "xx????xxxxxx?????xxxx");  // WmipFindSMBiosStructure -> WmipSMBiosTableLength
		log("WmipSMBiosTableLength ptr:%llp\n", WmipSMBiosTableLengthPtr);
		if (!WmipSMBiosTableLengthPtr)
		{
			err("Failed to find SMBIOS size!\n");
			return STATUS_UNSUCCESSFUL;
		}

		LONG WmipSMBiosTableLength = *Utils::translateAddress<PLONG>(WmipSMBiosTableLengthPtr, 6);
		//const auto WmipSMBiosTableLength = *(PLONG)(*((PLONG)((PBYTE)WmipSMBiosTableLengthPtr + 2)) + (PBYTE)WmipSMBiosTableLengthPtr + 6);

		if (!WmipSMBiosTableLength)
		{
			err("SMBIOS size is null!\n");
			return STATUS_UNSUCCESSFUL;
		}

		log("WmipSMBiosTableLength:%lx\n", WmipSMBiosTableLength);

		//return 0;

#ifdef __LOOP_TABLE__
		auto* mapped = MmMapIoSpace(*WmipSMBiosTablePhysicalAddress, WmipSMBiosTableLength, MmNonCached);
		if (!mapped)
		{
			log("Failed to map SMBIOS structures!\n");
			return STATUS_UNSUCCESSFUL;
		}

		//LoopTables(mapped, WmipSMBiosTableLength);

		MmUnmapIoSpace(mapped, WmipSMBiosTableLength);

#else // zero   查询smbios的时候会发现找不到实例

		memset(WmipSMBiosTablePhysicalAddress, 0, sizeof(PPHYSICAL_ADDRESS));
#endif
		log("ChangeSmbiosSerials Status:Success\n");
		return STATUS_SUCCESS;
	}



	bool SmbiosManager::Initialize()
	{
		m_ntoskrnlBase = Utils::GetModuleBase("ntoskrnl.exe");
		if (m_ntoskrnlBase == nullptr) {
			log("Failed to find m_ntoskrnlBase!");
			return false;
		}

		/*
			2: kd> u WmipFindSMBiosStructure  L20
			nt!WmipFindSMBiosStructure:
			fffff805`30134d68 48895c2408      mov     qword ptr [rsp+8],rbx
			fffff805`30134d6d 4c894c2420      mov     qword ptr [rsp+20h],r9
			fffff805`30134d72 4c89442418      mov     qword ptr [rsp+18h],r8
			fffff805`30134d77 56              push    rsi
			fffff805`30134d78 57              push    rdi
			fffff805`30134d79 4154            push    r12
			fffff805`30134d7b 4156            push    r14
			fffff805`30134d7d 4157            push    r15
			fffff805`30134d7f 4883ec30        sub     rsp,30h
			fffff805`30134d83 4d8bf1          mov     r14,r9
			fffff805`30134d86 498bf0          mov     rsi,r8
			fffff805`30134d89 4c8bfa          mov     r15,rdx
			fffff805`30134d8c 448ae1          mov     r12b,cl
			fffff805`30134d8f 65488b042588010000 mov   rax,qword ptr gs:[188h]
			fffff805`30134d98 66ff88e4010000  dec     word ptr [rax+1E4h]
			fffff805`30134d9f b201            mov     dl,1
			fffff805`30134da1 488d0dd8512e00  lea     rcx,[nt!WmipSMBiosLock (fffff805`30419f80)]
			fffff805`30134da8 e863c58eff      call    nt!ExAcquireResourceSharedLite (fffff805`2fa21310)
			fffff805`30134dad 33db            xor     ebx,ebx
			fffff805`30134daf 8bfb            mov     edi,ebx
			fffff805`30134db1 895c2420        mov     dword ptr [rsp+20h],ebx
			fffff805`30134db5 488b0d44833f00  mov     rcx,qword ptr [nt!WmipSMBiosTablePhysicalAddress (fffff805`3052d100)]
			fffff805`30134dbc 4885c9          test    rcx,rcx
			fffff805`30134dbf 742c            je      nt!WmipFindSMBiosStructure+0x85 (fffff805`30134ded)
			fffff805`30134dc1 8b157d823f00    mov     edx,dword ptr [nt!WmipSMBiosTableLength (fffff805`3052d044)]
			fffff805`30134dc7 448d4304        lea     r8d,[rbx+4]
		*/
		PVOID WmipSMBiosTablePhysicalAddressCalled = Utils::FindPatternImage(m_ntoskrnlBase,
			"\x48\x8b\x0d\x00\x00\x00\x00\x48\x85\xc9\x74\x00\x8b\x15\x00\x00\x00\x00\x44\x8d\x43",
			"xxx????xxxx?xx????xxx");
		if (WmipSMBiosTablePhysicalAddressCalled == nullptr) {
			log("Failed to find the WmipSMBiosTablePhysicalAddressCalled!");
			return false;
		}

		m_WmipSMBiosTablePhysicalAddress = Utils::translateAddress<PPHYSICAL_ADDRESS>(WmipSMBiosTablePhysicalAddressCalled, 7);

		PVOID WmipSMBiosTableLengthCalled = Utils::reinterpret<PVOID>(WmipSMBiosTablePhysicalAddressCalled, 12);
		if (WmipSMBiosTableLengthCalled == nullptr) {
			log("Failed to find the WmipSMBiosTableLengthCalled!");
			return false;
		}
		m_WmipSMBiosTableLengthAddr = Utils::translateAddress<PULONG>(WmipSMBiosTableLengthCalled, 6);
		m_WmipSMBiosTableLength = *m_WmipSMBiosTableLengthAddr;


		return false;
	}

	bool SmbiosManager::Execute()
	{
		if (m_ntoskrnlBase == nullptr ||
			m_WmipSMBiosTableLengthAddr == nullptr ||
			m_WmipSMBiosTablePhysicalAddress == nullptr || m_WmipSMBiosTableLength == 0)
		{
			log("Please Init SmbiosManager Module first!");
			ShowAllAddress();
			return false;
		}




		return false;
	}

	void SmbiosManager::ShowAllAddress()
	{
		log("\n m_ntoskrnlBase:%llx\n m_WmipSMBiosTablePhysicalAddress:%llx\n m_WmipSMBiosTableLengthAddr:%llx\n m_WmipSMBiosTableLength:%lx",
			m_ntoskrnlBase, m_WmipSMBiosTablePhysicalAddress, m_WmipSMBiosTableLengthAddr, m_WmipSMBiosTableLength);
	}

}
