#pragma once

#include <DeadLock/SDK/CSchemaOffset.hpp>
#include <DeadLock/SDK/Types/CEntityData.hpp>
#include <DeadLock/SDK/Types/CUtlVector.hpp>

// ── STeamFOWEntity ────────────────────────────────────────────────────────────
// Per-entry element inside C_CitadelTeam::m_vecFOWEntities.
// Source: schema dump → sdk/client/structs/ STeamFOWEntity
// kStride = sizeof(STeamFOWEntity) = 0x60 (from dump).
class STeamFOWEntity
{
public:
	static constexpr uint32_t kStride   = 0x60;
	static constexpr int32_t  kMaxItems = 512;

	// [Schema] STeamFOWEntity::m_nEntIndex    (+0x30, CEntityIndex / int32, 4b)
	SCHEMA_OFFSET("STeamFOWEntity", "m_nEntIndex",     m_nEntIndex,     int32_t);
	// [Schema] STeamFOWEntity::m_bVisibleOnMap (+0x41, bool, 1b)
	SCHEMA_OFFSET("STeamFOWEntity", "m_bVisibleOnMap", m_bVisibleOnMap, bool);
	// [Schema] STeamFOWEntity::m_nTickHidden   (+0x44, GameTick_t / int32, 4b)
	SCHEMA_OFFSET("STeamFOWEntity", "m_nTickHidden",   m_nTickHidden,   int32_t);

	// ── stride-aware iterator ─────────────────────────────────────────────────

	struct Iterator
	{
		uint8_t* m_pCur;

		Iterator& operator++()                        { m_pCur += kStride; return *this; }
		bool      operator!=(const Iterator& o) const { return m_pCur != o.m_pCur; }

		STeamFOWEntity*  operator->() const { return reinterpret_cast<STeamFOWEntity*>(m_pCur); }
		STeamFOWEntity&  operator*()  const { return *reinterpret_cast<STeamFOWEntity*>(m_pCur); }
	};

	struct Range
	{
		uint8_t* m_pBase;
		int32_t  m_nCount;

		Iterator begin() const { return { m_pBase }; }
		Iterator end()   const { return { m_pBase + m_nCount * kStride }; }
	};

	static Range Iterate(const CUtlVectorFixed<STeamFOWEntity>& vec)
	{
		if (!vec.m_pMemory || vec.size <= 0 || vec.size > kMaxItems)
			return { nullptr, 0 };

		return { reinterpret_cast<uint8_t*>(vec.m_pMemory), vec.size };
	}
};

// ── C_CitadelTeam ─────────────────────────────────────────────────────────────
// Team entity; holds the FOW visibility array for all players on the team.
// m_vecFOWEntities uses CUtlVectorFixed<STeamFOWEntity>:
//   size       @ +0x00
//   m_pMemory  @ +0x08  (raw pointer; stride between entries = STeamFOWEntity::kStride)
class C_CitadelTeam : public C_BaseEntity
{
public:
	// [Schema] C_CitadelTeam::m_vecFOWEntities (+0x6C8, C_UtlVectorEmbeddedNetworkVar, 104b)
	SCHEMA_OFFSET("C_CitadelTeam", "m_vecFOWEntities", m_vecFOWEntities, CUtlVectorFixed<STeamFOWEntity>);
};
