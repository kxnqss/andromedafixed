#pragma once

#include <map>
#include <string>

#include <Common/Common.hpp>
#include <ImGui/imgui.h>

class CAndromedaMenu final
{
public:
	// ── Widget descriptor types ───────────────────────────────────────────────

	struct SliderIntDesc
	{
		const char* label;
		const char* id;
		int& value;
		int min;
		int max;
		float leftPad = 0.f;
	};

	struct SliderFloatDesc
	{
		const char* label;
		const char* id;
		float& value;
		float min;
		float max;
		float leftPad = 0.f;
	};

	struct ComboDesc
	{
		const char* label;
		const char* id;
		int& value;
		const char** items;
		int count;
		float leftPad = 0.f;
	};

	struct InputTextDesc
	{
		const char* label;
		const char* id;
		char* buffer;
		int bufSize;
		float leftPad = 0.f;
		float maxWidth = -1.f;
	};

	// ── Public interface ──────────────────────────────────────────────────────

	void OnRenderMenu();

	void SetConfigSelected(uint32_t index)
	{
		m_nConfigSelected = index;
	}

	// Widget helpers — public so that IFeature::RenderMenu() implementations
	// can call them via GetAndromedaMenu()->RenderCheckBox(...) etc.
	bool RenderCheckBox(const char* label, const char* id, bool& value);
	bool RenderComboBox(const ComboDesc& desc);
	bool RenderInputText(const InputTextDesc& desc);
	void RenderSliderInt(const SliderIntDesc& desc);
	void RenderSliderFloat(const SliderFloatDesc& desc);
	bool ButtonIcon(const char8_t* szIcon, const char* szText, ImVec2 size);

private:
	void RenderTabButton(const char8_t* szIcon, const char* szLabel, int nTab);
	void RenderTabConfig();

	void RenderRandomBorderColor();

	static void PlayClick();
	static void PlayHover();

	void InternalPlaySoundHoveredItem(const std::string& itemID)
	{
		if (ImGui::IsItemHovered())
		{
			if (!m_HoveredUIItems[itemID])
			{
				PlayHover();
				m_HoveredUIItems[itemID] = true;
			}
		}
		else
		{
			m_HoveredUIItems[itemID] = false;
		}
	}

	// ── Layout constants ──────────────────────────────────────────────────────

	static constexpr float g_MainWindowW = 720.f;
	static constexpr float g_MainWindowH = 600.f;
	static constexpr float g_TabBarW = 95.f;
	static constexpr float g_ButtonSizeY = 36.f;

	// ── State ─────────────────────────────────────────────────────────────────

	float m_flMainMenuColorStepH = 0.0f;
	float m_flMainMenuColorStepS = 0.5f;
	int m_nActiveTab = 0;
	uint32_t m_nConfigSelected = 0;

	char m_szNewConfigFileName[32] = {};
	ImColor m_MainMenuBorderColor;

	std::map<std::string, bool> m_HoveredUIItems;
};

CAndromedaMenu* GetAndromedaMenu();