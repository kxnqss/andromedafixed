#pragma once

#include <array>
#include <mutex>
#include <vector>

#include <DeadLock/SDK/Math/Vector3.hpp>
#include <AndromedaClient/Features/IFeature.hpp>

#include "CEspBoxComponent.hpp"
#include "CEspBoneComponent.hpp"
#include "CEspHeroIconComponent.hpp"
#include "CEspNameComponent.hpp"
#include "CEspMinimapComponent.hpp"
#include "CEspChamsComponent.hpp"
#include "CEspSpectatorListComponent.hpp"

struct SoundData_t
{
	ULONGLONG dwTime = 0;
	Vector3 Pos;
};

// ── CVisual ───────────────────────────────────────────────────────────────────

class CVisual final : public IFeature
{
	using SoundListVecType_t = std::vector<SoundData_t>;
	using Lock_t = std::mutex;

public:
	void OnStartSound(const Vector3& Pos, int SourceEntityIndex, const char* szSoundName) override;
	void OnClientOutput() override;

	const char* GetName() const override;
	const char8_t* GetIcon() const override;
	void Load(const rapidjson::Value& doc) override;
	void Save(JsonWriter& w) override;
	void RenderMenu() override;
	void OnRender() override;

	void CalculateBoundingBoxes();

	// ── General ───────────────────────────────────────────────────────────────
	bool Active = true;

	// ── Sound Step ────────────────────────────────────────────────────────────
	bool SoundStepEsp = true;
	float SoundStepEspColor[3] = {1.f, 1.f, 0.f};

	// ── ESP Components ────────────────────────────────────────────────────────
	// To add a component: declare it here + add &m_NewComp to m_Components
	CEspBoxComponent m_Box;
	CEspBoneComponent m_Bones;
	CEspHeroIconComponent m_HeroIcon;
	CEspNameComponent m_Name;
	CEspMinimapComponent m_Minimap;
	CEspChamsComponent m_Chams;
	CEspSpectatorListComponent m_SpectatorList;

private:
	void DrawEsp();
	void DrawSoundStep();

	SoundListVecType_t m_SoundList;
	Lock_t m_SoundLock;

	constexpr static auto g_SoundShowTime = 1000;

	// Dispatch list – iterated once per frame for rendering and per-frame logic.
	// To add a component: add &m_NewComp here and change the array size.
	std::array<IEspComponent*, 7> m_Components{&m_HeroIcon, &m_Name, &m_Box, &m_Bones, &m_Chams, &m_Minimap, &m_SpectatorList};
};

CVisual* GetVisual();