#pragma once

#include "IEspComponent.hpp"
#include <string>
#include <vector>

struct SpectatorEntry_t
{
	std::string name;
	bool        isEnemy = false;
};

class CEspSpectatorListComponent final : public IEspComponent
{
public:
	void Load(const rapidjson::Value& v) override;
	void Save(JsonWriter& w)             override;
	void RenderEnemyMenu() override {}
	void RenderTeamMenu()  override {}
	void RenderOtherMenu() override;
	void OnRender()        override;

	bool Enabled = true;

private:
	std::vector<SpectatorEntry_t> GetSpectatorList();

	std::vector<SpectatorEntry_t> m_Cached;
	float                         m_flLastUpdate = 0.f;

	static constexpr float k_flUpdateInterval = 1.f;
};
