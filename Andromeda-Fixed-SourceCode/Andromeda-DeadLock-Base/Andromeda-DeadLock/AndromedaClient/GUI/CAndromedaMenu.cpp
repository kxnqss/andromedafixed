#include "CAndromedaMenu.hpp"
#include "CMenuWidgets.hpp"

#include <algorithm>
#include <cmath>
#include <string>

#include <AndromedaClient/CAndromedaGUI.hpp>
#include <AndromedaClient/Fonts/FontAwesomeIcon.hpp>
#include <AndromedaClient/Helpers/CPlayUISound.hpp>
#include <AndromedaClient/Features/FeatureRegistry.hpp>
#include <AndromedaClient/Features/CMisc/CMisc.hpp>
#include <AndromedaClient/Settings/CSettingsJson.hpp>

static CAndromedaMenu g_CAndromedaMenu{};

// ── CAndromedaMenu ────────────────────────────────────────────────────────────

bool CAndromedaMenu::ButtonIcon(const char8_t* szIcon, const char* szText, ImVec2 size)
{
	ImGui::PushFont(GetAndromedaGUI()->m_pFontAwesomeIcons);
	ImGui::Text(XorStr("%s"), szIcon);
	ImGui::PopFont();
	ImGui::SameLine(40.f);

	const bool ret = ImGui::Button(szText, size);
	InternalPlaySoundHoveredItem(szText);
	return ret;
}

bool CAndromedaMenu::RenderInputText(const InputTextDesc& desc)
{
	const float titleW = ImGui::CalcTextSize(desc.label).x + 10.f;

	if (desc.label)
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text(desc.label);
		ImGui::SameLine(desc.leftPad <= 0.f ? titleW : desc.leftPad);
	}

	ImGui::PushItemWidth(desc.maxWidth != -1.f ? desc.maxWidth - titleW : -1.f);
	const bool ret = ImGui::InputText(desc.id, desc.buffer, desc.bufSize, 0);
	InternalPlaySoundHoveredItem(desc.id);
	ImGui::PopItemWidth();

	return ret;
}

bool CAndromedaMenu::RenderCheckBox(const char* label, const char* id, bool& value)
{
	if (label)
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text(label);
		ImGui::SameLine(ImGui::CalcTextSize(label).x + 10.f);
	}

	ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - ImGui::GetFrameHeight() - ImGui::GetStyle().FramePadding.x, 0.f));
	ImGui::SameLine(0.f, 0.f);

	const bool ret = ImGui::Checkbox(id, &value);
	InternalPlaySoundHoveredItem(id);
	return ret;
}

bool CAndromedaMenu::RenderComboBox(const ComboDesc& desc)
{
	if (desc.label)
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text(desc.label);
		ImGui::SameLine(desc.leftPad <= 0.f ? ImGui::CalcTextSize(desc.label).x + 10.f : desc.leftPad);
	}

	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x);
	const bool ret = ImGui::Combo(desc.id, &desc.value, desc.items, desc.count);
	InternalPlaySoundHoveredItem(desc.id);
	ImGui::PopItemWidth();
	return ret;
}

void CAndromedaMenu::RenderSliderInt(const SliderIntDesc& desc)
{
	if (desc.label)
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text(desc.label);
		ImGui::SameLine(desc.leftPad <= 0.f ? ImGui::CalcTextSize(desc.label).x + 10.f : desc.leftPad);
	}

	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x);
	ImGui::SliderInt(desc.id, &desc.value, desc.min, desc.max, "%d", ImGuiSliderFlags_AlwaysClamp);
	InternalPlaySoundHoveredItem(desc.id);
	ImGui::PopItemWidth();
}

void CAndromedaMenu::RenderSliderFloat(const SliderFloatDesc& desc)
{
	if (desc.label)
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text(desc.label);
		ImGui::SameLine(desc.leftPad <= 0.f ? ImGui::CalcTextSize(desc.label).x + 10.f : desc.leftPad);
	}

	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x);
	ImGui::SliderFloat(desc.id, &desc.value, desc.min, desc.max, "%.1f", ImGuiSliderFlags_AlwaysClamp);
	desc.value = std::round(desc.value * 10.0f) / 10.0f;
	InternalPlaySoundHoveredItem(desc.id);
	ImGui::PopItemWidth();
}

