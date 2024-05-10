#pragma once
#include <ntifs.h>
#include <windef.h>


namespace Utils
{
	PVOID GetModuleBase(const char* moduleName);
	BOOL CheckMask(const char* base, const char* pattern, const char* mask);
	PVOID FindPattern(PVOID base, int length, const char* pattern, const char* mask);
	PVOID FindPatternImage(PVOID base, const char* pattern, const char* mask);
	

	VOID RandomText(char* text, const int length);
	VOID RandomizeString(char* string);

	VOID SpoofBuffer(DWORD seed, PBYTE buffer, DWORD length);

	BOOL SwapControl(wchar_t* driverName, PVOID hookFunc, PVOID* originalFunc);


#define AdjustOffset(OriAddr, Offset) (PBYTE)OriAddr + Offset

	// 将这个OriAddr指针增加一个Offset的偏移，并且重新解释为T类型
	template<class T>
	T reinterpret(PVOID OriAddr, DWORD Offset)
	{
		return reinterpret_cast<T>(AdjustOffset(OriAddr, Offset));
	}

	// 将调用函数的地址，转化为函数地址
	///
	/// base : 调用函数的起始地址(call)
	/// totalLen: 此指令总长度 = 指令 + 地址
	///
	template<class T>
	T translateAddress(PVOID base, int totalLen)
	{
		int instrLen = totalLen - 4;
		__try {
			
			//ProbeForRead((PBYTE)base + instrLen, sizeof(LONG), sizeof(UCHAR));

			return (T)((PBYTE)base + totalLen + *((PLONG)((PBYTE)base + instrLen)));
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return nullptr;
		}
	}
};