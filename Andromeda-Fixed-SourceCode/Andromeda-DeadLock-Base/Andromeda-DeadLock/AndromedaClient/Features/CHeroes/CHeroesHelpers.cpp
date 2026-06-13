#include "CHeroesHelpers.hpp"

#include <Common/Common.hpp>
#include <ImGui/imgui.h>

#include <DeadLock/SDK/Math/Math.hpp>
#include <DeadLock/SDK/FunctionListSDK.hpp>
#include <DeadLock/SDK/Types/CEntityData.hpp>
#include <DeadLock/SDK/Update/CCitadelCamera.hpp>
#include <DeadLock/SDK/Update/CCitadelInput.hpp>
#include <GameClient/CL_CitadelPlayerController.hpp>
#include <GameClient/CL_Bones.hpp>
#include <GameClient/CEntityCache/CEntityCache.hpp>
#include <AndromedaClient/Features/CAimbot/CAimbotBones.hpp>
#include <AndromedaClient/Features/CAimbot/intercept.hpp>
#include <AndromedaClient/Helpers/CTraceHelper.hpp>
#include <algorithm>

bool GetHeroesContext(CCitadelInput* pInput, HeroesContext& outCtx)
{
	if (!ImGui::GetCurrentContext())
		return false;

	auto* pLocalCtrl = GetCL_CitadelPlayerController()->GetLocal();
	if (!pLocalCtrl)
		return false;

	auto* pLocalPawn = pLocalCtrl->m_hHeroPawn().Get<C_CitadelPlayerPawn>();
	if (!pLocalPawn)
		return false;

	auto* pNode = pLocalPawn->m_pGameSceneNode();
	if (!pNode)
		return false;

	QAngle* pAngles = CCitadelInput_GetViewAngles(pInput, 0);
	if (!pAngles)
		return false;

	Vector3 vCamPos = pNode->m_vecAbsOrigin();
	QAngle qView = *pAngles;

	if (auto* camMgr = GetCCitadelCameraManager())
	{
		if (auto* pCam = camMgr->GetActiveCamera())
		{
			const Vector3& vPos = pCam->m_vecPosition();
			if (vPos.m_x != 0.f || vPos.m_y != 0.f || vPos.m_z != 0.f)
				vCamPos = vPos;

			const float* ang = pCam->m_angView();
			qView = QAngle(ang[0], ang[1], 0.f);
		}
	}

	outCtx.pLocalCtrl = pLocalCtrl;
	outCtx.pLocalPawn = pLocalPawn;
	outCtx.vCamPos = vCamPos;
	outCtx.qView = qView;
	outCtx.vCX = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
	return true;
}

static HeroTraceTarget CheckPawnVisibility(
	C_CitadelPlayerPawn* pPawn,
	const HeroesContext& ctx,
	uintptr_t localPawnAddr,
	float fovRadius,
	int boneMask = kBoneMaskAll,
	float* fClosestOut = nullptr)
{
	HeroTraceTarget result;

	auto* pNode = pPawn->m_pGameSceneNode();
	if (!pNode)
		return result;

	if (!pPawn->m_CBodyComponent() || !pPawn->m_pRenderComponent())
		return result;

	if (!pNode->GetSkeletonInstance()->m_modelState().m_hModel().is_valid())
		return result;

	float fClosest = FLT_MAX;
	Vector3 vFirstBone = {};
	bool bAnyVisible = false;

	for (const auto& bone : k_AimBones)
	{
		if (!(boneMask & bone.mask))
			continue;

		const auto* boneList = GetCL_Bones()->GetHitboxBones(pPawn, bone.slot);
		if (!boneList)
			continue;

		for (const int16_t bid : *boneList)
		{
			Vector3 bonePos;
			if (!pNode->GetBonePosition(static_cast<int>(bid), bonePos))
				continue;

			const TraceVec3 trFrom{ctx.vCamPos.m_x, ctx.vCamPos.m_y, ctx.vCamPos.m_z};
			const TraceVec3 trTo{bonePos.m_x, bonePos.m_y, bonePos.m_z};
			if (!Trace_IsVisibleEx(&trFrom, &trTo, localPawnAddr))
				continue;

			ImVec2 screen;
			if (!Math::WorldToScreen(bonePos, screen))
				continue;

			const float dx = screen.x - ctx.vCX.x;
			const float dy = screen.y - ctx.vCX.y;
			const float dist = sqrtf(dx * dx + dy * dy);

			if (dist >= fovRadius)
				continue;

			if (!bAnyVisible)
			{
				vFirstBone = bonePos;
				bAnyVisible = true;
			}

			fClosest = (std::min)(dist, fClosest);
			break; // first visible bone in this slot wins
		}
	}

	if (!bAnyVisible)
		return result;

	result.pPawn = pPawn;
	result.vAimPos = vFirstBone;

	if (fClosestOut)
		*fClosestOut = fClosest;

	return result;
}

