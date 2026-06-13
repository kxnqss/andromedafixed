#pragma once

#include <Common/MemoryEngine.hpp>
#include <DeadLock/SDK/CSchemaOffset.hpp>

// Forward-declared; full type in C_CitadelGameRules.cpp (needs SDK.hpp).
enum class EGameState : int32_t;

// ── C_CitadelGameRules ────────────────────────────────────────────────────────
// Access via SDK::Pointers::GameRules().
// Pattern (client.dll): 48 8B 05 ? ? ? ? 48 8B F9 48 85 C0 74 ? 83 78 74 06
// SEARCH_TYPE_PTR2 → resolves RIP+disp to the C_CitadelGameRules* storage.
class C_CitadelGameRules
{
public:
	// ── Pause tracking ────────────────────────────────────────────────────
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_nTotalPausedTicks",        m_nTotalPausedTicks,        int32_t );
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_nPauseStartTick",          m_nPauseStartTick,          int32_t );
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_bGamePaused",              m_bGamePaused,              bool    );
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_bServerPaused",            m_bServerPaused,            bool    );
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_iPauseTeam",               m_iPauseTeam,               int32_t );
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_flPauseTime",              m_flPauseTime,              double  );
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_fPauseRawTime",            m_fPauseRawTime,            float   );
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_fPauseCurTime",            m_fPauseCurTime,            float   );
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_fUnpauseRawTime",          m_fUnpauseRawTime,          float   );
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_fUnpauseCurTime",          m_fUnpauseCurTime,          float   );
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_nMatchClockUpdateTick",    m_nMatchClockUpdateTick,    int32_t );
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_flMatchClockAtLastUpdate", m_flMatchClockAtLastUpdate, float   );

	// ── Game start ────────────────────────────────────────────────────────
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_fLevelStartTime",  m_fLevelStartTime,  float      );
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_flGameStartTime",  m_flGameStartTime,  float      );
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_flRoundStartTime", m_flRoundStartTime, float      );
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_eGameState",       m_eGameState,       EGameState );
	SCHEMA_OFFSET( "C_CitadelGameRules", "m_bFreezePeriod",    m_bFreezePeriod,    bool       );

	// ── Timing methods ────────────────────────────────────────────────────

	// vtable[68] — adjusts inputTime by subtracting paused ticks * tick-interval.
	// Confirmed: sub_1815E55C0, accesses CGlobalVarsBase+0x54 internally.
	// Pass inputTime=0 and flags=0 to get the raw pause-adjusted current time.
	inline float GetPauseAdjustedTime( float flInputTime = 0.f, int nFlags = 0 )
	{
		using Fn = float* ( __fastcall* )( void*, float*, float, int );
		float flResult = 0.f;
		vget<Fn>( this, 68 )( this, &flResult, flInputTime, nFlags );
		return flResult;
	}

	float GetMatchTime();
};
