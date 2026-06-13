#pragma once

#include "IEspComponent.hpp"

// ── Settings ──────────────────────────────────────────────────────────────────

struct BoxEspSettings
{
	bool  Enabled   = true;
	float Color[3]  = {1.f, 0.f, 0.f};
	bool  HealthBar = true;
	float HpFull[3] = {0.f, 1.f, 0.f};
	float HpLow[3]  = {1.f, 0.f, 0.f};
	int   HpWidth   = 4;
};

struct TrooperEspSettings
{
	bool  Enabled  = true;
	float Color[3] = {1.f, 0.8f, 0.f};
};

// ── Component ─────────────────────────────────────────────────────────────────

class CEspBoxComponent final : public IEspComponent
{
public:
	CEspBoxComponent()
	{
		Team.Color[0] = 0.f;
		Team.Color[1] = 0.5f;
		Team.Color[2] = 1.f;
	}

	void Load(const rapidjson::Value& v) override;
	void Save(JsonWriter& w)             override;
	void RenderEnemyMenu()               override;
	void RenderTeamMenu()                override;
	void RenderOtherMenu()               override;

	void OnRenderPlayer (const EspPlayerContext& ctx)                override;
	void OnRenderTrooper(C_NPC_Trooper* pTrooper, const Rect_t& box) override;

	BoxEspSettings     Enemy;
	BoxEspSettings     Team;
	TrooperEspSettings Trooper;
};