HeroTraceTarget AcquireOrRefreshTarget(
	C_CitadelPlayerPawn* pCurrent,
	const HeroesContext& ctx,
	uintptr_t localPawnAddr,
	float fovRadius,
	const EnemyFilterFn& filter,
	int boneMask)
{
	HeroTraceTarget best;
	float fBestDist = fovRadius;

	auto* pCache = GetEntityCache();
	std::scoped_lock lock(pCache->GetLock());

	for (const auto& cached : pCache->GetEntitiesByType(CachedEntity_t::CITADEL_PLAYER_CONTROLLER))
	{
		auto* pEntity = cached.m_Handle.Get();
		if (!pEntity || !pEntity->pEntityIdentity())
			continue;

		auto* pCtrl = reinterpret_cast<CCitadelPlayerController*>(pEntity);
		if (pCtrl == ctx.pLocalCtrl)
			continue;

		if (pCtrl->m_iTeamNum() == ctx.pLocalCtrl->m_iTeamNum())
			continue;

		if (!pCtrl->m_PlayerDataGlobal().m_bAlive())
			continue;

		auto* pPawn = pCtrl->m_hHeroPawn().Get<C_CitadelPlayerPawn>();
		if (!pPawn || !pPawn->IsCitadelPlayerPawn())
			continue;

		static const CUtlStringToken k_EtherealShift("upgrade_self_bubble");
		if (pPawn->IsModifierActive(k_EtherealShift, "CCitadel_Modifier_Bubble"))
			continue;

		if (filter && !filter(pCtrl, pPawn))
			continue;

		float dist;
		const HeroTraceTarget candidate = CheckPawnVisibility(pPawn, ctx, localPawnAddr, fovRadius, boneMask, &dist);

		if (!candidate.IsValid())
			continue;

		// Prioritise the already-locked target — return it immediately if it still
		// passes the filter and is visible, without switching to a closer stranger.
		if (pPawn == pCurrent)
			return candidate;

		if (dist >= fBestDist)
			continue;

		fBestDist = dist;
		best = candidate;
	}

	return best;
}

static constexpr float k_flMaxAngVelPerFrame = 2.f; // °/frame feedforward clamp

