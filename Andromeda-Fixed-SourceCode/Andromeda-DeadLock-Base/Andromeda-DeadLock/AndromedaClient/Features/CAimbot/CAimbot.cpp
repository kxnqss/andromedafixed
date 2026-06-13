#include "CAimbot.hpp"
#include "CAimbotBones.hpp"

#include <numbers>
#include <Windows.h>
#include <algorithm>

#include <ImGui/imgui.h>

#include <AndromedaClient/CAndromedaGUI.hpp>
#include <AndromedaClient/Fonts/FontAwesomeIcon.hpp>
#include <AndromedaClient/Helpers/CTraceHelper.hpp>
#include <AndromedaClient/Features/CMisc/CMisc.hpp>
#include <DeadLock/SDK/FunctionListSDK.hpp>
#include <DeadLock/SDK/Interface/IEngineToClient.hpp>
#include <DeadLock/SDK/Math/Math.hpp>
#include <DeadLock/SDK/SDK.hpp>
#include <DeadLock/SDK/Types/CEntityData.hpp>
#include <GameClient/CEntityCache/CEntityCache.hpp>
#include <GameClient/CL_Bones.hpp>
#include <GameClient/CL_CitadelPlayerController.hpp>
#include <GameClient/CL_PrimaryWeapon.hpp>
#include "intercept.hpp"
#include "DeadLock/SDK/Interface/CGameEntitySystem.hpp"

static CAimbot g_CAimbot{};

// ─── Constants ────────────────────────────────────────────────────────────────

static constexpr float k_flHalfBaseFov = 90.f * (std::numbers::pi_v<float> / 360.f);

static constexpr int kVelWindow = 8; // origin ring-buffer size
static constexpr int kLockHysteresisFrames = 10; // grace frames before dropping a lock
static constexpr float kLockFovMult = 1.2f; // locked target FOV retention factor

static constexpr float kBoneTransSpeed = 2.0f; // 1/speed = bone transition duration (s)
static constexpr float kHumanWobbleDeg = 0.75f; // peak yaw wobble during bone switch (°)
static constexpr float kLateralFull = 0.06f; // below this lateral speed → full wobble
static constexpr float kLateralMin = 0.40f; // above this lateral speed → minimum wobble (kWobbleMinScale)
static constexpr float kWobbleMinScale = 0.20f; // wobble scale floor for fast-moving targets

static constexpr float kMaxAngVelPerFrame = 2.f; // feedforward angular velocity clamp (°/frame)
static constexpr float kTeleportThreshold = 500.f; // velocity ring: reject jumps above this
static constexpr float kMinWindowTime = 0.010f; // velocity ring: min elapsed time
static constexpr float kMaxInterceptTime = 8.f; // prediction: discard solutions beyond this (s)
static constexpr float kGravityAccel = 800.f; // game gravity (units/s²)
static constexpr float kServerVelMinLen = 10.f; // treat m_vecServerVelocity as stale below this

static constexpr int kAcqChance = 45; // % chance of acquisition overshoot on new lock
static constexpr float kAcqTriggerErr = 2.5f; // fire overshoot when aim error drops below this (°)
static constexpr float kAcqOvershootMin = 0.25f; // min overshoot magnitude (°)
static constexpr float kAcqOvershootMax = 0.70f; // max overshoot magnitude (°)
static constexpr float kAcqYawRatio = 0.35f; // yaw-to-pitch ratio of overshoot (less horizontal)
static constexpr float kAcqDuration = 0.20f; // overshoot decay time (s)

static constexpr float kInertiaBlend = 6.f; // inertia strength: kI = S/(S+kInertiaBlend); 0=instant, →1 at S→∞
static constexpr float kMaxVelBase = 6.f; // max °/frame at S=1; scales as Base/(S*0.5+1)
static constexpr float kStickyZoneDeg = 2.0f; // within this radius approach speed smoothstep-tapers to zero
static constexpr float kDriftStep = 0.04f; // random walk step per frame (°); active when locked on target
static constexpr float kDriftDecay = 0.93f; // drift decay toward 0 per frame
static constexpr float kDriftFullDist = 7'00.f; // units: full drift (1.0) at or below this
static constexpr float kDriftMidDist = 20'00.f; // units: linear 1.0→0.2 from Full to here
static constexpr float kDriftFarDist = 4'000.f; // units: linear 0.2→0.1 from Mid to here
static constexpr float kDriftMidScale = 0.2f; // scale at kDriftMidDist
static constexpr float kDriftMinScale = 0.1f; // floor scale at kDriftFarDist and beyond

// ─── AntiFrog state ───────────────────────────────────────────────────────────

static int s_nDmgHits = 0;
static int s_nDmgHeadHits = 0;
static float s_flHsPercent = 0.f;
static bool s_bBodySlot = false;
static bool s_bNeckSlot = false; // silent correction: aim neck instead of head
static int s_nSlotRemaining = 0;

// ─── Per-frame aimbot state ───────────────────────────────────────────────────
// All declared here (not as function statics) so the key-release path and
// helper functions can access them without extra parameters.

static C_CitadelPlayerPawn* s_lastPawn = nullptr;
static Vector3 s_lastHeadRaw = {};
static int s_lastBoneMask = 0;
static QAngle s_prevRawDir = {};

