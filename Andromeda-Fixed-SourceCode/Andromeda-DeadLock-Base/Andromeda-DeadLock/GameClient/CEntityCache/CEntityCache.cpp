#include "CEntityCache.hpp"

static CEntityCache g_CEntityCache{};

void CEntityCache::OnAddEntity(CEntityInstance* pInst, CHandle handle)
{
	std::scoped_lock lock(m_Lock);

	auto* pBaseEntity = pInst->pEntityIdentity()->pBaseEntity();
	if (!pBaseEntity)
		return;

	const auto type = GetEntityType(pBaseEntity);
	if (type == CachedEntity_t::UNKNOWN)
		return;

	if (m_HandleIndex.contains(handle.m_Index))
		return;

	m_HandleIndex[handle.m_Index] = type;

	CachedEntity_t entry;
	entry.m_Handle = handle;
	entry.m_Type = type;
	m_CachedEntities[static_cast<int>(type)].emplace_back(entry);
}

void CEntityCache::OnRemoveEntity(CEntityInstance* pInst, CHandle handle)
{
	std::scoped_lock lock(m_Lock);

	auto idxIt = m_HandleIndex.find(handle.m_Index);
	if (idxIt == m_HandleIndex.end())
		return;

	const auto type = idxIt->second;
	auto& vec = m_CachedEntities[static_cast<int>(type)];

	auto it = std::ranges::find_if(vec, [handle](const CachedEntity_t& e)
	{
		return e.m_Handle == handle;
	});

	if (it != vec.end())
	{
		*it = vec.back();
		vec.pop_back();
	}

	m_HandleIndex.erase(idxIt);
}

CachedEntity_t::Type CEntityCache::GetEntityType(C_BaseEntity* pBaseEntity)
{
	if (pBaseEntity->IsCitadelPlayerController())
		return CachedEntity_t::CITADEL_PLAYER_CONTROLLER;

	if (pBaseEntity->IsCitadelPlayerPawn())
		return CachedEntity_t::CITADEL_PLAYER_PAWN;

	if (pBaseEntity->IsNpcTrooper())
		return CachedEntity_t::NPC_TROOPER;

	if (pBaseEntity->IsCitadelTeam())
		return CachedEntity_t::CITADEL_TEAM;

	if (pBaseEntity->IsCitadelFissureWall())
		return CachedEntity_t::CITADEL_FISSURE_WALL;

	return CachedEntity_t::UNKNOWN;
}

CEntityCache* GetEntityCache()
{
	return &g_CEntityCache;
}