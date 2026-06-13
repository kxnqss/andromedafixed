#pragma once

#include <Common/Common.hpp>

#include <array>
#include <vector>
#include <unordered_map>

#include <DeadLock/SDK/Types/CHandle.hpp>
#include <DeadLock/SDK/Types/CEntityData.hpp>

struct CachedEntity_t
{
	enum Type
	{
		UNKNOWN = 0,
		CITADEL_PLAYER_CONTROLLER,
		CITADEL_PLAYER_PAWN,
		NPC_TROOPER,
		CITADEL_TEAM,
		CITADEL_FISSURE_WALL,
		TYPE_COUNT,
	};

	CHandle m_Handle = {INVALID_EHANDLE_INDEX};
	Type m_Type = UNKNOWN;

	Rect_t m_Bbox = {0.f, 0.f, 0.f, 0.f};

	bool m_bDraw = false;
	bool m_bVisible = false;
};

class IEntityCache
{
public:
	virtual void OnAddEntity(CEntityInstance* pInst, CHandle handle) = 0;
	virtual void OnRemoveEntity(CEntityInstance* pInst, CHandle handle) = 0;
};

class CEntityCache final : public IEntityCache
{
public:
	using CachedEntityVec_t = std::vector<CachedEntity_t>;
	using Lock_t = std::recursive_mutex;

	void OnAddEntity(CEntityInstance* pInst, CHandle handle) override;
	void OnRemoveEntity(CEntityInstance* pInst, CHandle handle) override;

	CachedEntity_t::Type GetEntityType(C_BaseEntity* pBaseEntity);

	CachedEntityVec_t&
	GetEntitiesByType(CachedEntity_t::Type type)
	{
		return m_CachedEntities[static_cast<int>(type)];
	}

	Lock_t&
	GetLock()
	{
		return m_Lock;
	}

private:
	std::array<CachedEntityVec_t, CachedEntity_t::TYPE_COUNT> m_CachedEntities;
	std::unordered_map<uint32_t, CachedEntity_t::Type> m_HandleIndex;

	Lock_t m_Lock;
};

CEntityCache* GetEntityCache();