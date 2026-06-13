#include "CAutoParryComponent.hpp"

#include <numbers>
#include <utility>

#include <AndromedaClient/GUI/CAndromedaMenu.hpp>
#include <AndromedaClient/GUI/CMenuWidgets.hpp>
#include <AndromedaClient/Helpers/CTraceHelper.hpp>

#include <DeadLock/SDK/Types/CEntityData.hpp>
#include <DeadLock/SDK/Math/QAngle.hpp>
#include <DeadLock/SDK/Interface/CGameEntitySystem.hpp>
#include <GameClient/CL_CitadelPlayerController.hpp>

#include "DeadLock/SDK/Update/CCitadelInput.hpp"

// ── Constants ─────────────────────────────────────────────────────────────────

static constexpr auto kChargeSound = "Player.Melee.Hold.Shared";
static constexpr ULONGLONG kMinElapsedForFacingMs = 350;
static constexpr ULONGLONG kMaxChargeMs = 600;
static constexpr float kParryRange = 850.f;
static constexpr float kParryPointBlank = 320.f;
static constexpr float kPointBlankFlat = 60.f;
static constexpr float kFacingCosMin = 0.924f;
static constexpr float kFacingCosMinCorner = 0.707f;
static constexpr ULONGLONG kCornerApproachMs = 150;
static constexpr float kMaxVerticalDiff = 160.f;

// ── Helpers ───────────────────────────────────────────────────────────────────

static Vector3 AngleToForward(const QAngle& ang)
{
	const float pitchRad = ang.m_x * (std::numbers::pi_v<float> / 180.f);
	const float yawRad = ang.m_y * (std::numbers::pi_v<float> / 180.f);
	return {cosf(pitchRad) * cosf(yawRad), cosf(pitchRad) * sinf(yawRad), -sinf(pitchRad)};
}

// ── Load / Save ───────────────────────────────────────────────────────────────

void CAutoParryComponent::Load(const rapidjson::Value& v)
{
	GetBoolJson(v, XorStr("AutoParry"), Enabled);
	GetFloatJson(v, XorStr("AutoParryFov"), Fov, 30.f, 180.f);
}

void CAutoParryComponent::Save(JsonWriter& w)
{
	AddBoolJson(w, XorStr("AutoParry"), Enabled);
	AddFloatJson(w, XorStr("AutoParryFov"), Fov);
}

// ── Menu ──────────────────────────────────────────────────────────────────────

void CAutoParryComponent::RenderMenu()
{
	const ImVec2 arrowMax = CMenuWidgets::RenderExpandableToggle({
		XorStr("Auto Parry"),
		XorStr("##Misc.AutoParry"),
		XorStr("##AutoParryPopup"),
		&Enabled
	});

	if (CMenuWidgets::BeginAnimatedPopup(XorStr("##AutoParryPopup"), arrowMax))
	{
		GetAndromedaMenu()->RenderSliderFloat({XorStr("FOV"), XorStr("##Misc.AutoParryFov"), Fov, 30.f, 180.f, 60.f});
		ImGui::EndPopup();
	}
}

// ── Callbacks ─────────────────────────────────────────────────────────────────

void CAutoParryComponent::OnStartSound(const Vector3&, int SourceEntityIndex, const char* szSoundName)
{
	HandleSound(SourceEntityIndex, szSoundName);
}

void CAutoParryComponent::OnPostMove(CCitadelInput* pInput)
{
	Tick(pInput);
}

// ── Logic ─────────────────────────────────────────────────────────────────────

void CAutoParryComponent::HandleSound(int entityIndex, const char* szSoundName)
{
	if (!Enabled || entityIndex <= 0 || strcmp(szSoundName, kChargeSound) != 0)
		return;

	std::scoped_lock lk(m_mutex);
	if (!m_pendingCharges.contains(entityIndex))
		m_pendingCharges[entityIndex] = MeleeCharge{.startMs = GetTickCount64()};
}

