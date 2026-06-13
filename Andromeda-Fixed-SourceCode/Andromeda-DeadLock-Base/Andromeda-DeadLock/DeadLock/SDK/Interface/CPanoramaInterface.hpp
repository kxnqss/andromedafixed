#pragma once

#include <stdint.h>
#include <cstring>

struct ID3D11ShaderResourceView;

// ─── ImageData_t ──────────────────────────────────────────────────────────────
// Sixth argument to LoadImageFromURL (forum / Dota2 debug symbols).
// pszString must point to the same URL string passed as szURL.
// Width/height of 0xFFFFFFFF means "use natural image size".
struct ImageData_t
{
	void*    pszString = nullptr;   // = szURL (same pointer)
	uint32_t width     = 0xFFFFFFFF;
	uint32_t height    = 0xFFFFFFFF;
	uint32_t unk1      = 0xFFFFFFFF;
	uint32_t unk2      = 0xFFFFFFFF;
	float    scale     = 1.f;
	char     _pad[0x40]{};
};

// ─── Panorama image texture chain (confirmed for Deadlock) ───────────────────
//
// LoadImageFromURL returns CImageProxySource* (opaque entry handle).
// The SRV is reached via this fixed pointer chain:
//
//   CImageProxySource + 0x10  →  CImageData*            (set async after load)
//   CImageData        + 0x30  →  CSource2UITexture*
//   CSource2UITexture + 0x28  →  CSource2UITextureData*
//   CSource2UITextureData + 0x00 → CTextureDx11*
//   CTextureDx11      + 0x10  →  ID3D11ShaderResourceView*  (SRV0)
//
struct CTextureDx11
{
	uint8_t                   _pad[0x10];
	ID3D11ShaderResourceView* m_pSRV0;   // +0x10
	ID3D11ShaderResourceView* m_pSRV1;   // +0x18
};

struct CSource2UITextureData
{
	CTextureDx11* m_pDx11Texture;        // +0x00
};

struct CSource2UITexture
{
	uint8_t                _pad[0x28];
	CSource2UITextureData* m_pData;      // +0x28
};

struct CImageData
{
	uint8_t            _pad[0x30];
	CSource2UITexture* m_pUITexture;     // +0x30
};

// Entry returned by LoadImageFromURL — aka CImageProxySource.
// m_pImageData is filled asynchronously; poll each frame until non-null.
struct PanoramaImageEntry_t
{
	uint8_t     _pad[0x10];
	CImageData* m_pImageData;            // +0x10  (null until Panorama finishes)

	// Returns the D3D11 SRV, or nullptr if loading is still in progress.
	ID3D11ShaderResourceView* GetSRV() const
	{
		if ( !m_pImageData )                      return nullptr;
		auto* pTex = m_pImageData->m_pUITexture;  if ( !pTex ) return nullptr;
		auto* pDat = pTex->m_pData;               if ( !pDat ) return nullptr;
		auto* pDx  = pDat->m_pDx11Texture;        if ( !pDx  ) return nullptr;
		return pDx->m_pSRV0;
	}
};

// ─── CImageResourceManager ───────────────────────────────────────────────────
// Panorama image-loading interface. Fetched at runtime via CUIEngine::GetImageManager().
//
// LoadImageFromURL (vtable slot 0) call convention (Dota2 debug symbols):
//   pPanel = nullptr
//   unk    = nullptr
//   szURL  = "s2r://panorama/images/..."
//   fmt    = 2
//   pData  = &ImageData_t{ .pszString = szURL, .scale = 1.f }
class CImageResourceManager
{
public:
	auto LoadImageFromURL( void*        pPanel ,
	                       void*        unk    ,
	                       const char*  szURL  ,
	                       int          fmt    ,
	                       ImageData_t* pData  ) -> PanoramaImageEntry_t*
	{
		using Fn = PanoramaImageEntry_t* ( __fastcall* )(
			CImageResourceManager* , void* , void* , const char* , int , ImageData_t* );
		return ( *reinterpret_cast<Fn**>( this ) )[0]( this , pPanel , unk , szURL , fmt , pData );
	}
};

// ─── Panel system ─────────────────────────────────────────────────────────────

// 8-byte panel handle. The serial prevents stale-handle access: if a slot is
// reused after a panel is destroyed, the serial changes and the old handle
// resolves to nullptr.
//
// panorama.dll  CUIEngine::GetPanelPointer (vtable slot 33):
//   if (index < 0 || index >= count)               → nullptr
//   if (array[index].freeListMarker == index)       → nullptr  (free slot)
//   if (array[index].serial != handle.serial)       → nullptr  (stale)
//   return array[index].pPanel
//
struct PanelHandle_t
{
	int32_t  m_iPanelIndex    = -1;
	uint32_t m_unSerialNumber =  0;

	bool IsValid() const { return m_iPanelIndex != -1; }
};

// ─── CUIPanel ────────────────────────────────────────────────────────────────
// Confirmed from CUIPanel::Initialize (panorama.dll sub_1800DA960):
//   this + 0x10  →  CUtlString  { char* m_pMemory, int nAlloc, int nGrow }
//   *(const char**)(this + 0x10)  →  panel ID C-string
//
// Known panel IDs:
//   "Hud"                    — in-game HUD root
//   "CitadelDashboardRoot"   — main-menu / dashboard root
//   "panorama_world_panel_N" — per-entity world panels (nameplates)
//
class CUIPanel
{
public:
	// Returns the panel ID (e.g. "Hud"), or nullptr if not initialised.
	const char* GetID() const
	{
		return *reinterpret_cast<const char* const*>(
			reinterpret_cast<const uint8_t*>( this ) + 0x10 );
	}
};

