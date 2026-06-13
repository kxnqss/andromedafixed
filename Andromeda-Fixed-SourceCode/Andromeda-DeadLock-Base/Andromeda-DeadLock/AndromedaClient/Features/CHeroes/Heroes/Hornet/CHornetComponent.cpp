#include "CHornetComponent.hpp"

#include <DeadLock/SDK/SDK.hpp>
#include <DeadLock/SDK/Types/CEntityData.hpp>
#include <DeadLock/SDK/Types/C_CitadelGameRules.hpp>
#include <DeadLock/SDK/Update/CCitadelInput.hpp>
#include <DeadLock/SDK/Update/CUserCmd.hpp>
#include <DeadLock/SDK/Math/Math.hpp>
#include <DeadLock/SDK/FunctionListSDK.hpp>
#include <GameClient/CL_CitadelPlayerController.hpp>
#include <numbers>

static constexpr float k_flScopeWait = 0.1f;
static constexpr float k_flAttackCooldown = 0.1f;
static constexpr float k_flOnTargetThreshold = 0.2f;
static constexpr float k_flHalfBaseFov = 90.f * (std::numbers::pi_v<float> / 360.f);

void CHornetComponent::RefreshSnipeProps(CCitadel_Ability_Hornet_Snipe* pSnipe)
{
	float d, lb, lt, hs;
	__try
	{
		d = pSnipe->GetPropertyValue("Damage");
		lb = pSnipe->GetPropertyValue("LowHealthEnemyDamageBonus");
		lt = pSnipe->GetPropertyValue("LowHealthEnemyThresholdPct");
		hs = 1.0f + pSnipe->GetPropertyValue("HeadshotBonus") / 100.0f;
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
		          ? EXCEPTION_EXECUTE_HANDLER
		          : EXCEPTION_CONTINUE_SEARCH)
	{
		return;
	}

	if (d > 0.f && lb >= 0.f && lt > 0.f && hs >= 1.f)
	{
		m_propsCache.flBaseDamage = d;
		m_propsCache.flLowHealthBonus = lb;
		m_propsCache.flLowHPThresh = lt;
		m_propsCache.flHeadshotScale = hs;
		m_propsCache.bValid = true;
	}
}


void CHornetComponent::ResetLock()
{
	m_pLockedPawn = nullptr;
	m_vAimPoint = {};
	m_pending = {};
	m_aimState.Reset();
}

