#pragma once

#include <Common/MemoryEngine.hpp>

// ── CGlobalVarsBase ───────────────────────────────────────────────────────────
// Source: reclass analysis of the live object.  Key offsets confirmed live.
// Pattern (client.dll): 48 8B 05 ? ? ? ? 44 8B B7 DC 00 00 00 8B 70 04 B8 FF FF 00 00 89 B7 DC 00 00 00 66 01 43 04
// The instruction MOV RAX,[rip+disp] resolves to the CGlobalVarsBase* storage.
// Access via SDK::Pointers::GlobalVarsBase().
//
// Update notes:
//   +0x00 m_flRealTime            — real wall-clock seconds since map load
//   +0x04 m_nFrameCount           — total rendered frames
//   +0x30 m_flCurrentTime         — server game clock (same domain as GameTime_t)
//   +0x54 m_flTickInterval2       — used by GetPauseAdjustedTime internally
class CGlobalVarsBase
{
public:
	CUSTOM_OFFSET_FIELD( float, m_flRealTime,                  0x00 );
	CUSTOM_OFFSET_FIELD( int,   m_nFrameCount,                 0x04 );
	CUSTOM_OFFSET_FIELD( float, m_flAbsoluteFrameTime,         0x08 );
	CUSTOM_OFFSET_FIELD( float, m_flAbsoluteFrameStartTime,    0x0C );
	CUSTOM_OFFSET_FIELD( int,   m_nMaxClients,                 0x10 );
	CUSTOM_OFFSET_FIELD( float, m_flCurrentTimeUnscaled,       0x14 );

	// Server game clock — directly comparable with GameTime_t fields.
	// Confirmed live: advances at ~1 unit/second, same epoch as m_flGameStartTime.
	CUSTOM_OFFSET_FIELD( float, m_flCurrentTime,               0x30 );
	CUSTOM_OFFSET_FIELD( float, m_flFrameTime,                 0x34 );
	CUSTOM_OFFSET_FIELD( float, m_flFrameTimeStdDeviation,     0x38 );
	CUSTOM_OFFSET_FIELD( int,   m_nTickCount,                  0x48 );
	// Used internally by C_CitadelGameRules::GetPauseAdjustedTime (vtable[68]).
	CUSTOM_OFFSET_FIELD( float, m_flIntervalPerTick,           0x4C );
	CUSTOM_OFFSET_FIELD( float, m_flTickInterval2,             0x54 );

	CUSTOM_OFFSET_FIELD( const char*, m_pszMapName,            0x180 );
	CUSTOM_OFFSET_FIELD( const char*, m_pszMapGroupName,       0x188 );

	inline const char* GetMapName()
	{
		const char* p = m_pszMapName();
		return p ? p : "unknown";
	}

	inline const char* GetMapGroupName()
	{
		const char* p = m_pszMapGroupName();
		return p ? p : "unknown";
	}
};
