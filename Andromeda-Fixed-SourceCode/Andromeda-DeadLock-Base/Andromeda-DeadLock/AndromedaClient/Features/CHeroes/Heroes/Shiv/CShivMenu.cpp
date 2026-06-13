#include "CShivComponent.hpp"

#include <Common/Common.hpp>
#include <ImGui/imgui.h>
#include <AndromedaClient/GUI/CAndromedaMenu.hpp>
#include <AndromedaClient/GUI/CMenuWidgets.hpp>

void CShivComponent::RenderMenu()
{
	auto* menu = GetAndromedaMenu();

	ImGui::BeginChild(XorStr("##Shiv.Knives.Block"), ImVec2(-1.f, 0.f), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);
	ImGui::TextUnformatted(XorStr("Serrated Knives"));
	ImGui::Separator();

	menu->RenderCheckBox(XorStr("Active"), XorStr("##Shiv.Knives.Active"), KnivesActive);

	{
		const ImVec2 arrowMax = CMenuWidgets::RenderExpandableLabel(XorStr("FOV"), XorStr("##Shiv.Knives.FovPopup"));
		if (CMenuWidgets::BeginAnimatedPopup(XorStr("##Shiv.Knives.FovPopup"), arrowMax))
		{
			menu->RenderSliderFloat({XorStr("Value"), XorStr("##Shiv.Knives.Fov"), KnivesFov, 10.f, 500.f, 70.f});
			menu->RenderCheckBox(XorStr("Show"), XorStr("##Shiv.Knives.ShowFov"), KnivesShowFov);
			CMenuWidgets::RenderColor(XorStr("Color"), KnivesFovColor, 600);
			ImGui::EndPopup();
		}
	}

	{
		const ImVec2 arrowMax = CMenuWidgets::RenderExpandableLabel(XorStr("Smooth"), XorStr("##Shiv.Knives.SmoothPopup"));
		if (CMenuWidgets::BeginAnimatedPopup(XorStr("##Shiv.Knives.SmoothPopup"), arrowMax))
		{
			menu->RenderSliderFloat({XorStr("X"), XorStr("##Shiv.Knives.SmoothX"), KnivesSmoothX, 1.f, 30.f, 40.f});
			menu->RenderSliderFloat({XorStr("Y"), XorStr("##Shiv.Knives.SmoothY"), KnivesSmoothY, 1.f, 30.f, 40.f});
			ImGui::EndPopup();
		}
	}

	ImGui::EndChild();
}