static Vector3 s_velRing[kVelWindow] = {};
static ULONGLONG s_velTimeRing[kVelWindow] = {};
static int s_velRingIdx = 0;
static int s_velRingFilled = 0;

static C_CitadelPlayerPawn* s_lockedPawn = nullptr;
static int s_lockLostFrames = 0;

static QAngle s_transOffsetAngle = {}; // angular gap (old bone − new bone) at switch time, pitch-only
static ULONGLONG s_transMsStart = 0;
static float s_transWobbleSign = 1.f;
static float s_transWobbleScale = 1.f; // locked at transition start: 1.0 stationary → kWobbleMinScale fast-moving
static bool s_inTransition = false;
static int s_lastBoneMaskForTrans = 0;

static bool s_acqPending = false; // waiting for aim to arrive before firing overshoot
static bool s_inAcqOvershoot = false; // overshoot is active and decaying
static QAngle s_acqOvershootOffset = {}; // signed overshoot magnitude + direction
static ULONGLONG s_acqMsStart = 0;

// Physics aim velocity state (°/frame), persists across frames to model hand inertia.
static float s_aimVelX = 0.f;
static float s_aimVelY = 0.f;

// Organic drift: slow random walk, active only when locked on target (errMag ≈ 0).
static float s_driftX = 0.f;
static float s_driftY = 0.f;
static uint32_t s_rng = 0x9E3779B9u;


// Sliding-window velocity from entity origin samples.
// Prefers m_vecVelocity when non-zero; falls back to origin delta.
static Vector3 EstimateTargetVelocity(C_CitadelPlayerPawn* pBestPawn, const Vector3& vHeadRaw, bool bSamePawn)
{
	const auto& nv = pBestPawn->m_vecVelocity();
	const Vector3 serverVel = {nv.x(), nv.y(), nv.z()};
	if (serverVel.Length() >= kServerVelMinLen)
		return serverVel;

	auto* pNode = pBestPawn->m_pGameSceneNode();
	const Vector3 origin = pNode ? pNode->m_vecAbsOrigin() : vHeadRaw;

	if (!bSamePawn)
	{
		s_velRingFilled = 0;
		s_velRingIdx = 0;
	}

	const ULONGLONG nowMs = GetTickCount64();
	s_velRing[s_velRingIdx] = origin;
	s_velTimeRing[s_velRingIdx] = nowMs;
	s_velRingIdx = (s_velRingIdx + 1) % kVelWindow;
	if (s_velRingFilled < kVelWindow)
		++s_velRingFilled;

	if (s_velRingFilled < kVelWindow)
		return {};

	const Vector3& oldest = s_velRing[s_velRingIdx];
	const ULONGLONG oldestMs = s_velTimeRing[s_velRingIdx];
	const Vector3 totalDelta = origin - oldest;
	const float windowTime = static_cast<float>(nowMs - oldestMs) * 0.001f;

	if (totalDelta.Length() >= kTeleportThreshold)
	{
		s_velRingFilled = 0;
		s_velRingIdx = 0;
		return {};
	}

	if (windowTime < kMinWindowTime)
		return {};

	Vector3 result = totalDelta / windowTime;
	return result;
}

// Solves projectile intercept; returns the predicted aim point.
// Returns vTarget unchanged if no valid solution is found.
static Vector3 SolveIntercept(
	const Vector3& vShooterPos,
	const Vector3& vBonePos,
	const Vector3& vBoneVel,
	float speed,
	float gravScale)
{
	const prediction::math::Vec3 pmSrc{vShooterPos.m_x, vShooterPos.m_y, vShooterPos.m_z};
	const prediction::math::Vec3 pmTgt{vBonePos.m_x, vBonePos.m_y, vBonePos.m_z};
	const prediction::math::Vec3 pmVel{vBoneVel.m_x, vBoneVel.m_y, vBoneVel.m_z};

	const float gravAccel = gravScale * kGravityAccel;

	auto r = gravAccel > 0.001f
		         ? prediction::math::solve_ballistic(pmSrc, pmTgt, pmVel, speed, gravAccel)
		         : prediction::math::solve_linear(pmSrc, pmTgt, pmVel, speed);

	if (!r.valid || r.time >= kMaxInterceptTime)
		return {};

	return {r.aim_point.x, r.aim_point.y, r.aim_point.z};
}

struct ScanResult
{
	C_CitadelPlayerPawn* pBestPawn = nullptr;
	Vector3 vBestHead = {};
	int nBestBoneMask = 0;

	C_CitadelPlayerPawn* pLockedFound = nullptr;
	Vector3 vLockedHead = {};
	int nLockedBoneMask = 0;
	float fLockedDist = FLT_MAX;
};

