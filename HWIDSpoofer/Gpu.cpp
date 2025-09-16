#include "Gpu.h"

#include "Shared.h"
#include "Logger.hpp"
#include "Utils.h"
#include <wchar.h>


namespace Gpu
{
	extern "C" POBJECT_TYPE * IoDriverObjectType;

	PDRIVER_DISPATCH GpuManager::m_original_device_ctl = nullptr;

	GpuManager::GpuManager()
	{
	}

	GpuManager::~GpuManager()
	{
		
	}
	bool GpuManager::Initialize()
	{
		// Reference: https://gist.github.com/Xyrem/4b6ea950e26565581ccd851be09ace00
		GUID guid{};
		m_nvlddmkmBase = Utils::GetModuleBase("nvlddmkm.sys");
		if (!m_nvlddmkmBase) {
			err("Failed to get the m_nvlddmkmBase!\n");
			return false;
		}
		log("Found nvlddmkmBase:%llx\n", m_nvlddmkmBase);

		/*
			works for 560.92 → 576.57
		*/
		auto callAddr = Utils::FindPatternImage(m_nvlddmkmBase, "\xE8\x00\x00\x00\x00\x48\x8B\xD8\x48\x85\xC0\x0F\x84\xF0", "x????xxxxxxxxx");
		if(!callAddr) {
			err("Failed to find GpuMgrGetGpuFromId pattern!\n");
			return false;
		}

		m_UuidValidOffset = 0xC6C;
		/*	m_gpuObjectOffset = 0x124700;*/
		
		m_gpuMgrGetGpuFromId = Utils::translateAddress<GpuMgrGetGpuFromId>(callAddr, 5);
		if(m_gpuMgrGetGpuFromId == nullptr) {
			err("Failed to translate GpuMgrGetGpuFromId address!\n");
			return false;
		}
		log("Found GpuMgrGetGpuFromId:%llx\n", (PVOID)m_gpuMgrGetGpuFromId);

		for (int idx = 0; idx < 0x20; ++idx) {
			auto gpu = m_gpuMgrGetGpuFromId(idx);
			if (!gpu) continue;

			auto uuid = (PBYTE)gpu + m_UuidValidOffset;
			
			if(!uuid || !*uuid) continue;
			
			log("Gpu UUID Valid: %llx", uuid);
			auto uuidPtr = uuid + 1;
			for (int i = 0; i < sizeof(UUID); ++i) {
				uuidPtr[i] = __rdtsc();
			}
		}
		
	}
	bool GpuManager::Execute()
	{
		return true;
	}
	void GpuManager::ShowAllAddress()
	{
		log("m_nvlddmkmBase:%llx, m_gpuObject:%llx\n", m_nvlddmkmBase, m_gpuObject);
	}
}