#pragma once

#include <array>

#include <AndromedaClient/Features/IFeature.hpp>

#include "CAutoParryComponent.hpp"
#include "CAutoReloadComponent.hpp"

class CMisc final : public IFeature
{
public:
	const char* GetName() const override;
	const char8_t* GetIcon() const override;

	void Load(const rapidjson::Value& doc) override;
	void Save(JsonWriter& w) override;
	void RenderMenu() override;

	void OnStartSound(const Vector3&, int, const char*) override;
	void OnPostMove(CCitadelInput*) override;

	// ── Components ────────────────────────────────────────────────────────────
	CAutoParryComponent m_AutoParry;
	CAutoReloadComponent m_AutoReload;

	// ── Menu-only settings ────────────────────────────────────────────────────
	int MenuAlpha = 200;
	int MenuStyle = 0;
	bool MenuSounds = false;

private:
	std::array<IMiscComponent*, 2> m_Components{&m_AutoParry, &m_AutoReload};
};

CMisc* GetMisc();