// Iterates the entity cache and returns the best aim candidate plus the
// currently locked pawn (if still visible).  All AntiFrog bone redirection
// is applied here so the caller sees only the final aim position.
static ScanResult ScanTargets(
	const Vector3& vCamPos,
	const ImVec2& vCX,
	uintptr_t localPawnAddr,
	CCitadelPlayerController* pLocalCtrl,
	float fFov,
	int bonesMask,
	bool antiFrog)
{
	ScanResult result;
	float fBestDist = fFov;

	auto* pCache = GetEntityCache();
	std::scoped_lock lock(pCache->GetLock());

	for (const auto& cached : pCache->GetEntitiesByType(CachedEntity_t::CITADEL_PLAYER_CONTROLLER))
	{
		auto* pEntity = cached.m_Handle.Get();
		if (!pEntity || !pEntity->pEntityIdentity())
			continue;

		auto* pCtrl = reinterpret_cast<CCitadelPlayerController*>(pEntity);
		if (pCtrl == pLocalCtrl)
			continue;

		if (pCtrl->m_iTeamNum() == pLocalCtrl->m_iTeamNum())
			continue;

		if (!pCtrl->m_PlayerDataGlobal().m_bAlive())
			continue;

		auto* pPawn = pCtrl->m_hHeroPawn().Get<C_CitadelPlayerPawn>();
		if (!pPawn || !pPawn->IsCitadelPlayerPawn())
			continue;

		static const CUtlStringToken k_EtherealShift("upgrade_self_bubble");
		if (pPawn->IsModifierActive(k_EtherealShift, "CCitadel_Modifier_Bubble"))
			continue;

		auto* pNode = pPawn->m_pGameSceneNode();
		if (!pNode)
			continue;

		if (!pPawn->m_CBodyComponent() || !pPawn->m_pRenderComponent())
			continue;

		auto* pSkelInst = pNode->GetSkeletonInstance();
		if (!pSkelInst || !pSkelInst->m_modelState().m_hModel().is_valid())
			continue;

		// AntiFrog: the invariant is "never aim Head", regardless of which slot
		// the hero's model actually provides data for.
		//
		// Strategy — strip bits from effectiveMask and let the slot-priority loop
		// (Neck → Torso → Arms → Legs) find the first slot that has hitbox data.
		// No manual redirect needed: if Neck has no data for this model, the loop
		// naturally falls through to Torso, then Arms, then Legs.
		//
		//   s_bNeckSlot — light correction; strip Head only.
		//   s_bBodySlot — heavy correction; strip Head + Neck, force body slots.
		int effectiveMask = bonesMask;
		if (antiFrog)
		{
			// Always remove Head — this is the core guarantee of AntiFrog.
			effectiveMask &= ~kBoneMaskHead;

			if (s_bBodySlot)
			{
				// Stronger correction: also skip Neck, push to Torso/Arms/Legs.
				effectiveMask &= ~kBoneMaskNeck;
			}

			// If the user had only Head enabled (effectiveMask now 0),
			// open all non-head slots so there is always something to aim at.
			if (effectiveMask == 0)
				effectiveMask = kBoneMaskAll & ~kBoneMaskHead;
		}

		// Find the highest-priority trace-visible bone for aiming,
		// and separately the bone closest to the crosshair for FOV selection.
		// For each slot we try bones in radius-descending order (BoneExtractor sorts them)
		// and stop at the first one that passes all checks — one bone per slot maximum.
		Vector3 vAimPos = {};
		bool bFound = false;
		int nSelMask = 0;
		float fClosest = FLT_MAX;

		for (const auto& bone : k_AimBones)
		{
			if (!(effectiveMask & bone.mask))
				continue;

			const auto* boneList = GetCL_Bones()->GetHitboxBones(pPawn, bone.slot);
			if (!boneList)
				continue;

			for (const int16_t bid : *boneList)
			{
				Vector3 bonePos;
				if (!pNode->GetBonePosition(static_cast<int>(bid), bonePos))
					continue;

				const TraceVec3 trFrom{.x = vCamPos.m_x, .y = vCamPos.m_y, .z = vCamPos.m_z};
				const TraceVec3 trTo{.x = bonePos.m_x, .y = bonePos.m_y, .z = bonePos.m_z};
				if (!Trace_IsVisibleEx(&trFrom, &trTo, localPawnAddr))
					continue;

				ImVec2 screen;
				if (!Math::WorldToScreen(bonePos, screen))
					continue;

				if (!bFound)
				{
					vAimPos = bonePos;
					nSelMask = bone.mask;
					bFound = true;
				}

				const float dx = screen.x - vCX.x;
				const float dy = screen.y - vCX.y;
				const float dist = sqrtf(dx * dx + dy * dy);
				fClosest = (std::min)(dist, fClosest);
				break; // first visible bone in this slot wins
			}
		}

		if (pPawn == s_lockedPawn && bFound)
		{
			result.pLockedFound = pPawn;
			result.vLockedHead = vAimPos;
			result.nLockedBoneMask = nSelMask;
			result.fLockedDist = fClosest;
		}

		if (!bFound || fClosest >= fBestDist)
			continue;

		fBestDist = fClosest;
		result.pBestPawn = pPawn;
		result.vBestHead = vAimPos;
		result.nBestBoneMask = nSelMask;
	}

	return result;
}

