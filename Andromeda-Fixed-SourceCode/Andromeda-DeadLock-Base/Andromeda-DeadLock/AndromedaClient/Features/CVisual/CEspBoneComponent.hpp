#pragma once

#include "IEspComponent.hpp"

// ── Settings ──────────────────────────────────────────────────────────────────

struct BoneEspSettings
{
	bool  Enabled  = true;
	float Color[3] = {1.f, 0.2f, 0.2f};
};

// ── Component ─────────────────────────────────────────────────────────────────

class CEspBoneComponent final : public IEspComponent
{
public:
	CEspBoneComponent()
	{
		Team.Color[0] = 0.2f;
		Team.Color[1] = 0.6f;
		Team.Color[2] = 1.f;
	}

	void Load(const rapidjson::Value& v) override;
	void Save(JsonWriter& w)             override;
	void RenderEnemyMenu()               override;
	void RenderTeamMenu()                override;

	void OnRenderPlayer(const EspPlayerContext& ctx) override;

	BoneEspSettings Enemy;
	BoneEspSettings Team;
};
