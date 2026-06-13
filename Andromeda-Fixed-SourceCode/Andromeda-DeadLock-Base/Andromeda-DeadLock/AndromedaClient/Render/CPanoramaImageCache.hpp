#pragma once

#include <string>
#include <unordered_map>

#include <DeadLock/SDK/Interface/CPanoramaInterface.hpp>

// ─────────────────────────────────────────────────────────────────────────────
//  CPanoramaImageCache
//
//  Generic cache for any Panorama / Source 2 image loaded via LoadImageFromURL.
//  Callers pass a relative S2 resource path; the cache prepends "s2r://" and
//  manages the async entry lifecycle internally.
//
//  Flow (per frame):
//   GetTexture("panorama/images/heroes/inferno_mm_psd.vtex")
//    ├─ entry in cache → entry->GetSRV()  (nullptr until Panorama finishes)
//    └─ no entry yet  → RequestEntry() → LoadImageFromURL → cache entry
// ─────────────────────────────────────────────────────────────────────────────
class CPanoramaImageCache
{
public:
	// Returns the D3D11 SRV for the given resource path once Panorama finishes
	// loading. Returns nullptr while loading is in progress — poll each frame.
	// szPath is a relative S2 path, e.g. "panorama/images/heroes/inferno_mm_psd.vtex"
	static ID3D11ShaderResourceView* GetTexture(const char* szPath);

private:
	// Calls LoadImageFromURL with "s2r://<szPath>"; caches the returned entry
	// pointer for async SRV polling.
	static PanoramaImageEntry_t* RequestEntry(const char* szPath);

	// Keyed by relative path. Value is the Panorama image entry (SRV filled async).
	static std::unordered_map<std::string, PanoramaImageEntry_t*> m_Cache;
};