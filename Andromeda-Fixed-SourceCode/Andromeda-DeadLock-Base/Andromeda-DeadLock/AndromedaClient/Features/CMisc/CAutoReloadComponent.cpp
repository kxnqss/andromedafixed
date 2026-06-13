#include "CAutoReloadComponent.hpp"

#include <AndromedaClient/GUI/CAndromedaMenu.hpp>
#include <AndromedaClient/GUI/CMenuWidgets.hpp>

#include <DeadLock/SDK/SDK.hpp>
#include <DeadLock/SDK/Types/CEntityData.hpp>
#include <DeadLock/SDK/Types/C_CitadelGameRules.hpp>
#include <DeadLock/SDK/Update/CCitadelInput.hpp>
#include <GameClient/CL_PrimaryWeapon.hpp>
#include <GameClient/CL_CitadelPlayerController.hpp>

static constexpr float kWindowHalfSec = 0.1f;

// ── Load / Save ───────────────────────────────────────────────────────────────

void CAutoReloadComponent::Load(const rapidjson::Value& v)
{
	GetBoolJson(v, XorStr("AutoActiveReload"), Enabled);
	GetBoolJson(v, XorStr("PreventFail"), PreventFail);
}

void CAutoReloadComponent::Save(JsonWriter& w)
{
	AddBoolJson(w, XorStr("AutoActiveReload"), Enabled);
	AddBoolJson(w, XorStr("PreventFail"), PreventFail);
}

// ── Menu ──────────────────────────────────────────────────────────────────────

void CAutoReloadComponent::RenderMenu()
{
	const ImVec2 arrowMax = CMenuWidgets::RenderExpandableToggle({
		XorStr("Auto Active Reload"),
		XorStr("##Misc.AutoReload"),
		XorStr("##AutoReloadPopup"),
		&Enabled
	});

	if (CMenuWidgets::BeginAnimatedPopup(XorStr("##AutoReloadPopup"), arrowMax))
	{
		GetAndromedaMenu()->RenderCheckBox(XorStr("Prevent Fail"), XorStr("##Misc.AutoReload.PreventFail"), PreventFail);
		ImGui::EndPopup();
	}
}

// ── Callbacks ─────────────────────────────────────────────────────────────────

void CAutoReloadComponent::OnPostMove(CCitadelInput* pInput)
{
	Tick(pInput);
}

// ── Logic ─────────────────────────────────────────────────────────────────────

void CAutoReloadComponent::Tick(CCitadelInput* pInput)
{
	if (!Enabled)
		return;

	auto* pLocalCtrl = GetCL_CitadelPlayerController()->GetLocal();
	auto* pWeapon = GetCL_PrimaryWeapon()->Get();
	auto* pGameRules = SDK::Pointers::GameRules();
	if (!pLocalCtrl || !pWeapon || !pGameRules)
		return;

	const bool bReload = pWeapon->m_bInReload();
	const bool bCanAR = pWeapon->m_bCanActiveReload();
	const float flStart = pWeapon->m_flLastReloadStartTime();
	const float flEnd = pWeapon->m_flNextPrimaryAttack();
	const float flPause = pWeapon->m_flAttackDelayPauseTotalTime();

	const float flNow = pGameRules->GetMatchTime();
	const float flDur = (flEnd - flStart + flPause) * 0.5f;
	const float flMid = flStart + flDur - kWindowHalfSec;
	const bool bCanArm = bCanAR && flStart > 0.f && flEnd > flStart && flMid > flNow;

	if (!bReload)
		m_flFireAt = 0.f;
	else if (!m_bWasInReload && bCanArm)
		m_flFireAt = flMid;
	else
		if (m_flFireAt > 0.f)
			m_flFireAt = flMid;

	m_bWasInReload = bReload;

	auto* pUserCmd = pInput->GetUserCmd(pLocalCtrl);
	if (!pUserCmd)
		return;

	if (PreventFail && m_flFireAt > 0.f)
		pUserCmd->ClearButton(IN_RELOAD);

	if (m_flFireAt <= 0.f || flNow < m_flFireAt)
		return;

	m_flFireAt = 0.f;
	pUserCmd->TapButton(IN_RELOAD);
}