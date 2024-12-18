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


		/*  //������һ����Mac��ַ��Ϊ����
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
		auto* WmipSMBiosTablePhysicalAddressCall = static_cast<PPHYSICAL_ADDRESS>(Utils::FindPatternImage(base, 
			"\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x74\x00\x8b\x15", "xxx????xxxx?xx")); 
		// WmipFindSMBiosStructure -> WmipSMBiosTablePhysicalAddress
		
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

#else // zero   ��ѯsmbios��ʱ��ᷢ���Ҳ���ʵ��

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
			err("Failed to find the WmipSMBiosTableLengthCalled!");
			return false;
		}
		m_WmipSMBiosTableLengthAddr = Utils::translateAddress<PULONG>(WmipSMBiosTableLengthCalled, 6);
		m_WmipSMBiosTableLength = *m_WmipSMBiosTableLengthAddr;



		/*
		*	���ExpBootEnvironmentInformation �ṹ��nt!ExpQuerySystemInformation+0xc88 �ڲ����ù�
		* 
		* fffff804`6b7fa4c8 e9410e0000      jmp     nt!ExpQuerySystemInformation+0x2fae (fffff804`6b7fb30e)
			fffff804`6b7fa4cd e93c0e0000      jmp     nt!ExpQuerySystemInformation+0x2fae (fffff804`6b7fb30e)
			fffff804`6b7fa4d2 0f100547f16100  movups  xmm0,xmmword ptr [nt!ExpBootEnvironmentInformation (fffff804`6be19620)]
			fffff804`6b7fa4d9 0f1103          movups  xmmword ptr [rbx],xmm0
			fffff804`6b7fa4dc 8b054ef16100    mov     eax,dword ptr [nt!ExpBootEnvironmentInformation+0x10 (fffff804`6be19630)]
			fffff804`6b7fa4e2 894310          mov     dword ptr [rbx+10h],eax
			fffff804`6b7fa4e5 3b7c2430        cmp     edi,dword ptr [rsp+30h]
			fffff804`6b7fa4e9 0f82cbf5ffff    jb      nt!ExpQuerySystemInformation+0x175a (fffff804`6b7f9aba)
		*/

		PVOID CallExpBootEnvironmentInformation = Utils::FindPatternImage(m_ntoskrnlBase,
			"\x0f\x10\x05\x00\x00\x00\x00\x0f\x11\x03\x8b\x05",
			"xxx????xxxxx");
		m_ExpBootEnvironmentInformation = Utils::translateAddress<PBOOT_ENVIRONMENT_INFORMATION>(CallExpBootEnvironmentInformation, 7);
		if (m_ExpBootEnvironmentInformation == nullptr) {
			err("Failed to find m_ExpBootEnvironmentInformation!\n");
			return false;
		}

		return true;
	}

	bool SmbiosManager::Execute()
	{
		
		if (m_ntoskrnlBase == nullptr ||
			m_WmipSMBiosTableLengthAddr == nullptr ||
			m_WmipSMBiosTablePhysicalAddress == nullptr || m_WmipSMBiosTableLength == 0
			|| m_ExpBootEnvironmentInformation == nullptr)
		{
			err("Please Init SmbiosManager Module first!");
			ShowAllAddress();
			return false;
		}

		if(!ChangeSmbiosSerials()) {
			err("Failed to execute ChangeSmbiosSerials!");
			return false;
		}

		if (!ChangeBootInfo()) {
			err("Failed to execute ChangeBootInfo!");
			return false;
		}

		return true;
	}

	void SmbiosManager::ShowAllAddress()
	{
		log("\n m_ntoskrnlBase:%llx\n m_WmipSMBiosTablePhysicalAddress:%llx\n m_WmipSMBiosTableLengthAddr:%llx\n m_WmipSMBiosTableLength:%lx\n",
			m_ntoskrnlBase, m_WmipSMBiosTablePhysicalAddress, m_WmipSMBiosTableLengthAddr, m_WmipSMBiosTableLength);
	}

	bool SmbiosManager::ChangeSmbiosSerials()
	{
		auto* mapped = MmMapIoSpace(*m_WmipSMBiosTablePhysicalAddress, m_WmipSMBiosTableLength, MmNonCached);
		if (mapped == nullptr) {
			err("Failed to map SMBIOS structures!\n");
			return false;
		}

		LoopTables(mapped, m_WmipSMBiosTableLength);

		MmUnmapIoSpace(mapped, m_WmipSMBiosTableLength);

		return true;
	}

	bool SmbiosManager::ChangeBootInfo() const
	{
		PBOOT_ENVIRONMENT_INFORMATION bootInfo = m_ExpBootEnvironmentInformation;
		
		GUID guid = bootInfo->BootIdentifier;
		log("GUID: {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}\n",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
			guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
		
		RtlZeroMemory(bootInfo, sizeof(GUID));

		return true;
	}

	/**
	 * \brief Modify information in the table of given header
	 * \param header Table header (only 0-3 implemented)
	 * \return
	 */
	NTSTATUS SmbiosManager::ProcessTable(SMBIOS_HEADER* header)
	{
		using namespace Utils;
		if (!header->Length)
			return STATUS_UNSUCCESSFUL;

		if (header->Type == BIOS_INFO) {
			auto* ptr = reinterpret_cast<PBIOSInfo>(header);
			log("Handle Bios info. Vender:%s\n", GetString(header, ptr->Vendor));
			auto* vendor = GetString(header, ptr->Vendor);
			if(vendor) RandomizeString(vendor);
		}

		if (header->Type == SYSTEM_INFO)
		{
			auto* ptr = reinterpret_cast<PSystemInfo>(header);
			auto* uuid = reinterpret_cast<UINT8*>(&ptr->UUID);
			log("Handle SYSTEM Info. Manufacturer:%s, ProductName:%s, Version:%s, SerialNumber:%s, SKUNumber:%s\n",	
				GetString(header, ptr->Manufacturer),
				GetString(header, ptr->ProductName),
				GetString(header, ptr->Version),
				GetString(header, ptr->SerialNumber),
				GetString(header, ptr->SKUNumber)
				);
			/*log("Handle SYSTEM Info. %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
				uuid[0], uuid[1], uuid[2], uuid[3],
				uuid[4], uuid[5], uuid[6], uuid[7],
				uuid[8], uuid[9], uuid[10], uuid[11],
				uuid[12], uuid[13], uuid[14], uuid[15]);*/

			auto* manufacturer = GetString(header, ptr->Manufacturer);
			if(manufacturer) RandomizeString(manufacturer);

			auto* productName = GetString(header, ptr->ProductName);
			if(productName) RandomizeString(productName);

			auto* serialNumber = GetString(header, ptr->SerialNumber);
			if(serialNumber) RandomizeString(serialNumber);

			RtlZeroMemory(uuid, 16);
		}

		if (header->Type == BASEBOARD_INFO)
		{
			auto* ptr = reinterpret_cast<PBoardInfo>(header);
			log("Handle BASEBOARD Info. Manufacturer:%s, ProductName:%s, Version:%s, SerialNumber:%s, AssetTag:%s, LocationInChassis:%s\n",
				GetString(header, ptr->Manufacturer),
				GetString(header, ptr->ProductName),
				GetString(header, ptr->Version),
				GetString(header, ptr->SerialNumber),
				GetString(header, ptr->AssetTag), 
				GetString(header, ptr->LocationInChassis)
			);
			
			auto* manufacturer = GetString(header, ptr->Manufacturer);
			if(manufacturer) RandomizeString(manufacturer);

			auto* productName = GetString(header, ptr->ProductName);
			if(productName) RandomizeString(productName);
			
			auto* serialNumber = GetString(header, ptr->SerialNumber);
			if(serialNumber) RandomizeString(serialNumber);
		}
		
		
		if (header->Type == PROCESSOR_INFO) {
			auto* ptr = reinterpret_cast<PProcessorInfo>(header);
			log("Handle BASEBOARD Info. ProcessorManufacturer:%s, ProcessorVersion:%s, SerialNumber:%s, AssetTag:%s, PartNumber:%s\n",
				GetString(header, ptr->ProcessorManufacturer),
				GetString(header, ptr->ProcessorVersion),
				GetString(header, ptr->SerialNumber),
				GetString(header, ptr->AssetTag),
				GetString(header, ptr->PartNumber)
			);
			auto* manufacturer = GetString(header, ptr->ProcessorManufacturer);
			if(manufacturer) RandomizeString(manufacturer);

			auto* serialNumber = GetString(header, ptr->SerialNumber);
			if(serialNumber) RandomizeString(serialNumber);
		}

		if (header->Type == MEMORY_DEVICE) {
			auto* ptr = reinterpret_cast<PMemoryDevice>(header);
			log("Handle MEMORY_DEVICE Info. Manufacturer:%s, SerialNumber:%s\n", 
					GetString(header, ptr->Manufacturer),
					GetString(header, ptr->SerialNumber)
				);

			auto* manufacturer = GetString(header, ptr->Manufacturer);
			if(manufacturer) RandomizeString(manufacturer);
			
			auto* serialNumber = GetString(header, ptr->SerialNumber);
			if(serialNumber) RandomizeString(serialNumber);
		}

		return STATUS_SUCCESS;
	}

	/**
	 * \brief Loop through SMBIOS tables with provided first table header
	 * \param mapped Header of the first table
	 * \param size Size of all tables including strings
	 * \return
	 */
	NTSTATUS SmbiosManager::LoopTables(void* mapped, ULONG size)
	{
		auto* endAddress = static_cast<char*>(mapped) + size;
		while (true)
		{
			auto* header = static_cast<SMBIOS_HEADER*>(mapped);
			if (header->Type > 127 && header->Length == 4) // 4 byte == header size
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
	}


	/**
	 * \brief Get's the string from SMBIOS table
	 * \param header Table header
	 * \param string String itself
	 * \return Pointer to the null terminated string
	 */
	char* SmbiosManager::GetString(SMBIOS_HEADER* header, SMBIOS_STRING string)
	{
		const auto* start = reinterpret_cast<const char*>(header) + header->Length;

		if (!string || *start == 0)
			return nullptr;

		while (--string)
		{
			start += strlen(start) + 1;
		}

		return const_cast<char*>(start);
	}
}
