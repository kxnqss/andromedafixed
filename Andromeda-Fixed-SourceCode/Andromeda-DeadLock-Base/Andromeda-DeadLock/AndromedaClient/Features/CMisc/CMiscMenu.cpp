#include "CMisc.hpp"

#include <AndromedaClient/CAndromedaGUI.hpp>
#include <AndromedaClient/GUI/CAndromedaMenu.hpp>
#include <AndromedaClient/GUI/CMenuWidgets.hpp>

static const char* kMenuStyles[] = {"Indigo", "Vermillion", "Classic Steam"};

void CMisc::RenderMenu()
{
	auto* menu = GetAndromedaMenu();
	ImGui::BeginChild(XorStr("##MiscContent"), ImVec2(-1.f, -1.f), true);

	CMenuWidgets::SectionHeader(XorStr("General"));
	m_AutoReload.RenderMenu();
	m_AutoParry.RenderMenu();

	CMenuWidgets::SectionHeader(XorStr("Menu"));
	menu->RenderSliderInt({XorStr("Alpha"), XorStr("##Misc.MenuAlpha"), MenuAlpha, 100, 255, 120.f});
	if (menu->RenderComboBox({
		.label = XorStr("Style"),
		.id = XorStr("##Misc.MenuStyle"),
		.value = MenuStyle,
		.items = kMenuStyles,
		.count = IM_ARRAYSIZE(kMenuStyles),
		.leftPad = 120.f
	}))
	{
		GetAndromedaGUI()->UpdateStyle();
	}
	menu->RenderCheckBox(XorStr("Sounds"), XorStr("##Misc.MenuSounds"), MenuSounds);

	ImGui::EndChild();
}