void CAndromedaMenu::RenderRandomBorderColor()
{
	const ImVec2 winPos = ImGui::GetWindowPos() - ImVec2(1.f, 1.f);
	const ImVec2 winSize = ImGui::GetWindowSize() + ImVec2(2.f, 2.f);

	m_flMainMenuColorStepH = m_flMainMenuColorStepH > 1.f ? 0.f : m_flMainMenuColorStepH + 0.01f;
	m_flMainMenuColorStepS = m_flMainMenuColorStepS > 1.f ? 0.f : m_flMainMenuColorStepS + 0.01f;
	m_MainMenuBorderColor.SetHSV(m_flMainMenuColorStepH, m_flMainMenuColorStepS, 1.f);

	ImGui::GetBackgroundDrawList()->AddRect(winPos, winPos + winSize, m_MainMenuBorderColor, 1.f, 0, 1.f);
}

void CAndromedaMenu::PlayClick()
{
	if (GetMisc()->MenuSounds)
		GetPlayUISound()->PlayUISound(UISound::UI_Click);
}

void CAndromedaMenu::PlayHover()
{
	if (GetMisc()->MenuSounds)
		GetPlayUISound()->PlayUISound(UISound::UI_Hover);
}

void CAndromedaMenu::OnRenderMenu()
{
	const auto& rootFeatures = FeatureRegistry::GetRootFeatures();
	const int kFeatureCount = static_cast<int>(rootFeatures.size());
	const int kConfigTabIdx = kFeatureCount;
	const int kTotalTabs = kFeatureCount + 1;

	m_nActiveTab = std::clamp(m_nActiveTab, 0, kTotalTabs - 1);

	constexpr ImGuiWindowFlags kWinFlags =
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

	const float menuAlpha = static_cast<float>(GetMisc()->MenuAlpha) / 255.f;
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, menuAlpha);
	ImGui::SetNextWindowSize(ImVec2(g_MainWindowW, g_MainWindowH), ImGuiCond_Always);

	if (ImGui::Begin(XorStr(CHEAT_NAME), nullptr, kWinFlags))
	{
		RenderRandomBorderColor();

		// ── Tab bar (left panel) ───────────────────────────────────────────────
		ImGui::BeginChild(XorStr("##TabBar"), ImVec2(g_TabBarW, -1.f), true);

		for (int i = 0; i < kFeatureCount; ++i)
		{
			const std::string label = std::string(rootFeatures[i]->GetName()) + "##T" + std::to_string(i);
			RenderTabButton(rootFeatures[i]->GetIcon(), label.c_str(), i);
		}

		RenderTabButton(ICON_FA_FILE_ALT, XorStr("Config##TConfig"), kConfigTabIdx);

		ImGui::EndChild();
		ImGui::SameLine();

		// ── Content (right panel) ─────────────────────────────────────────────
		ImGui::BeginChild(XorStr("##MainContent"), ImVec2(-1.f, -1.f), false);

		if (m_nActiveTab < kFeatureCount)
			rootFeatures[m_nActiveTab]->RenderMenu();
		else
			RenderTabConfig();

		ImGui::EndChild();
	}

	ImGui::End();
	ImGui::PopStyleVar();

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		PlayClick();
}

void CAndromedaMenu::RenderTabButton(const char8_t* szIcon, const char* szLabel, int nTab)
{
	const bool bActive = (m_nActiveTab == nTab);
	const float btnW = ImGui::GetContentRegionAvail().x;
	const ImVec2 btnPos = ImGui::GetCursorScreenPos();

	const std::string btnId = std::string("##tabBtn") + std::to_string(nTab);
	ImGui::InvisibleButton(btnId.c_str(), ImVec2(btnW, g_ButtonSizeY));

	const bool bHovered = ImGui::IsItemHovered();
	if (ImGui::IsItemClicked())
		m_nActiveTab = nTab;

	InternalPlaySoundHoveredItem(szLabel);

	const ImGuiCol bgIdx = bActive ? ImGuiCol_ButtonActive : bHovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button;
	const ImU32 bgCol = ImGui::GetColorU32(bgIdx);
	const ImU32 textCol = ImGui::GetColorU32(ImGuiCol_Text);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	dl->AddRectFilled(btnPos, ImVec2(btnPos.x + btnW, btnPos.y + g_ButtonSizeY), bgCol, 4.f);

	ImFont* pIconFont = GetAndromedaGUI()->m_pFontAwesomeIcons;
	ImGui::PushFont(pIconFont);
	const ImVec2 iconSz = ImGui::CalcTextSize(reinterpret_cast<const char*>(szIcon));
	ImGui::PopFont();

	const char* szTextEnd = strstr(szLabel, XorStr("##"));
	const ImVec2 textSz = ImGui::CalcTextSize(szLabel, szTextEnd);
	const float totalW = iconSz.x + 6.f + textSz.x;
	float curX = btnPos.x + (btnW - totalW) * 0.5f;

	dl->AddText(pIconFont, pIconFont->FontSize,
	            ImVec2(curX, btnPos.y + (g_ButtonSizeY - iconSz.y) * 0.5f),
	            textCol, reinterpret_cast<const char*>(szIcon));

	curX += iconSz.x + 6.f;

	dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
	            ImVec2(curX, btnPos.y + (g_ButtonSizeY - textSz.y) * 0.5f),
	            textCol, szLabel, szTextEnd);
}

