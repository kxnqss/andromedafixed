#include "CEspMinimapComponent.hpp"

#include <DeadLock/SDK/SDK.hpp>
#include <DeadLock/SDK/Types/CEntityData.hpp>
#include <DeadLock/SDK/Types/CFOWTypes.hpp>

#include <GameClient/CEntityCache/CEntityCache.hpp>
#include <GameClient/CL_CitadelPlayerController.hpp>

#include <AndromedaClient/GUI/CAndromedaMenu.hpp>

#include "DeadLock/SDK/Interface/CGameEntitySystem.hpp"

// ── Load / Save ───────────────────────────────────────────────────────────────

void CEspMinimapComponent::Load(const rapidjson::Value& v)
{
	if (!v.HasMember(XorStr("Minimap")))
		return;
	GetBoolJson(v[XorStr("Minimap")], XorStr("Enabled"), Settings.Enabled);
}

void CEspMinimapComponent::Save(JsonWriter& w)
{
	w.String(XorStr("Minimap"));
	w.StartObject();
	{
		AddBoolJson(w, XorStr("Enabled"), Settings.Enabled);
	}
	w.EndObject();
}

// ── Menu ──────────────────────────────────────────────────────────────────────

void CEspMinimapComponent::RenderOtherMenu()
{
	GetAndromedaMenu()->RenderCheckBox(XorStr("Minimap Unlock"), XorStr("##Visual.Minimap"), Settings.Enabled);
}

// ── Per-frame logic ───────────────────────────────────────────────────────────

void CEspMinimapComponent::OnFrame()
{
	if (!Settings.Enabled)
		return;

	auto* pLocalCtrl = GetCL_CitadelPlayerController()->GetLocal();
	if (!pLocalCtrl)
		return;

	const uint8_t nLocalTeam = pLocalCtrl->m_iTeamNum();

	auto* pCache = GetEntityCache();
	std::scoped_lock lock(pCache->GetLock());

	for (const auto& cached : pCache->GetEntitiesByType(CachedEntity_t::CITADEL_TEAM))
	{
		auto* pEnt = cached.m_Handle.Get();
		if (!pEnt)
			continue;

		if (pEnt->m_iTeamNum() != nLocalTeam)
			continue;

		auto& vec = static_cast<C_CitadelTeam*>(pEnt)->m_vecFOWEntities();
		for (auto& entry : STeamFOWEntity::Iterate(vec))
		{
			if (entry.m_nEntIndex() <= 0)
				continue;

			auto* pEntObj = SDK::Interfaces::GameEntitySystem()->GetBaseEntity(entry.m_nEntIndex());
			if (!pEntObj || !pEntObj->IsCitadelPlayerPawn())
				continue;

			entry.m_bVisibleOnMap() = true;
			entry.m_nTickHidden() = 0;
		}
	}
}