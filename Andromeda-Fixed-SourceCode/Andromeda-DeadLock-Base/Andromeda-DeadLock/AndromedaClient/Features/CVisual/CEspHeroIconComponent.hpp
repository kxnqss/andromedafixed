#pragma once

#include "IEspComponent.hpp"

// ── Settings ──────────────────────────────────────────────────────────────────

struct HeroIconSettings
{
	bool  Enabled  = true;
	float Size     = 16.f;
	int   Position = static_cast<int>(EEspPosition::TopLeft);
	float OffsetX  = -19.f;
	float OffsetY  = -19.f;
};

// ── Component ─────────────────────────────────────────────────────────────────

class CEspHeroIconComponent final : public IEspComponent
{
public:
	void Load(const rapidjson::Value& v) override;
	void Save(JsonWriter& w)             override;
	void RenderEnemyMenu()               override;
	void RenderTeamMenu()                override;

	void OnRenderPlayer(const EspPlayerContext& ctx) override;

	HeroIconSettings Enemy;
	HeroIconSettings Team;

private:
	void RenderSide(HeroIconSettings& s, bool bEnemy);
};
