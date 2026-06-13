#include "CVisual.hpp"

#include <DeadLock/SDK/SDK.hpp>
#include <DeadLock/SDK/Types/CEntityData.hpp>
#include <DeadLock/SDK/Math/Math.hpp>
#include <DeadLock/SDK/Interface/CGameEntitySystem.hpp>
#include <DeadLock/SDK/Interface/IEngineToClient.hpp>

#include <GameClient/CEntityCache/CEntityCache.hpp>
#include <GameClient/CL_CitadelPlayerController.hpp>

#include <AndromedaClient/Fonts/FontAwesomeIcon.hpp>
#include <AndromedaClient/Render/CRenderStackSystem.hpp>

static CVisual g_CVisual{};

// ── Sound Step ────────────────────────────────────────────────────────────────

void CVisual::DrawSoundStep()
{
	if (!SoundStepEsp)
		return;

	std::scoped_lock lock(m_SoundLock);
	auto removeIt = std::ranges::remove_if(
		m_SoundList,
		[](const SoundData_t& s)
		{
			return GetTickCount64() - s.dwTime >= g_SoundShowTime;
		}
		);
	m_SoundList.erase(removeIt.begin(), m_SoundList.end());

	for (const auto& sound : m_SoundList)
	{
		const float ratio = static_cast<float>(GetTickCount64() - sound.dwTime) / static_cast<float>(g_SoundShowTime);
		const float alpha = std::lerp(1.f, 0.f, ratio);

		ImVec2 screen;
		if (Math::WorldToScreen(sound.Pos, screen))
		{
			constexpr float kSoundSize = 20.f;
			GetRenderStackSystem()->DrawCircle3D(sound.Pos, std::lerp(kSoundSize, 0.f, ratio), ImColor(1.f, 1.f, 0.f, alpha));
		}
	}
}

// ── Entity loop ───────────────────────────────────────────────────────────────

void CVisual::DrawEsp()
{
	auto* pCache = GetEntityCache();
	std::scoped_lock Lock(pCache->GetLock());

	auto* pLocal = GetCL_CitadelPlayerController()->GetLocal();

	for (const auto& cached : pCache->GetEntitiesByType(CachedEntity_t::CITADEL_PLAYER_CONTROLLER))
	{
		auto* pEntity = cached.m_Handle.Get();
		if (!pEntity || pEntity->pEntityIdentity()->Handle() != cached.m_Handle)
			continue;

		auto* pCtrl = reinterpret_cast<CCitadelPlayerController*>(pEntity);
		if (pCtrl == pLocal)
			continue;

		EspPlayerContext ctx;
		ctx.pCtrl = pCtrl;
		ctx.bTeam = pLocal && pCtrl->m_iTeamNum() == pLocal->m_iTeamNum();
		ctx.pPawn = pCtrl->m_hHeroPawn().Get<C_CitadelPlayerPawn>();

		if (ctx.pPawn)
		{
			ctx.bAlive = pCtrl->m_PlayerDataGlobal().m_bAlive();
			ctx.boxValid = ctx.pPawn->ComputeHitboxBounds(ctx.box);
		}

		for (auto* comp : m_Components)
			comp->OnRenderPlayer(ctx);
	}

	for (const auto& cached : pCache->GetEntitiesByType(CachedEntity_t::NPC_TROOPER))
	{
		if (!cached.m_bDraw)
			continue;

		auto* pEntity = cached.m_Handle.Get();
		if (!pEntity || pEntity->pEntityIdentity()->Handle() != cached.m_Handle)
			continue;

		auto* pTrooper = reinterpret_cast<C_NPC_Trooper*>(pEntity);
		for (auto* comp : m_Components)
			comp->OnRenderTrooper(pTrooper, cached.m_Bbox);
	}
}

// ── CalculateBoundingBoxes ────────────────────────────────────────────────────

void CVisual::CalculateBoundingBoxes()
{
	if (!SDK::Interfaces::EngineToClient()->IsInGame())
		return;

	auto* pCache = GetEntityCache();
	std::scoped_lock Lock(pCache->GetLock());

	for (auto& it : pCache->GetEntitiesByType(CachedEntity_t::NPC_TROOPER))
	{
		auto* pEntity = it.m_Handle.Get();
		if (!pEntity || pEntity->pEntityIdentity()->Handle() != it.m_Handle)
			continue;

		auto* pTrooper = reinterpret_cast<C_NPC_Trooper*>(pEntity);
		it.m_bDraw = pTrooper->GetBoundingBox(it.m_Bbox) && !pTrooper->m_pGameSceneNode()->m_bDormant();
	}
}

// ── OnClientOutput ────────────────────────────────────────────────────────────

void CVisual::OnClientOutput()
{
	if (!Active)
		return;

	DrawEsp();
	DrawSoundStep();

	for (auto* comp : m_Components)
		comp->OnFrame();
}

// ── Sound Step hook ───────────────────────────────────────────────────────────

void CVisual::OnStartSound(const Vector3& Pos, const int SourceEntityIndex, const char* szSoundName)
{
	if (!strstr(szSoundName, XorStr("Footstep")))
		return;

	auto* pEnt = SDK::Interfaces::GameEntitySystem()->GetBaseEntity(SourceEntityIndex);
	if (!pEnt || !pEnt->IsCitadelPlayerPawn())
		return;

	auto* pLocal = GetCL_CitadelPlayerController()->GetLocal();
	if (!pLocal || pLocal->m_iTeamNum() == pEnt->m_iTeamNum())
		return;

	std::scoped_lock lock(m_SoundLock);
	m_SoundList.emplace_back(GetTickCount64(), Pos);
}

// ── OnRender ─────────────────────────────────────────────────────────────────

void CVisual::OnRender()
{
	for (auto* comp : m_Components)
		comp->OnRender();
}

// ── Metadata ─────────────────────────────────────────────────────────────────

const char* CVisual::GetName() const
{
	return "Visuals";
}

const char8_t* CVisual::GetIcon() const
{
	return ICON_FA_EYE;
}

CVisual* GetVisual()
{
	return &g_CVisual;
}