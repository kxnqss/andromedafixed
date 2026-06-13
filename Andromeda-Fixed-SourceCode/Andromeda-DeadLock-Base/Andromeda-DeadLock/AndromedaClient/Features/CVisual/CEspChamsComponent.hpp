#pragma once

#include "IEspComponent.hpp"

// ── Settings ──────────────────────────────────────────────────────────────────

struct ChamsSettings
{
	bool  Enabled  = true;
	float Color[3] = { 1.f , 0.f , 0.8f };
};

// ── Component ─────────────────────────────────────────────────────────────────
// Hook_DrawModel resolves the owning C_CitadelPlayerPawn for every hero mesh
// via the CBodyComponent::m_pSceneNode cross-reference and selects the
// correct material based on team membership.

class CEspChamsComponent final : public IEspComponent
{
public:
	void Load( const rapidjson::Value& v ) override;
	void Save( JsonWriter& w )             override;

	void RenderEnemyMenu() override;
	void RenderTeamMenu()  override;
	void RenderOtherMenu() override {}

	ChamsSettings Enemy;
	ChamsSettings Team;
};
