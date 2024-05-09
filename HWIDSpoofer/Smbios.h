#pragma once
#include <ntifs.h>
#include "HWIDChanger.h"

namespace Smbios
{
	NTSTATUS ChangeSmbiosSerials();


	class SmbiosManager : public HWIDChanger
	{
	public:
		SmbiosManager() = default;
		~SmbiosManager() = default;

		bool Initialize() override;

		bool Execute() override;

		void ShowAllAddress() override;
	private:
		PVOID m_ntoskrnlBase = nullptr;
		PPHYSICAL_ADDRESS m_WmipSMBiosTablePhysicalAddress = nullptr;
		PULONG m_WmipSMBiosTableLengthAddr = nullptr;
		ULONG m_WmipSMBiosTableLength = 0;
	};
}


