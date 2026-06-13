#pragma once

#include "IEspComponent.hpp"

// ── Settings ──────────────────────────────────────────────────────────────────

struct MinimapSettings
{
	bool Enabled = true;
};

// ── Component ─────────────────────────────────────────────────────────────────

class CEspMinimapComponent final : public IEspComponent
{
public:
	void Load(const rapidjson::Value& v) override;
	void Save(JsonWriter& w)             override;
	void RenderEnemyMenu() override {}
	void RenderTeamMenu()  override {}
	void RenderOtherMenu() override;
	void OnFrame()         override;

	MinimapSettings Settings;
};
