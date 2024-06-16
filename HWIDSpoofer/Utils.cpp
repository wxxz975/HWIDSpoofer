#include "Utils.h"

#include <ntimage.h>

#include "Shared.h"
#include "Logger.hpp"


namespace Utils
{
	PCHAR LowerStr(PCHAR str) {
		for (PCHAR s = str; *s; ++s) {
			*s = (CHAR)tolower(*s);
		}
		return str;
	}
	/**
	 * \brief Get base address of kernel module
	 * \param moduleName Name of the module (ex. storport.sys)
	 * \return Address of the module or null pointer if failed
	 */
	PVOID GetModuleBase(const char* moduleName)
	{
		PVOID address = nullptr;
		ULONG size = 0;

		auto status = ZwQuerySystemInformation(SystemModuleInformation, &size, 0, &size);
		if (status != STATUS_INFO_LENGTH_MISMATCH)
			return nullptr;

		auto* moduleList = static_cast<PSYSTEM_MODULE_INFORMATION>(ExAllocatePoolWithTag(NonPagedPool, size, POOL_TAG));
		if (!moduleList)
			return nullptr;

		status = ZwQuerySystemInformation(SystemModuleInformation, moduleList, size, nullptr);
		if (!NT_SUCCESS(status))
			goto end;

		for (auto i = 0; i < moduleList->ulModuleCount; i++)
		{
			auto module = moduleList->Modules[i];
			if (strstr(LowerStr(module.ImageName), moduleName))
			{
				address = module.Base;
				break;
			}
		}

	end:
		ExFreePool(moduleList);
		return address;
	}


	/**
	 * \brief Checks if buffer at the location of base parameter
	 * matches pattern and mask
	 * \param base Address to check
	 * \param pattern Byte pattern to match
	 * \param mask Mask containing unknown bytes
	 * \return
	 */
	BOOL CheckMask(const char* base, const char* pattern, const char* mask)
	{
		for (; *mask; ++base, ++pattern, ++mask)
		{
			if ('x' == *mask && *base != *pattern)
			{
				return false;
			}
		}

		return true;
	}

	/**
	 * \brief Find byte pattern in given buffer
	 * \param base Address to start searching in
	 * \param length Maximum length
	 * \param pattern Byte pattern to match
	 * \param mask Mask containing unknown bytes
	 * \return Pointer to matching memory
	 */
	PVOID FindPattern(PVOID base, int length, const char* pattern, const char* mask)
	{
		length -= static_cast<int>(strlen(mask));
		for (auto i = 0; i <= length; ++i)
		{
			const auto* data = static_cast<char*>(base);
			const auto* address = &data[i];
			if (CheckMask(address, pattern, mask))
				return PVOID(address);
		}

		return nullptr;
	}

	/**
	 * \brief Find byte pattern in given module/image ".text" and "PAGE" sections
	 * \param base Base address of the kernel module
	 * \param pattern Byte pattern to match
	 * \param mask Mask containing unknown bytes
	 * \return Pointer to matching memory
	 */
	PVOID FindPatternImage(PVOID base, const char* pattern, const char* mask)
	{
		PVOID match = nullptr;

		auto* headers = reinterpret_cast<PIMAGE_NT_HEADERS>(static_cast<char*>(base) + static_cast<PIMAGE_DOS_HEADER>(base)->e_lfanew);
		auto* sections = IMAGE_FIRST_SECTION(headers);

		for (auto i = 0; i < headers->FileHeader.NumberOfSections; ++i)
		{
			auto* section = &sections[i];
			if ('EGAP' == *reinterpret_cast<PINT>(section->Name) || memcmp(section->Name, ".text", 5) == 0)
			{
				match = FindPattern(static_cast<char*>(base) + section->VirtualAddress, section->Misc.VirtualSize, pattern, mask);
				if (match)
					break;
			}
		}

		return match;
	}

	/**
	 * \brief Generate pseudo-random text into given buffer
	 * \param text Pointer to text
	 * \param length Desired length
	 */
	VOID RandomText(char* text, const int length)
	{
		if (!text)
			return;

		static const char alphanum[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";

		auto seed = KeQueryTimeIncrement() * 0x975fff;

		for (auto n = 0; n < length; n++)
		{
			auto key = RtlRandomEx(&seed) % static_cast<int>(sizeof(alphanum) - 1);
			text[n] = alphanum[key];
		}
	}

	VOID RandomizeString(char* string, int size)
	{
		int len = 0;
		if (size == 0) len = strlen(string);
		else len = size;
		
		Utils::RandomText(string, len - 1);
		string[len] = '\0';
	}


	

	bool IsValidKernelPtr(PVOID Ptr)
	{
		if (Ptr == nullptr)
		{
			return false;
		}

		if (MmIsAddressValid(Ptr))
		{
			ULONG_PTR Address = (ULONG_PTR)Ptr;
			if (Address >= (ULONG_PTR)MmSystemRangeStart)
			{
				return true;
			}
		}

		return false;
	}


	static DWORD Random(PDWORD seed) {
		DWORD s = *seed * 1103515245 + 12345;
		*seed = s;
		return (s / 65536) % 32768;
	}

	static DWORD Hash(PBYTE buffer, DWORD length) {
		if (!length) {
			return 0;
		}

		DWORD h = (*buffer ^ 0x4B9ACE3F) * 0x1040193;
		for (DWORD i = 1; i < length; ++i) {
			h = (buffer[i] ^ h) * 0x1040193;
		}
		return h;
	}

	VOID SpoofBuffer(DWORD seed, PBYTE buffer, DWORD length) {
		seed ^= Hash(buffer, length);
		for (DWORD i = 0; i < length; ++i) {
			buffer[i] ^= (BYTE)Random(&seed);
		}
	}
}