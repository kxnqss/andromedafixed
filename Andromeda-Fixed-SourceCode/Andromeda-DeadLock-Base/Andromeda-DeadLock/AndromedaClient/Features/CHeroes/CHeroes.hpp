#pragma once

#include <array>

#include <AndromedaClient/Features/IFeature.hpp>
#include <AndromedaClient/Features/CHeroes/IHeroComponent.hpp>

#include "Heroes/Hornet/CHornetComponent.hpp"
#include "Heroes/Haze/CHazeComponent.hpp"
#include "Heroes/Shiv/CShivComponent.hpp"

class CHeroes final : public IFeature
{
public:
	const char*    GetName() const override;
	const char8_t* GetIcon() const override;

	void Load(const rapidjson::Value& doc)               override;
	void Save(JsonWriter& w)                             override;
	void RenderMenu()                                    override;
	void OnCreateMove(CCitadelInput*, CUserCmd*)         override;
	void OnPostMove(CCitadelInput*)                      override;
	void OnRender()                                      override;

	// ── Components (one per hero) ─────────────────────────────────────────────
	CHornetComponent m_Hornet;
	CHazeComponent   m_Haze;
	CShivComponent   m_Shiv;

private:
	void SyncTabToCurrentHero();

	std::array<IHeroComponent*, 3> m_Components{ &m_Hornet, &m_Haze, &m_Shiv };
	int m_nActiveHero    = 0;
	int m_nLastMenuFrame = -1;
};

auto GetHeroes() -> CHeroes*;
