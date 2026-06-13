#include "CShivComponent.hpp"

#include <DeadLock/SDK/SDK.hpp>
#include <DeadLock/SDK/Types/CEntityData.hpp>
#include <DeadLock/SDK/Update/CCitadelInput.hpp>
#include <DeadLock/SDK/Update/CUserCmd.hpp>
#include <DeadLock/SDK/Math/Math.hpp>
#include <DeadLock/SDK/FunctionListSDK.hpp>
#include <GameClient/CL_CitadelPlayerController.hpp>
#include <AndromedaClient/Features/CAimbot/CAimbotBones.hpp>
#include <numbers>
#include <DeadLock/SDK/Types/C_CitadelGameRules.hpp>

static constexpr float k_flOnTargetThreshold = 0.8f;
static constexpr float k_flPostFireSec = 0.2f;
static constexpr int k_nKnivesBoneMask = kBoneMaskAll & ~kBoneMaskHead;
static constexpr float k_flHalfBaseFov = 90.f * (std::numbers::pi_v<float> / 360.f);

void CShivComponent::ResetLock()
{
	m_pLockedPawn = nullptr;
	m_vAimPoint = {};
	m_flPostFireUntil = 0.f;
	m_aimState.Reset();
	m_eState = EKnivesState::Idle;
}

void CShivComponent::OnCreateMove(CCitadelInput* pInput, CUserCmd* /*pUserCmd*/)
{
	if (!KnivesActive)
		return;

	m_pending = {};
	if (m_eState != EKnivesState::Aiming)
		return;

	auto* pCamMgr = GetCCitadelCameraManager();
	if (!pCamMgr)
		return;

	HeroesContext ctx;
	if (!GetHeroesContext(pInput, ctx))
		return;

	auto* pActiveCam = pCamMgr->GetActiveCamera();
	const uintptr_t localAddr = reinterpret_cast<uintptr_t>(ctx.pLocalPawn);

	const float flCamFov = pActiveCam ? pActiveCam->m_flFov() : 0.f;
	const float flHalfCur = flCamFov * (std::numbers::pi_v<float> / 360.f);
	m_flFovScale = flCamFov > 1.f ? tanf(k_flHalfBaseFov) / tanf(flHalfCur) : 1.f;

	m_pending = {};
	m_pending.valid = true;
	m_pending.pawnAddr = localAddr;
	m_pending.pActiveCam = pActiveCam;

	const HeroTraceTarget target = AcquireOrRefreshTarget(m_pLockedPawn, ctx, localAddr, KnivesFov * m_flFovScale, {}, k_nKnivesBoneMask);
	if (!target.IsValid())
	{
		ResetLock();
		return;
	}

	m_pLockedPawn = target.pPawn;
	m_vAimPoint = target.vAimPos;

	QAngle* pInputAngles = CCitadelInput_GetViewAngles(pInput, 0);
	if (!pInputAngles)
		return;

	auto* pKnives = m_Knives.Get();
	auto* pVData = pKnives ? pKnives->m_pSubclassVData() : nullptr;
	const float flSpeed = pVData ? pVData->m_WeaponInfo().m_flBulletSpeed() : 0.f;
	const float flGravity = pVData ? pVData->m_WeaponInfo().m_flBulletGravityScale() * 800.f : 0.f;
	const float flInherit = pVData ? pVData->m_WeaponInfo().m_flBulletInheritShooterVelocityScale() : 0.f;

	Vector3 vPredicted = m_vAimPoint;
	const QAngle qFinal = ComputeHeroAim(m_aimState, ctx, m_vAimPoint, m_pLockedPawn, KnivesSmoothX, KnivesSmoothY, flSpeed, flGravity,
	                                     &vPredicted, flInherit);
	const QAngle qTarget = Math::CalcAngle(ctx.vCamPos, vPredicted);

	*pInputAngles = qFinal;
	m_pending.qFinal = qFinal;

	QAngle qErr = qTarget - qFinal;
	Math::NormalizeAngles(qErr);
	const float errMag = sqrtf(qErr.m_x * qErr.m_x + qErr.m_y * qErr.m_y);
	m_pending.bFireKnive = errMag <= k_flOnTargetThreshold;
}

void CShivComponent::OnPostMove(CCitadelInput* pInput)
{
	if (!KnivesActive)
		return;

	if (m_pending.valid && m_pending.pActiveCam && m_pending.qFinal != QAngle{})
	{
		m_pending.pActiveCam->SetAngles(m_pending.qFinal.m_x, m_pending.qFinal.m_y);
		auto* pLocalPawn = reinterpret_cast<C_CitadelPlayerPawn*>(m_pending.pawnAddr);
		pLocalPawn->m_angClientCamera() = m_pending.qFinal;
	}

	auto* pLocalCtrl = GetCL_CitadelPlayerController()->GetLocal();
	if (!pLocalCtrl)
		return;

	auto* pUserCmd = pInput->GetUserCmd(pLocalCtrl);
	if (!pUserCmd)
		return;

	auto* pGameRules = SDK::Pointers::GameRules();
	if (!pGameRules)
		return;

	auto* pKnives = m_Knives.Get();
	if (!pKnives)
		return;

	const float flNow = pGameRules->GetMatchTime();
	const bool bIsCooldown = pKnives->m_flCooldownEnd() > flNow;
	const bool bLostCharges = pKnives->m_iRemainingCharges() == 0;
	const bool bCanUse = !bIsCooldown && !bLostCharges;

	const bool bButtonHeld = pUserCmd->IsButtonHeld(IN_ABILITY1);
	const bool bFreshPress = bCanUse && bButtonHeld && !m_bWasButtonHeld;

	switch (m_eState)
	{
		case EKnivesState::Idle:
		{
			if (!bFreshPress)
				break;

			HeroesContext ctx;
			if (!GetHeroesContext(pInput, ctx))
				break;

			const uintptr_t localAddr = reinterpret_cast<uintptr_t>(ctx.pLocalPawn);
			const HeroTraceTarget target = AcquireOrRefreshTarget(nullptr, ctx, localAddr, KnivesFov * m_flFovScale, {}, k_nKnivesBoneMask);
			if (!target.IsValid())
				break;

			pUserCmd->ClearButton(IN_ABILITY1);
			m_pLockedPawn = target.pPawn;
			m_vAimPoint = target.vAimPos;
			m_eState = EKnivesState::Aiming;
			break;
		}

		case EKnivesState::Aiming:
		{
			pUserCmd->ClearButton(IN_ABILITY1);
			if (m_flPostFireUntil > 0.f)
			{
				if (flNow >= m_flPostFireUntil)
				{
					ResetLock();
					m_bWasButtonHeld = true;
					return;
				}
				break;
			}

			if (!m_pending.bFireKnive)
				break;

			pUserCmd->TapButton(IN_ABILITY1);
			m_flPostFireUntil = flNow + k_flPostFireSec;
			m_bWasButtonHeld = true;
			return;
		}
	}

	m_bWasButtonHeld = bButtonHeld;
}

void CShivComponent::OnRender()
{
	if (!KnivesActive || !KnivesShowFov)
		return;

	if (!ImGui::GetCurrentContext())
		return;

	auto* dl = ImGui::GetBackgroundDrawList();
	if (!dl)
		return;

	const auto& io = ImGui::GetIO();
	const ImVec2 cx = {io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f};
	const ImColor col(KnivesFovColor[0], KnivesFovColor[1], KnivesFovColor[2], 1.f);

	dl->AddCircle(cx, KnivesFov * m_flFovScale, col, 64, 1.f);
}