void CAutoParryComponent::Tick(CCitadelInput* pInput)
{
	if (!Enabled)
		return;

	std::scoped_lock lk(m_mutex);
	if (m_pendingCharges.empty())
		return;

	auto* pLocalCtrl = GetCL_CitadelPlayerController()->GetLocal();
	if (!pLocalCtrl)
		return;

	auto* pLocalPawn = pLocalCtrl->m_hHeroPawn().Get<C_CitadelPlayerPawn>();
	if (!pLocalPawn)
		return;

	auto* pLocalNode = pLocalPawn->m_pGameSceneNode();
	if (!pLocalNode)
		return;

	const Vector3 vLocalPos = pLocalNode->m_vecAbsOrigin();
	const int localTeam = pLocalCtrl->m_iTeamNum();
	const ULONGLONG now = GetTickCount64();
	const uintptr_t localPawnAddr = reinterpret_cast<uintptr_t>(pLocalPawn);

	Trace_EnsureReady();

	for (auto it = m_pendingCharges.begin(); it != m_pendingCharges.end();)
	{
		MeleeCharge& charge = it->second;

		if (now - charge.startMs > kMaxChargeMs)
		{
			it = m_pendingCharges.erase(it);
			continue;
		}

		auto* pAttacker = SDK::Interfaces::GameEntitySystem()->GetBaseEntity(it->first);
		if (!pAttacker || std::cmp_equal(pAttacker->m_iTeamNum(), localTeam))
		{
			it = m_pendingCharges.erase(it);
			continue;
		}

		auto* pAttackerPawn = reinterpret_cast<C_CitadelPlayerPawn*>(pAttacker);
		auto* pAttackerNode = pAttackerPawn ? pAttackerPawn->m_pGameSceneNode() : nullptr;
		if (!pAttackerNode)
		{
			++it;
			continue;
		}

		const Vector3 vAttackerPos = pAttackerNode->m_vecAbsOrigin();

		if (!charge.posInitialized)
		{
			charge.initialPos = vAttackerPos;
			charge.initialDist = vLocalPos.Distance(vAttackerPos);
			charge.posInitialized = true;
			++it;
			continue;
		}

		const float dist = vLocalPos.Distance(vAttackerPos);
		const ULONGLONG elapsed = now - charge.startMs;

		if (dist > kParryRange || dist > kParryPointBlank)
		{
			++it;
			continue;
		}
		if (fabsf(vAttackerPos.m_z - vLocalPos.m_z) > kMaxVerticalDiff)
		{
			++it;
			continue;
		}

		{
			const TraceVec3 trFrom{.x = vLocalPos.m_x, .y = vLocalPos.m_y, .z = vLocalPos.m_z};
			const TraceVec3 trTo{.x = vAttackerPos.m_x, .y = vAttackerPos.m_y, .z = vAttackerPos.m_z};

			if (!Trace_IsVisibleEx(&trFrom, &trTo, localPawnAddr))
			{
				charge.firstVisibleMs = 0;
				++it;
				continue;
			}

			if (charge.firstVisibleMs == 0)
				charge.firstVisibleMs = now;
		}

		const float flatDx = vAttackerPos.m_x - vLocalPos.m_x;
		const float flatDy = vAttackerPos.m_y - vLocalPos.m_y;
		const float flatDist = sqrtf(flatDx * flatDx + flatDy * flatDy);
		const bool bPointBlank = flatDist < kPointBlankFlat;

		if (!bPointBlank)
		{
			const Vector3 vToAttacker = (vAttackerPos - vLocalPos).Normalized();
			const Vector3 vLocalForward = AngleToForward(pLocalPawn->m_angClientCamera());
			if (vLocalForward.Dot(vToAttacker) < cosf(Fov * (std::numbers::pi_v<float> / 180.f)))
			{
				++it;
				continue;
			}
		}

		if (!bPointBlank)
		{
			if (elapsed < kMinElapsedForFacingMs)
			{
				++it;
				continue;
			}

			const bool bHadLos = charge.firstVisibleMs > 0;
			const bool bCornerApproach = bHadLos && (charge.firstVisibleMs - charge.startMs) >= kCornerApproachMs;
			const float facingThreshold = bCornerApproach ? kFacingCosMinCorner : kFacingCosMin;

			const Vector3 vForward3D = AngleToForward(pAttackerPawn->m_angEyeAngles());
			const Vector3 vForwardFlat = Vector3{vForward3D.m_x, vForward3D.m_y, 0.f}.Normalized();
			const Vector3 vToUsFlat = Vector3{-flatDx, -flatDy, 0.f}.Normalized();

			if (vForwardFlat.Dot(vToUsFlat) < facingThreshold)
			{
				++it;
				continue;
			}
		}

		auto* pUserCmd = pInput->GetUserCmd(pLocalCtrl);
		if (!pUserCmd)
			return;

		m_pendingCharges.clear();
		pUserCmd->TapButton(IN_ABILITY_HELD);
		return;
	}
}