void CAndromedaMenu::RenderTabConfig()
{
	const auto& configList = GetSettingsJson()->GetConfigList();
	const auto loadedIdx = GetSettingsJson()->GetConfigLoadedIndex();
	const bool bAnyConfig = !configList.empty();

	// ── Config list ───────────────────────────────────────────────────────────
	ImGui::BeginChild(XorStr("##ConfigList"), ImVec2(240.f, -1.f), true);
	for (auto idx = 0u; idx < configList.size(); ++idx)
	{
		const bool bLoaded = (loadedIdx == idx);
		const bool bSelected = (m_nConfigSelected == idx);

		std::string label = configList[idx];
		auto color = ImColor(255, 255, 255);
		if (bLoaded && bSelected)
		{
			label += XorStr(" Loaded+");
			color = ImColor(0, 255, 0);
		}
		else if (bLoaded)
		{
			label += XorStr(" Loaded");
			color = ImColor(0, 255, 0);
		}
		else if (bSelected)
		{
			label += XorStr(" Selected");
			color = ImColor(255, 140, 0);
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_Text, color.operator ImVec4());
		if (ImGui::Button(label.c_str(), ImVec2(-1.f, 0.f)))
			m_nConfigSelected = idx;

		InternalPlaySoundHoveredItem(XorStr("Config##") + std::to_string(idx));
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}
	ImGui::EndChild();
	ImGui::SameLine();

	// ── Actions ───────────────────────────────────────────────────────────────
	ImGui::BeginChild(XorStr("##ConfigActions"), ImVec2(-1.f, -1.f), true);
	RenderInputText({XorStr("Name:"), XorStr("##NewConfigFileName"), m_szNewConfigFileName, 32});

	if (ButtonIcon(ICON_FA_FILE_ALT, XorStr("Create & Save##CreateAndSaveNewConfig"), ImVec2(-1.f, g_ButtonSizeY)))
	{
		const std::string name = m_szNewConfigFileName;
		if (!name.empty())
		{
			GetSettingsJson()->SaveConfig(name + XorStr(".json"));
			GetSettingsJson()->UpdateConfigList();
		}
	}

	if (ButtonIcon(ICON_FA_DOWNLOAD, XorStr("Load Selected##LoadSelectedConfig"), ImVec2(-1.f, g_ButtonSizeY)) && bAnyConfig)
		GetSettingsJson()->LoadConfig(configList[m_nConfigSelected]);

	if (ButtonIcon(ICON_FA_SAVE, XorStr("Save Selected##SaveSelectedConfig"), ImVec2(-1.f, g_ButtonSizeY)) && bAnyConfig)
		GetSettingsJson()->SaveConfig(configList[m_nConfigSelected]);

	if (ButtonIcon(ICON_FA_CUT, XorStr("Delete Selected##DeleteSelectedConfig"), ImVec2(-1.f, g_ButtonSizeY)) && bAnyConfig)
	{
		GetSettingsJson()->DeleteConfig(configList[m_nConfigSelected]);
		GetSettingsJson()->UpdateConfigList();
	}

	if (ButtonIcon(ICON_FA_UNDO, XorStr("Refresh List##RefreshConfigList"), ImVec2(-1.f, g_ButtonSizeY)))
		GetSettingsJson()->UpdateConfigList();

	ImGui::Separator();
	ImGui::Text(XorStr("Version: %s"), CHEAT_VERSION);
	ImGui::Text(XorStr("Build: %s %s"), __DATE__, __TIME__);

	ImGui::EndChild();
}

CAndromedaMenu* GetAndromedaMenu()
{
	return &g_CAndromedaMenu;
}