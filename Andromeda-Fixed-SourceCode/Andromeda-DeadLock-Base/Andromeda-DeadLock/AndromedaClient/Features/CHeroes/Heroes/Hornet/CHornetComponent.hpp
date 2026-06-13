#pragma once

#include <Common/Common.hpp>
#include <AndromedaClient/Features/CHeroes/IHeroComponent.hpp>
#include <AndromedaClient/Features/CHeroes/CHeroesHelpers.hpp>
#include <GameClient/CL_AbilityCache.hpp>
#include <DeadLock/SDK/Math/Vector3.hpp>
#include <DeadLock/SDK/Update/CCitadelCamera.hpp>

class C_CitadelPlayerPawn;
class CCitadel_Ability_Hornet_Snipe;

class CHornetComponent final : public IHeroComponent
{
public:
	uint32_t GetHeroID() const override { return 3u; }

	void Load(const rapidjson::Value& v)              override;
	void Save(JsonWriter& w)                          override;
	void RenderMenu()                                 override;
	void OnCreateMove(CCitadelInput*, CUserCmd*)      override;
	void OnPostMove(CCitadelInput*)                   override;
	void OnRender()                                   override;

	// ── Settings ──────────────────────────────────────────────────────────────
	bool  SnipeActive  = false;
	float SnipeFov          = 60.f;
	bool  SnipeShowFov      = false;
	float SnipeFovColor[3]  = { 1.f, 1.f, 1.f };
	float SnipeSmoothX      = 5.f;
	float SnipeSmoothY      = 5.f;

private:
	CL_AbilityCache m_Snipe { EAbilitySlots_t::ESlot_Signature_4 };

	enum class ESnipeState { Idle, Scoping };
	float                m_flNextAttackTime = 0.f;
	ESnipeState          m_eState          = ESnipeState::Idle;

	HeroAimState         m_aimState    = {};
	float                m_flFovScale  = 1.f;
	Vector3              m_vAimPoint   = {};
	C_CitadelPlayerPawn* m_pLockedPawn = nullptr;

	struct SnipePropsCache
	{
		float flBaseDamage     = 90.f;
		float flLowHealthBonus = 90.f;
		float flLowHPThresh    = 50.f;
		float flHeadshotScale  = 1.20f;
		bool  bValid           = false;
	} m_propsCache;

	struct AimPending
	{
		bool             valid       = false;
		CCitadel_Camera* pActiveCam  = nullptr;
		QAngle           qFinal      = {};
		uintptr_t        pawnAddr    = 0;
		bool             bHoldScope  = false;
		bool             bFireAttack = false;
	} m_pending;

	void ResetLock();
	void RefreshSnipeProps( CCitadel_Ability_Hornet_Snipe* pSnipe );
};