// Acquisition overshoot: fires once per lock when the aim has nearly arrived.
// Pushes qTarget slightly past the real position; the smooth controller follows,
// flies past the actual target, then the decaying offset pulls it back naturally.
static void ApplyAcquisitionOvershoot(QAngle& qTarget, const QAngle& qView)
{
	if (s_acqPending && !s_inAcqOvershoot)
	{
		QAngle qApproach = qTarget - qView;
		Math::NormalizeAngles(qApproach);
		const float err = sqrtf(qApproach.m_x * qApproach.m_x + qApproach.m_y * qApproach.m_y);
		if (err < kAcqTriggerErr)
		{
			s_acqPending = false;
			s_inAcqOvershoot = true;

			const float len = err > 0.001f ? err : 1.f;
			const float scale = kAcqOvershootMin + static_cast<float>(rand() % 1000) * 0.001f * (kAcqOvershootMax - kAcqOvershootMin);

			s_acqOvershootOffset.m_x = qApproach.m_x / len * scale;
			s_acqOvershootOffset.m_y = qApproach.m_y / len * scale * kAcqYawRatio;
			s_acqOvershootOffset.m_z = 0.f;

			s_acqMsStart = GetTickCount64();
		}
	}

	if (s_inAcqOvershoot)
	{
		const float elapsed = static_cast<float>(GetTickCount64() - s_acqMsStart) * 0.001f;
		if (elapsed >= kAcqDuration)
		{
			s_inAcqOvershoot = false;
		}
		else
		{
			const float t01 = elapsed / kAcqDuration;
			const float decay = 1.f - t01 * t01 * (3.f - 2.f * t01); // smoothstep 1→0
			qTarget.m_x += s_acqOvershootOffset.m_x * decay;
			qTarget.m_y += s_acqOvershootOffset.m_y * decay;
			Math::NormalizeAngles(qTarget);
		}
	}
}


// Bone transition: computes a QAngle delta to add to qTarget after prediction.
//
// Works entirely in angle space so the transition is a straight screen-space line
// regardless of geometry, and prediction (which operates on world-space positions)
// cannot interact with or amplify it.
//
// Called with the clean bone position BEFORE any prediction.
// s_lastHeadRaw must also be the clean bone position from the previous frame
// so the angular gap is exact and feedforward velocity estimation stays clean.
static QAngle ComputeTransitionDelta(
	const Vector3& vBestHead,
	int nBestBoneMask,
	const C_CitadelPlayerPawn* pBestPawn,
	const Vector3& vCamPos)
{
	const bool bSamePawn = s_lastPawn == pBestPawn;
	const bool maskChanged = s_lastBoneMaskForTrans != 0 && s_lastBoneMaskForTrans != nBestBoneMask;

	if (!bSamePawn)
	{
		s_inTransition = false;
	}
	else if (maskChanged && !s_inTransition)
	{
		const QAngle qOld = Math::CalcAngle(vCamPos, s_lastHeadRaw);
		const QAngle qNew = Math::CalcAngle(vCamPos, vBestHead);
		s_transOffsetAngle = qOld - qNew;
		Math::NormalizeAngles(s_transOffsetAngle);
		s_transOffsetAngle.m_y = 0.f;

		// Wobble scale locked once at start: smoothstep from 1.0 (stationary) down to
		// kWobbleMinScale (fast-moving). Locked here so there are no per-frame glitches.
		float yawDelta = qNew.m_y - qOld.m_y;
		if (yawDelta > 180.f)
			yawDelta -= 360.f;
		if (yawDelta < -180.f)
			yawDelta += 360.f;
		const float lat = fabsf(yawDelta);
		const float t01 = std::clamp((lat - kLateralFull) / (kLateralMin - kLateralFull), 0.f, 1.f);
		const float smooth01 = t01 * t01 * (3.f - 2.f * t01); // smoothstep
		s_transWobbleScale = 1.f - smooth01 * (1.f - kWobbleMinScale);

		s_transWobbleSign = rand() & 1 ? 1.f : -1.f;
		s_transMsStart = GetTickCount64();
		s_inTransition = true;

	}

	s_lastBoneMaskForTrans = nBestBoneMask;

	if (!s_inTransition)
		return {};

	const float t = static_cast<float>(GetTickCount64() - s_transMsStart) * 0.001f * kBoneTransSpeed;

	if (t >= 1.f)
	{
		s_inTransition = false;
		return {};
	}

	const float ts = t * t * (3.f - 2.f * t); // smoothstep
	const float decay = 1.f - ts;

	// sin(t*π) peaks at t=0.5 and is zero at both endpoints — clean entry and exit.
	// s_transWobbleScale is locked at transition start: 1.0 on stationary, kWobbleMinScale on fast.
	const float wobbleYaw = sinf(std::numbers::pi_v<float> * t) * kHumanWobbleDeg * s_transWobbleScale * s_transWobbleSign;

	QAngle qDelta;
	qDelta.m_x = s_transOffsetAngle.m_x * decay;
	qDelta.m_y = wobbleYaw;
	qDelta.m_z = 0.f;

	return qDelta;
}