// ─── CUIEngine ───────────────────────────────────────────────────────────────
// The inner engine object (3 200-byte heap allocation, panorama.dll).
//
// Object hierarchy:
//   panoramauiclient.dll global  →  CPanoramaUIEngine  (static .data object)
//   CPanoramaUIEngine  + 0x28    →  CUIEngine*          (this class)
//
// Memory layout (confirmed in IDA from CUIEngine constructor + GetPanelPointer):
//   this + 0x220   int32   panel slot count   (active + free)
//   this + 0x224   int32   CUtlVector capFlags
//   this + 0x228   ptr     CUIPanelEntry[count]  (32-byte entries)
//   this + 0xC18   ptr     CImageResourceManager*
//
// Each 32-byte panel entry:
//   + 0x00  int32   freeListMarker — equals own index when the slot is FREE
//   + 0x10  ptr     CUIPanel*
//   + 0x18  uint32  serial
//
class CUIEngine
{
public:
	// ── Engine accessors ─────────────────────────────────────────────────────

	int32_t GetPanelCount() const
	{
		return *field<int32_t>( 0x220 );
	}

	// Returns the image resource manager, or nullptr if not yet initialised.
	CImageResourceManager* GetImageManager() const
	{
		auto* p = *field<CImageResourceManager*>( 0xC18 );
		return isPtr( p ) ? p : nullptr;
	}

	// ── Panel handle resolution ───────────────────────────────────────────────

	// Resolve a PanelHandle to CUIPanel*.
	// Returns nullptr when the handle is out-of-range, stale, or the slot is free.
	CUIPanel* GetPanelPointer( PanelHandle_t handle ) const
	{
		const int32_t idx = handle.m_iPanelIndex;
		if ( idx < 0 || idx >= GetPanelCount() )
			return nullptr;

		const uint8_t* base = panelArrayBase();
		if ( !base )
			return nullptr;

		const uint8_t* entry = base + static_cast<uintptr_t>( idx ) * k_EntrySize;

		if ( *reinterpret_cast<const int32_t*>( entry ) == idx )       // free slot
			return nullptr;
		if ( *reinterpret_cast<const uint32_t*>( entry + 0x18 ) != handle.m_unSerialNumber )
			return nullptr;

		return *reinterpret_cast<CUIPanel* const*>( entry + 0x10 );
	}

	// Returns the first live panel whose ID exactly matches szID, or nullptr.
	CUIPanel* FindPanelByID( const char* szID ) const
	{
		if ( !szID || !*szID )
			return nullptr;

		const uint8_t* base = panelArrayBase();
		if ( !base )
			return nullptr;

		const int32_t count = GetPanelCount();
		for ( int32_t i = 0; i < count; ++i )
		{
			const uint8_t* entry = base + static_cast<uintptr_t>( i ) * k_EntrySize;
			if ( *reinterpret_cast<const int32_t*>( entry ) == i )
				continue;

			auto* pPanel = *reinterpret_cast<CUIPanel* const*>( entry + 0x10 );
			if ( !pPanel )
				continue;

			const char* id = pPanel->GetID();
			if ( id && strcmp( id, szID ) == 0 )
				return pPanel;
		}
		return nullptr;
	}

	// ── Panel enumeration ─────────────────────────────────────────────────────

	// Calls fn(CUIPanel*, int32_t slotIndex) for every live panel.
	// Return false from the callback to stop iteration early.
	//
	// Usage — enumerate all panel IDs:
	//   engine->EnumeratePanels( []( CUIPanel* p, int32_t i ) -> bool {
	//       DEV_LOG( "  [%d]  %s\n", i, p->GetID() ? p->GetID() : "?" );
	//       return true;
	//   });
	//
	template<typename Fn>
	void EnumeratePanels( Fn&& fn ) const
	{
		const uint8_t* base = panelArrayBase();
		if ( !base )
			return;

		const int32_t count = GetPanelCount();
		for ( int32_t i = 0; i < count; ++i )
		{
			const uint8_t* entry = base + static_cast<uintptr_t>( i ) * k_EntrySize;
			if ( *reinterpret_cast<const int32_t*>( entry ) == i )
				continue;

			auto* pPanel = *reinterpret_cast<CUIPanel* const*>( entry + 0x10 );
			if ( !pPanel )
				continue;

			if ( !static_cast<bool>( fn( pPanel, i ) ) )
				break;
		}
	}

private:
	static constexpr uintptr_t k_EntrySize = 32;
	static constexpr uintptr_t k_PtrMin    = 0x10000;

	template<typename T>
	const T* field( uintptr_t offset ) const
	{
		return reinterpret_cast<const T*>(
			reinterpret_cast<const uint8_t*>( this ) + offset );
	}

	template<typename T>
	static bool isPtr( T* p )
	{
		return reinterpret_cast<uintptr_t>( p ) >= k_PtrMin;
	}

	// Returns the base of the panel entry array, or nullptr if unavailable.
	const uint8_t* panelArrayBase() const
	{
		if ( ( *field<int32_t>( 0x224 ) & 0x7FFFFFFF ) == 0 )
			return nullptr;

		const auto* ptr = *field<const uint8_t*>( 0x228 );
		return isPtr( ptr ) ? ptr : nullptr;
	}
};
