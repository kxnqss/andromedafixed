#pragma once

#include <Common/Common.hpp>
#include <AndromedaClient/Features/CHeroes/IHeroComponent.hpp>
#include <AndromedaClient/Features/CHeroes/CHeroesHelpers.hpp>
#include <DeadLock/SDK/Math/Vector3.hpp>
#include <GameClient/CL_AbilityCache.hpp>
#include <DeadLock/SDK/Update/CCitadelCamera.hpp>

class C_CitadelPlayerPawn;

class CHazeComponent final : public IHeroComponent
{
public:
	uint32_t GetHeroID() const override { return 13u; }

	void Load(const rapidjson::Value& v)              override;
	void Save(JsonWriter& w)                          override;
	void RenderMenu()                                 override;
	void OnCreateMove(CCitadelInput*, CUserCmd*)      override;
	void OnPostMove(CCitadelInput*)                   override;
	void OnRender()                                   override;

	// ── Settings ──────────────────────────────────────────────────────────────
	bool  DaggerActive       = true;
	float DaggerFov          = 120.f;
	bool  DaggerShowFov      = false;
	float DaggerFovColor[3]  = { 1.f, 1.f, 1.f };
	float DaggerSmoothX      = 4.f;
	float DaggerSmoothY      = 4.f;

private:
	CL_AbilityCache m_SleepDagger { EAbilitySlots_t::ESlot_Signature_1 };

	enum class EHazeDaggerState { Idle, Aiming };
	EHazeDaggerState     m_eState          = EHazeDaggerState::Idle;
	bool                 m_bWasButtonHeld  = false;

	HeroAimState         m_aimState       = {};
	float                m_flFovScale     = 1.f;
	Vector3              m_vAimPoint      = {};
	C_CitadelPlayerPawn* m_pLockedPawn    = nullptr;
	float                m_flPostFireUntil = 0.f;

	struct DaggerPending
	{
		bool             valid       = false;
		CCitadel_Camera* pActiveCam  = nullptr;
		QAngle           qFinal      = {};
		uintptr_t        pawnAddr    = 0;
		bool             bFireDagger = false;
	} m_pending;

	void ResetLock();
};