// Physics-based aim smoothing simulating hand movement:
//   • Inertia   — velocity blends toward desired each frame (kI = S/(S+kInertiaBlend))
//                 low S → fast response, high S → heavy lag (more human at medium S)
//   • Max speed — physical cap so the crosshair can't snap instantly on acquisition
//   • Sticky    — smoothstep taper within kStickyZoneDeg: crosshair "settles" softly
//                 rather than vibrating on a locked target
//   • Feedforward — unchanged: eliminates steady-state lag on constant-velocity targets
//
// bSamePawn=false resets velocity so there is no inertia carryover to a new target.
static QAngle ComputeSmoothedAngle(
	const QAngle& qView,
	const QAngle& qTarget,
	const QAngle& qRawDir,
	bool bSamePawn,
	float sensX,
	float sensY,
	float inertiaScale,
	float driftScale,
	float distUnits)
{
	if (!bSamePawn)
	{
		s_aimVelX = 0.f;
		s_aimVelY = 0.f;
		s_driftX = 0.f;
		s_driftY = 0.f;
	}

	// ── Feedforward ────────────────────────────────────────────────────────
	QAngle qRawAngVel = {};
	if (bSamePawn && (s_prevRawDir.m_x != 0.f || s_prevRawDir.m_y != 0.f))
	{
		qRawAngVel = qRawDir - s_prevRawDir;
		Math::NormalizeAngles(qRawAngVel);
		qRawAngVel.m_x = std::clamp(qRawAngVel.m_x, -kMaxAngVelPerFrame, kMaxAngVelPerFrame);
		qRawAngVel.m_y = std::clamp(qRawAngVel.m_y, -kMaxAngVelPerFrame, kMaxAngVelPerFrame);
	}

	const float kffX = (sensX + 1.f) / std::max(sensX, 0.1f);
	const float kffY = (sensY + 1.f) / std::max(sensY, 0.1f);

	const float ffX = qRawAngVel.m_x * kffX;
	const float ffY = qRawAngVel.m_y * kffY;

	QAngle qViewFF = qView;
	qViewFF.m_x += ffX;
	qViewFF.m_y += ffY;
	Math::NormalizeAngles(qViewFF);

	// ── IIR correction step ────────────────────────────────────────────────
	QAngle qDiff = qTarget - qViewFF;
	Math::NormalizeAngles(qDiff);

	const float errMag = sqrtf(qDiff.m_x * qDiff.m_x + qDiff.m_y * qDiff.m_y);

	// ── Sticky zone: taper only the correction step (NOT feedforward) ──────
	// Feedforward tracks target velocity instantly → no overshoot on moving targets.
	// Inertia on correction only → natural settle lag on stationary targets.
	const float stickyT = (std::min)(1.f, errMag / kStickyZoneDeg);
	const float stickyScale = stickyT * stickyT * (3.f - 2.f * stickyT);

	// ── Distance-based drift attenuation (two-segment piecewise linear) ───
	// [0, 700]       → 1.0
	// [700, 2000]    → 1.0 → 0.2
	// [2000, 3500]   → 0.2 → 0.1
	// [3500, ∞]      → 0.1
	float distScale;
	if (distUnits <= kDriftFullDist)
		distScale = 1.f;
	else if (distUnits <= kDriftMidDist)
		distScale = 1.f + (kDriftMidScale - 1.f) * (distUnits - kDriftFullDist) / (kDriftMidDist - kDriftFullDist);
	else if (distUnits <= kDriftFarDist)
		distScale = kDriftMidScale + (kDriftMinScale - kDriftMidScale) * (distUnits - kDriftMidDist) / (kDriftFarDist - kDriftMidDist);
	else
		distScale = kDriftMinScale;

	// ── Organic drift: active only when locked on target (stickyScale ≈ 0) ─
	// Amplitude = kDriftStep × smooth_ramp × distScale × user DriftScale.
	// smooth ramp: S=1→ ~0, S=20→ ×1.0 (human-like feel scales with smooth)
	{
		auto xorshift = [](uint32_t& s) -> float
		{
			s ^= s << 13;
			s ^= s >> 17;
			s ^= s << 5;
			return static_cast<float>(static_cast<int32_t>(s)) * (1.f / 2147483648.f);
		};
		const float driftAmp = kDriftStep * (sensX / 20.f) * distScale * driftScale;
		s_driftX = s_driftX * kDriftDecay + xorshift(s_rng) * driftAmp;
		s_driftY = s_driftY * kDriftDecay + xorshift(s_rng) * driftAmp;
	}
	const float lockScale = 1.f - stickyScale; // 1.0 on target, 0.0 at sticky edge

	const float iirX = (qDiff.m_x / (sensX + 1.f)) * stickyScale + s_driftX * lockScale;
	const float iirY = (qDiff.m_y / (sensY + 1.f)) * stickyScale + s_driftY * lockScale;

	// ── Inertia on IIR correction step only ───────────────────────────────
	// effectiveBlend = kInertiaBlend / inertiaScale:
	//   inertiaScale=0.1 → blend÷0.1 → kI→1 → very heavy inertia (slow)
	//   inertiaScale=1.0 → nominal
	//   inertiaScale=2.0 → blend÷2 → kI smaller → snappier inertia
	const float effectiveBlend = kInertiaBlend / std::max(inertiaScale, 0.01f);
	const float kIX = sensX / (sensX + effectiveBlend);
	const float kIY = sensY / (sensY + effectiveBlend);
	s_aimVelX = s_aimVelX * kIX + iirX * (1.f - kIX);
	s_aimVelY = s_aimVelY * kIY + iirY * (1.f - kIY);

	// ── Max velocity cap on correction ────────────────────────────────────
	const float maxVX = kMaxVelBase / (sensX * 0.5f + 1.f);
	const float maxVY = kMaxVelBase / (sensY * 0.5f + 1.f);
	s_aimVelX = std::clamp(s_aimVelX, -maxVX, maxVX);
	s_aimVelY = std::clamp(s_aimVelY, -maxVY, maxVY);

	// ── Apply: feedforward instant, correction with inertia ────────────────
	// qViewFF = qView + ff  (ff instant)
	// qFinal  = qViewFF + vel (vel = lazy correction)
	QAngle qFinal;
	qFinal.m_x = qViewFF.m_x + s_aimVelX;
	qFinal.m_y = qViewFF.m_y + s_aimVelY;
	qFinal.m_z = 0.f;

	return qFinal;
}

