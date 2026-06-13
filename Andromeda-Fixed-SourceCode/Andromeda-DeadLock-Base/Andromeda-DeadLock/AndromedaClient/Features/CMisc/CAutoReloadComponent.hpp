#pragma once

#include <Common/Common.hpp>
#include "IMiscComponent.hpp"

class CAutoReloadComponent final : public IMiscComponent
{
public:
	void Load(const rapidjson::Value& v) override;
	void Save(JsonWriter& w) override;
	void RenderMenu() override;

	void OnPostMove(CCitadelInput* pInput) override;

	bool Enabled = true;
	bool PreventFail = true;

private:
	void Tick(CCitadelInput* pInput);

	bool m_bWasInReload = false;
	float m_flFireAt = 0.f;
};