void CHornetComponent::OnCreateMove(CCitadelInput* pInput, CUserCmd* /*pUserCmd*/)
{
	if (!SnipeActive)
		return;

	auto* pGlobalVars = SDK::Pointers::GlobalVarsBase();
	if (!pGlobalVars)
		return;

	auto* pCamMgr = GetCCitadelCameraManager();
	if (!pCamMgr)
		return;

	m_pending = {};
	auto* pSnipe = static_cast<CCitadel_Ability_Hornet_Snipe*>(m_Snipe.Get());
	if (!pSnipe)
	{
		m_eState = ESnipeState::Idle;
		ResetLock();
		return;
	}

	auto* pGameRules = SDK::Pointers::GameRules();
	if (!pGameRules)
		return;

	HeroesContext ctx;
	if (!GetHeroesContext(pInput, ctx))
	{
		return;
	}

	auto* pActiveCam = pCamMgr->GetActiveCamera();
	const uintptr_t localPawnAddr = reinterpret_cast<uintptr_t>(ctx.pLocalPawn);
	const float flScopeTime = pSnipe->m_flScopeStartTime();
	const float flNow = pGameRules->GetMatchTime();

	const float flCamFov = pActiveCam ? pActiveCam->m_flFov() : 0.f;
	const float flHalfCurrent = flCamFov * (std::numbers::pi_v<float> / 360.f);
	m_flFovScale = flCamFov > 1.f ? tanf(k_flHalfBaseFov) / tanf(flHalfCurrent) : 1.f;

	m_pending.valid = true;
	m_pending.pawnAddr = reinterpret_cast<uintptr_t>(ctx.pLocalPawn);
	m_pending.pActiveCam = pActiveCam;

	RefreshSnipeProps(pSnipe);
	const float flBaseDamage = m_propsCache.flBaseDamage;
	const float flLowHealthBonus = m_propsCache.flLowHealthBonus;
	const float flLowHealthThresh = m_propsCache.flLowHPThresh;
	const float flHeadshotScale = m_propsCache.flHeadshotScale;

	const auto makeKillFilter = [flBaseDamage, flLowHealthBonus, flLowHealthThresh, flHeadshotScale](float chargeScale) -> EnemyFilterFn
	{
		return [flBaseDamage, flLowHealthBonus, flLowHealthThresh, flHeadshotScale, chargeScale]
		(CCitadelPlayerController* pCtrl, C_CitadelPlayerPawn* pPawn) -> bool
		{
			const float spiritResistPct = GetEntityStat(static_cast<int>(EStatsType::ETechArmorDamageReduction), pPawn);

			auto& globalData = pCtrl->m_PlayerDataGlobal();
			const int targetHp = globalData.m_iHealth();
			const int targetHpMax = globalData.m_iHealthMax();

			if (targetHp <= 0 || targetHpMax <= 0)
				return false;

			const float targetHpPct = static_cast<float>(targetHp) / static_cast<float>(targetHpMax);
			const bool isLowHp = targetHpPct * 100.f <= flLowHealthThresh;
			const float lowHpBonus = isLowHp ? flLowHealthBonus : 0.f;

			const float baseDamage = flBaseDamage + lowHpBonus;
			const float chargedDamage = baseDamage * chargeScale;
			const float headshotDamage = chargedDamage * flHeadshotScale;

			const float effectiveDmg = headshotDamage * (1.f - spiritResistPct / 100.f);
			return effectiveDmg >= static_cast<float>(targetHp);
		};
	};

	switch (m_eState)
	{
		case ESnipeState::Idle:
		{
			if (flScopeTime > 0.f)
			{
				m_eState = ESnipeState::Scoping;
				break;
			}

			if (pSnipe->m_iRemainingCharges() == 0 || flNow < pSnipe->m_flCooldownEnd())
				break;

			const EnemyFilterFn killFilter = makeKillFilter(0.5f + 0.5f * k_flScopeWait);
			const HeroTraceTarget target = AcquireOrRefreshTarget(m_pLockedPawn, ctx, localPawnAddr, SnipeFov, killFilter);
			if (!target.IsValid())
			{
				ResetLock();
				break;
			}

			m_pLockedPawn = target.pPawn;
			m_vAimPoint = target.vAimPos;

			m_pending.bHoldScope = true;
			m_flNextAttackTime = flNow + k_flScopeWait;
			break;
		}

		case ESnipeState::Scoping:
		{
			if (flScopeTime <= 0.f)
			{
				m_eState = ESnipeState::Idle;
				ResetLock();
				break;
			}

			if (pSnipe->m_iRemainingCharges() == 0)
				break;

			const float chargeElapsed = flNow - flScopeTime;
			const float scopeChargeScale = chargeElapsed >= 1.f ? 1.f : 0.5f + 0.5f * chargeElapsed;
			const EnemyFilterFn killFilter = makeKillFilter(scopeChargeScale);

			const HeroTraceTarget target = AcquireOrRefreshTarget(m_pLockedPawn, ctx, localPawnAddr, SnipeFov * m_flFovScale, killFilter);
			if (!target.IsValid())
			{
				ResetLock();
				break;
			}

			m_pLockedPawn = target.pPawn;
			m_vAimPoint = target.vAimPos;

			const bool bCanAttack = flNow > m_flNextAttackTime;
			const bool bNewScope = flNow - flScopeTime < 0.1f;

			if (!bNewScope && !bCanAttack)
				break;

			QAngle* pInputAngles = CCitadelInput_GetViewAngles(pInput, 0);
			if (!pInputAngles)
				break;

			const QAngle qTarget = Math::CalcAngle(ctx.vCamPos, m_vAimPoint);
			const QAngle qFinal = ComputeHeroAim(m_aimState, ctx, m_vAimPoint, m_pLockedPawn, SnipeSmoothX, SnipeSmoothY);

			*pInputAngles = qFinal;
			m_pending.qFinal = qFinal;

			if (!bCanAttack)
				break;

			QAngle qErr = qTarget - qFinal;
			Math::NormalizeAngles(qErr);
			const float errMag = sqrtf(qErr.m_x * qErr.m_x + qErr.m_y * qErr.m_y);

			if (errMag > k_flOnTargetThreshold)
				break;

			const float chargeCooldown = pSnipe->GetPropertyValue("AbilityCooldownBetweenCharge");
			const float fullCooldown = chargeCooldown + k_flAttackCooldown;

			m_pending.bFireAttack = true;
			m_flNextAttackTime = flNow + fullCooldown;
			break;
		}
	}
}

void CHornetComponent::OnPostMove(CCitadelInput* pInput)
{
	if (!m_pending.valid)
		return;

	if (m_pending.pActiveCam && m_pending.qFinal != QAngle{})
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

	if (m_pending.bHoldScope)
		pUserCmd->HoldButton(IN_ABILITY4);

	if (m_pending.bFireAttack)
		pUserCmd->TapButton(IN_ATTACK);
}

void CHornetComponent::OnRender()
{
	if (!SnipeActive || !SnipeShowFov)
		return;

	if (!ImGui::GetCurrentContext())
		return;

	auto* dl = ImGui::GetBackgroundDrawList();
	if (!dl)
		return;

	const auto& io = ImGui::GetIO();
	const ImVec2 vCX = {io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f};
	const auto col = ImColor(SnipeFovColor[0], SnipeFovColor[1], SnipeFovColor[2], 1.f);

	dl->AddCircle(vCX, SnipeFov * m_flFovScale, col, 64, 1.f);
}