// ─────────────────────────────────────────────────────────────────────────────

void CAimbot::OnDamage(int attacker_entidx, int victim_entidx, int hitgroup_id)
{
	if (hitgroup_id <= 0)
		return;

	auto* pVictimEnt = SDK::Interfaces::GameEntitySystem()->GetBaseEntity(victim_entidx);
	if (!pVictimEnt || !pVictimEnt->IsCitadelPlayerPawn())
		return;

	auto* pLocalCtrl = GetCL_CitadelPlayerController()->GetLocal();
	if (!pLocalCtrl)
		return;

	auto* pLocalPawn = pLocalCtrl->m_hHeroPawn().Get<C_CitadelPlayerPawn>();
	if (!pLocalPawn)
		return;

	auto* pIdentity = pLocalPawn->pEntityIdentity();
	if (!pIdentity)
		return;

	if (attacker_entidx != pIdentity->Handle().GetEntryIndex())
		return;

	++s_nDmgHits;
	if (hitgroup_id == 1) // HITGROUP_HEAD
		++s_nDmgHeadHits;

	s_flHsPercent = static_cast<float>(s_nDmgHeadHits) / static_cast<float>(s_nDmgHits) * 100.f;

	if (!AntiFrog)
		return;

	if (--s_nSlotRemaining <= 0)
	{
		const float ratio = s_flHsPercent / HsThreshold;

		// Three-tier correction:
		//   head  — when below or on target
		//   neck  — soft invisible correction (aim barely moves, hit ≠ headshot)
		//   body  — strong correction when well above target
		//
		// Neck slots are used first because the transition is nearly invisible;
		// body slots only when neck isn't enough.
		const float rnd = static_cast<float>(rand()) / RAND_MAX;

		s_bBodySlot = false;
		s_bNeckSlot = false;

		if (ratio < 0.8f)
		{
			// Well below target — head only
		}
		else if (ratio < 1.0f)
		{
			// Slightly below target — start mixing in neck (0 → 35%)
			const float neckProb = (ratio - 0.8f) / 0.2f * 0.35f;
			s_bNeckSlot = rnd < neckProb;
		}
		else if (ratio < 1.2f)
		{
			// Above target — neck more likely (35 → 65%), body starts appearing
			const float neckProb = 0.35f + (ratio - 1.0f) / 0.2f * 0.30f;
			const float bodyProb = (ratio - 1.0f) / 0.2f * 0.20f;

			if (rnd < bodyProb)
				s_bBodySlot = true;
			else if (rnd < bodyProb + neckProb)
				s_bNeckSlot = true;
		}
		else
		{
			// Well above target — mostly body, some neck
			constexpr float bodyProb = 0.75f;
			constexpr float neckProb = 0.15f;

			if (rnd < bodyProb)
				s_bBodySlot = true;
			else if (rnd < bodyProb + neckProb)
				s_bNeckSlot = true;
		}

		// Head/neck slots can be slightly longer — aim barely changes.
		// Body slots shorter — stronger correction, don't overshoot.
		s_nSlotRemaining = s_bBodySlot ? 2 + rand() % 3 : 3 + rand() % 3;
	}
}

// ─────────────────────────────────────────────────────────────────────────────

