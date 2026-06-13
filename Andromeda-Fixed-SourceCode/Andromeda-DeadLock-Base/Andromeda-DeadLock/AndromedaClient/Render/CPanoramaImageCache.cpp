#include "CPanoramaImageCache.hpp"

#include <DeadLock/SDK/SDK.hpp>
#include <Common/MemoryEngine.hpp>

// ─── statics ─────────────────────────────────────────────────────────────────

std::unordered_map<std::string, PanoramaImageEntry_t*> CPanoramaImageCache::m_Cache;

// ─────────────────────────────────────────────────────────────────────────────

PanoramaImageEntry_t* CPanoramaImageCache::RequestEntry(const char* szPath)
{
	auto* pEngine = SDK::Panorama::GetCUIEngine();
	if (!pEngine)
	{
		DEV_LOG("[pano-cache] CUIEngine not available\n");
		return nullptr;
	}

	auto* pImageMgr = pEngine->GetImageManager();
	if (!pImageMgr)
	{
		DEV_LOG("[pano-cache] GetImageManager() == null\n");
		return nullptr;
	}

	char szURL[256];
	snprintf(szURL, sizeof(szURL), XorStr("s2r://%s"), szPath);

	ImageData_t imgData;
	imgData.pszString = szURL;

	auto* pEntry = pImageMgr->LoadImageFromURL(nullptr, nullptr, szURL, 2, &imgData);

	constexpr uintptr_t k_ptrMin = 0x10000;
	auto addr = reinterpret_cast<uintptr_t>(pEntry);

	if (addr < k_ptrMin)
	{
		DEV_LOG("[pano-cache] LoadImageFromURL returned non-ptr 0x%llX for '%s'\n", static_cast<unsigned long long>(addr), szPath);
		return nullptr;
	}

	return pEntry;
}

ID3D11ShaderResourceView* CPanoramaImageCache::GetTexture(const char* szPath)
{
	if (!szPath || !szPath[0])
		return nullptr;

	auto it = m_Cache.find(szPath);
	if (it == m_Cache.end())
	{
		auto* pEntry = RequestEntry(szPath);
		m_Cache[szPath] = pEntry;
		return nullptr;
	}

	PanoramaImageEntry_t* pEntry = it->second;
	return pEntry ? pEntry->GetSRV() : nullptr;
}