QAngle ComputeHeroAim(
	HeroAimState& state,
	const HeroesContext& ctx,
	const Vector3& vAimPoint,
	C_CitadelPlayerPawn* pTargetPawn,
	float sensX,
	float sensY,
	float bulletSpeed,
	float gravityAccel,
	Vector3* pOutPredicted,
	float flInheritScale)
{
	const bool bSamePawn = (pTargetPawn && pTargetPawn == state.pPrevPawn);

	// ── Optional prediction ───────────────────────────────────────────────────
	static constexpr float k_flMinVelLen = 10.f;
	static constexpr float k_flMaxVelLen = 800.f;

	Vector3 vPredicted = vAimPoint;
	if (bulletSpeed > 100.f && pTargetPawn)
	{
		const auto& nv = pTargetPawn->m_vecVelocity();
		Vector3 vel = {nv.x(), nv.y(), nv.z()};

		// Subtract shooter velocity scaled by bullet inherit factor:
		// effective_target_vel = target_vel - shooter_vel * inherit_scale
		if (flInheritScale > 0.f && ctx.pLocalPawn)
		{
			const auto& sv = ctx.pLocalPawn->m_vecVelocity();
			vel.m_x -= sv.x() * flInheritScale;
			vel.m_y -= sv.y() * flInheritScale;
			vel.m_z -= sv.z() * flInheritScale;
		}

		const float velLen = vel.Length();
		if (velLen >= k_flMinVelLen && velLen <= k_flMaxVelLen)
		{
			const prediction::math::Vec3 pmSrc{ctx.vCamPos.m_x, ctx.vCamPos.m_y, ctx.vCamPos.m_z};
			const prediction::math::Vec3 pmTgt{vAimPoint.m_x, vAimPoint.m_y, vAimPoint.m_z};
			const prediction::math::Vec3 pmVel{vel.m_x, vel.m_y, vel.m_z};

			const auto r = gravityAccel > 0.f
				               ? prediction::math::solve_ballistic(pmSrc, pmTgt, pmVel, bulletSpeed, gravityAccel)
				               : prediction::math::solve_linear(pmSrc, pmTgt, pmVel, bulletSpeed);
			if (r.valid && r.time < 5.f)
				vPredicted = {r.aim_point.x, r.aim_point.y, r.aim_point.z};
		}
	}

	if (pOutPredicted)
		*pOutPredicted = vPredicted;

	// ── Directions ────────────────────────────────────────────────────────────
	// qRawDir — unpredicted, used for feedforward to track target angular motion.
	// qTarget  — predicted (or same as raw if no prediction), what we want to converge to.
	const QAngle qRawDir = Math::CalcAngle(ctx.vCamPos, vAimPoint);
	const QAngle qTarget = Math::CalcAngle(ctx.vCamPos, vPredicted);

	// ── Feedforward: track angular velocity of the raw bone direction ────────
	// Using qRaw (current bone) keeps this stable even when the velocity
	// estimate has per-frame noise from animation, which would make qTarget
	// oscillate and turn the feedforward into a destabilizing term.
	QAngle qRawAngVel = {};
	if (bSamePawn && (state.qPrevRawDir.m_x != 0.f || state.qPrevRawDir.m_y != 0.f))
	{
		qRawAngVel = qRawDir - state.qPrevRawDir;
		Math::NormalizeAngles(qRawAngVel);
		qRawAngVel.m_x = std::clamp(qRawAngVel.m_x, -k_flMaxAngVelPerFrame, k_flMaxAngVelPerFrame);
		qRawAngVel.m_y = std::clamp(qRawAngVel.m_y, -k_flMaxAngVelPerFrame, k_flMaxAngVelPerFrame);
	}

	QAngle qViewFF = ctx.qView;
	qViewFF.m_x += qRawAngVel.m_x;
	qViewFF.m_y += qRawAngVel.m_y;
	Math::NormalizeAngles(qViewFF);

	// ── IIR correction ────────────────────────────────────────────────────────
	QAngle qDiff = qTarget - qViewFF;
	Math::NormalizeAngles(qDiff);

	const float iirX = qDiff.m_x / (sensX + 1.f);
	const float iirY = qDiff.m_y / (sensY + 1.f);

	QAngle qFinal;
	qFinal.m_x = std::clamp(qViewFF.m_x + iirX, -89.f, 89.f);
	qFinal.m_y = qViewFF.m_y + iirY;
	qFinal.m_z = 0.f;
	Math::NormalizeAngles(qFinal);

	// ── Persist state for next frame ─────────────────────────────────────────
	state.qPrevRawDir = qRawDir;
	state.pPrevPawn = pTargetPawn;

	return qFinal;
}

void WriteViewAngles(
	CCitadelInput* pInput,
	C_CitadelPlayerPawn* pLocalPawn,
	const QAngle& qFinal)
{
	QAngle* pInputAngles = CCitadelInput_GetViewAngles(pInput, 0);
	if (pInputAngles)
		*pInputAngles = qFinal;

	CCitadel_Camera* pActiveCam = nullptr;
	if (auto* camMgr = GetCCitadelCameraManager())
		pActiveCam = camMgr->GetActiveCamera();

	if (pActiveCam)
		pActiveCam->SetAngles(qFinal.m_x, qFinal.m_y);

	pLocalPawn->m_angClientCamera() = qFinal;
}