void CAimbot::OnCreateMove(CCitadelInput* pInput, CUserCmd* pUserCmd)
{
	if (!Active)
		return;

	auto* pCamMgr = GetCCitadelCameraManager();
	if (!pCamMgr)
		return;

	CCitadel_Camera* pActiveCam = pCamMgr->GetActiveCamera();
	if (!pActiveCam)
		return;

	const float flCamFov = pActiveCam->m_flFov();
	const float flHalfCur = flCamFov * (std::numbers::pi_v<float> / 360.f);
	m_flFovScale = flCamFov > 1.f ? tanf(k_flHalfBaseFov) / tanf(flHalfCur) : 1.f;

	if (AimKey != 0 && !(GetAsyncKeyState(AimKey) & 0x8000))
	{
		s_lastPawn = nullptr;
		s_lastBoneMask = 0;
		s_inTransition = false;
		s_lastBoneMaskForTrans = 0;
		s_lockedPawn = nullptr;
		s_lockLostFrames = 0;
		s_prevRawDir = {};
		s_velRingFilled = 0;
		s_velRingIdx = 0;
		s_aimVelX = 0.f;
		s_aimVelY = 0.f;
		s_driftX = 0.f;
		s_driftY = 0.f;
		return;
	}

	if (!ImGui::GetCurrentContext())
		return;

	Trace_EnsureReady();

	QAngle* pInputAngles = CCitadelInput_GetViewAngles(pInput, 0);
	if (!pInputAngles)
		return;

	auto* pLocalCtrl = GetCL_CitadelPlayerController()->GetLocal();
	if (!pLocalCtrl)
		return;

	auto* pLocalPawn = pLocalCtrl->m_hHeroPawn().Get<C_CitadelPlayerPawn>();
	if (!pLocalPawn)
		return;

	// Suspend while reloading
	auto* pWeapon = GetCL_PrimaryWeapon()->Get();
	if (!pWeapon || pWeapon->m_bInReload())
		return;

	// Update cached bullet parameters from VData
	if (CitadelAbilityVData* pVData = pWeapon->m_pSubclassVData())
	{
		const float speed = pVData->m_WeaponInfo().m_flBulletSpeed();
		m_flCachedBulletSpeed = speed;
		m_flCachedBulletGravity = pVData->m_WeaponInfo().m_flBulletGravityScale();
	}

	auto* pLocalNode = pLocalPawn->m_pGameSceneNode();
	if (!pLocalNode)
		return;

	Vector3 vCamPos = pLocalNode->m_vecAbsOrigin();
	const Vector3& vPos = pActiveCam->m_vecPosition();
	if (vPos.m_x != 0.f || vPos.m_y != 0.f || vPos.m_z != 0.f)
		vCamPos = vPos;

	const float* ang = pActiveCam->m_angView();
	QAngle qView(ang[0], ang[1], 0.f);

	const ImVec2 vCX(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);

	const uintptr_t localPawnAddr = reinterpret_cast<uintptr_t>(pLocalPawn);
	ScanResult scan = ScanTargets(vCamPos, vCX, localPawnAddr, pLocalCtrl, Fov * m_flFovScale, BonesMask, AntiFrog);

	// ── Target lock resolution ─────────────────────────────────────────────────
	if (s_lockedPawn)
	{
		if (scan.pLockedFound && scan.fLockedDist < Fov * kLockFovMult)
		{
			s_lockLostFrames = 0;
			scan.pBestPawn = scan.pLockedFound;
			scan.vBestHead = scan.vLockedHead;
			scan.nBestBoneMask = scan.nLockedBoneMask;
		}
		else if (++s_lockLostFrames < kLockHysteresisFrames)
		{
			scan.pBestPawn = nullptr;
		}
		else
		{
			s_lockedPawn = nullptr;
			s_lockLostFrames = 0;
			s_acqPending = false;
			s_inAcqOvershoot = false;
		}
	}

	if (!s_lockedPawn && scan.pBestPawn)
	{
		s_lockedPawn = scan.pBestPawn;
		s_inAcqOvershoot = false;
		s_acqPending = (rand() % 100) < kAcqChance;
	}

	if (!scan.pBestPawn)
		return;

	// ── Bone transition ────────────────────────────────────────────────────────
	// vHeadRaw is captured BEFORE the transition delta so that:
	//   1. s_lastHeadRaw stays clean → angular gap at next switch is exact
	//   2. Prediction sees actual bone velocity, not transition drift
	//   3. qRawDir (feedforward) has no wobble/offset derivative
	const Vector3 vHeadRaw = scan.vBestHead;
	const bool bSamePawn = (s_lastPawn == scan.pBestPawn) && (s_lastBoneMask == scan.nBestBoneMask);
	const QAngle qTransDelta = ComputeTransitionDelta(scan.vBestHead, scan.nBestBoneMask, scan.pBestPawn, vCamPos);

	// ── Prediction ────────────────────────────────────────────────────────────
	if (m_flCachedBulletSpeed > 100.f)
	{
		const Vector3 targetVel = EstimateTargetVelocity(scan.pBestPawn, vHeadRaw, bSamePawn);
		scan.vBestHead = SolveIntercept(vCamPos, vHeadRaw, targetVel, m_flCachedBulletSpeed, m_flCachedBulletGravity);
	}

	s_lastPawn = scan.pBestPawn;
	s_lastHeadRaw = vHeadRaw;
	s_lastBoneMask = scan.nBestBoneMask;

	// ── Aim ───────────────────────────────────────────────────────────────────
	QAngle qPunch;
	CPlayer_CameraServices* pCamSvc = pLocalPawn->m_pCameraServices();
	if (!pCamSvc)
		return;

	const float* punch = &pCamSvc->m_flRecoilPunch(); // [0]=pitch, [1]=yaw
	qPunch.m_x = punch[0];
	qPunch.m_y = punch[1];

	QAngle qTarget = Math::CalcAngle(vCamPos, scan.vBestHead);
	qTarget.m_x -= qPunch.m_x;
	qTarget.m_y -= qPunch.m_y;

	// Apply transition delta in angle space after prediction so prediction
	// cannot interact with or amplify it.
	qTarget.m_x += qTransDelta.m_x;
	qTarget.m_y += qTransDelta.m_y;
	Math::NormalizeAngles(qTarget);

	QAngle qRawDir = Math::CalcAngle(vCamPos, vHeadRaw);
	qRawDir.m_x -= qPunch.m_x;
	qRawDir.m_y -= qPunch.m_y;

	// ── Acquisition overshoot ─────────────────────────────────────────────────
	ApplyAcquisitionOvershoot(qTarget, qView);

	const float distToTarget = (scan.vBestHead - vCamPos).Length();
	QAngle qFinal = ComputeSmoothedAngle(qView, qTarget, qRawDir, bSamePawn, SensitivityX, SensitivityY, InertiaScale, DriftScale,
	                                     distToTarget);

	s_prevRawDir = qRawDir;
	qFinal.m_x = std::clamp(qFinal.m_x, -89.f, 89.f);
	Math::NormalizeAngles(qFinal);

	*pInputAngles = qFinal;
	m_pending.valid = true;
	m_pending.pActiveCam = pActiveCam;
	m_pending.qFinal = qFinal;
	m_pending.pawnAddr = reinterpret_cast<uintptr_t>(pLocalPawn);
}

