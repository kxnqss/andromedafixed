#pragma once

#include "IEspComponent.hpp"

// ── Settings ──────────────────────────────────────────────────────────────────

struct NameEspSettings
{
	bool  Enabled  = true;
	float Color[3] = {1.f, 1.f, 1.f};
	float FontSize = 13.f;
	int   Position = static_cast<int>(EEspPosition::TopLeft);
	float OffsetX  = 0.f;
	float OffsetY  = -16.f;
};

// ── Component ─────────────────────────────────────────────────────────────────

class CEspNameComponent final : public IEspComponent
{
public:
	CEspNameComponent()
	{
		Team.Color[0] = 0.3f;
		Team.Color[1] = 0.7f;
		Team.Color[2] = 1.f;
	}

	void Load(const rapidjson::Value& v) override;
	void Save(JsonWriter& w)             override;
	void RenderEnemyMenu()               override;
	void RenderTeamMenu()                override;

	void OnRenderPlayer(const EspPlayerContext& ctx) override;

	NameEspSettings Enemy;
	NameEspSettings Team;

private:
	void RenderSide(NameEspSettings& s, bool bEnemy);
};
