#pragma once

#include <functional>

#include <DeadLock/SDK/Math/Vector3.hpp>
#include <DeadLock/SDK/Math/QAngle.hpp>
#include <ImGui/imgui.h>
#include <AndromedaClient/Features/CAimbot/CAimbotBones.hpp>

class CCitadelInput;
class CCitadelPlayerController;
class C_CitadelPlayerPawn;

struct HeroesContext
{
	CCitadelPlayerController* pLocalCtrl = nullptr;
	C_CitadelPlayerPawn*      pLocalPawn = nullptr;
	Vector3                   vCamPos    = {};
	QAngle                    qView      = {};
	ImVec2                    vCX        = {};
};

struct HeroTraceTarget
{
	C_CitadelPlayerPawn* pPawn   = nullptr;
	Vector3              vAimPos = {};

	bool IsValid() const { return pPawn != nullptr; }
};

// Per-component state for ComputeHeroAim. Store in the hero component, pass by ref.
struct HeroAimState
{
	QAngle               qPrevRawDir = {};
	C_CitadelPlayerPawn* pPrevPawn   = nullptr;

	void Reset() { *this = {}; }
};

using EnemyFilterFn = std::function<bool( CCitadelPlayerController*, C_CitadelPlayerPawn* )>;

bool GetHeroesContext( CCitadelInput* pInput, HeroesContext& outCtx );

HeroTraceTarget AcquireOrRefreshTarget(
	C_CitadelPlayerPawn*  pCurrent,
	const HeroesContext&  ctx,
	uintptr_t             localPawnAddr,
	float                 fovRadius,
	const EnemyFilterFn&  filter   = {},
	int                   boneMask = kBoneMaskAll );

// Smoothed aim toward vAimPoint.
// feedforward tracks target angular velocity to eliminate steady-state lag.
// IIR correction with sticky-zone taper closes the remaining error.
// bulletSpeed > 0  → leads the target using m_vecVelocity (linear intercept).
// bulletSpeed == 0 → instant / no lead (default).
QAngle ComputeHeroAim(
	HeroAimState&        state,
	const HeroesContext& ctx,
	const Vector3&       vAimPoint,
	C_CitadelPlayerPawn* pTargetPawn,
	float                sensX,
	float                sensY,
	float                bulletSpeed    = 0.f,
	float                gravityAccel   = 0.f,
	Vector3*             pOutPredicted  = nullptr,
	float                flInheritScale = 0.f );

void WriteViewAngles(
	CCitadelInput*       pInput,
	C_CitadelPlayerPawn* pLocalPawn,
	const QAngle&        qFinal );