// ─────────────────────────────────────────────────────────────────────────────
// Called AFTER the original CreateMove so our camera angles override the engine.

void CAimbot::OnPostMove(CCitadelInput* pInput)
{
	if (!m_pending.valid)
		return;

	m_pending.valid = false;

	if (m_pending.pActiveCam)
		m_pending.pActiveCam->SetAngles(m_pending.qFinal.m_x, m_pending.qFinal.m_y);

	auto pLocalPawn = reinterpret_cast<C_CitadelPlayerPawn*>(m_pending.pawnAddr);
	pLocalPawn->m_angClientCamera() = m_pending.qFinal;
}

// ─────────────────────────────────────────────────────────────────────────────

void CAimbot::OnRender()
{
	if (!Active)
		return;

	// Reset HS counters on map change / disconnect
	{
		const bool bInGame = SDK::Interfaces::EngineToClient()->IsInGame();
		static bool s_bWasInGame = false;

		if (s_bWasInGame && !bInGame)
		{
			s_nDmgHits = 0;
			s_nDmgHeadHits = 0;
			s_flHsPercent = 0.f;
			s_bBodySlot = false;
			s_bNeckSlot = false;
			s_nSlotRemaining = 0;
		}

		s_bWasInGame = bInGame;

		if (!bInGame)
			return;
	}

	if (!ImGui::GetCurrentContext())
		return;

	auto* dl = ImGui::GetBackgroundDrawList();
	if (!dl)
		return;

	const auto& io = ImGui::GetIO();
	if (ShowFov)
	{
		const ImVec2 vCX = {io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f};
		const auto col = ImColor(FovColor[0], FovColor[1], FovColor[2], 1.f);
		dl->AddCircle(vCX, Fov * m_flFovScale, col, 64, 1.f);
	}

	if (!AntiFrog)
		return;

	// ── AntiFrog overlay ──────────────────────────────────────────────────────
	// Colour reflects whether HS% is above the user's threshold, not the current slot.
	// Label shows the actual active slot (head / neck / body).
	const bool bAboveThreshold = s_nDmgHits > 0 && s_flHsPercent > HsThreshold;
	const char* szSlotLabel = s_bBodySlot ? "[BODY]" : (s_bNeckSlot ? "[NECK]" : "[HEAD]");
	constexpr ImVec4 colAbove{1.f, 0.35f, 0.35f, 1.f};
	constexpr ImVec4 colOk{0.35f, 1.f, 0.35f, 1.f};
	constexpr ImVec4 colNeutral{0.5f, 0.5f, 0.5f, 1.f};

	ImGui::SetNextWindowPos(ImVec2(10.f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(0.f, 0.f));

	const float fMenuAlpha = static_cast<float>(GetMisc()->MenuAlpha) / 255.f;
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, fMenuAlpha);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
	ImGui::PushStyleColor(ImGuiCol_WindowShadow, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_NavHighlight, ImVec4(0.f, 0.f, 0.f, 0.f));

	const float fSavedShadow = ImGui::GetStyle().WindowShadowSize;
	ImGui::GetStyle().WindowShadowSize = 0.f;

	constexpr ImGuiWindowFlags overlayFlags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoFocusOnAppearing;

	ImGui::Begin(XorStr("##AntiFrogOverlay"), nullptr, overlayFlags);
	ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.f), "HS%%");
	ImGui::SameLine();
	ImGui::TextColored(bAboveThreshold ? colAbove : colOk, "%.1f%%", s_flHsPercent);
	ImGui::SameLine();
	ImGui::TextColored(s_bBodySlot ? colAbove : s_bNeckSlot ? ImVec4(1.f, 0.75f, 0.2f, 1.f) : colNeutral, "%s", szSlotLabel);
	ImGui::End();

	ImGui::GetStyle().WindowShadowSize = fSavedShadow;
	ImGui::PopStyleColor(4);
	ImGui::PopStyleVar(2);
}

CAimbot* GetAimbot()
{
	return &g_CAimbot;
}

const char* CAimbot::GetName() const
{
	return "Aimbot";
}

const char8_t* CAimbot::GetIcon() const
{
	return ICON_FA_CROSSHAIRS;
}
