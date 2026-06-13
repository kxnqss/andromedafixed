#include "CHornetComponent.hpp"

#include <Common/Common.hpp>
#include <ImGui/imgui.h>
#include <AndromedaClient/GUI/CAndromedaMenu.hpp>
#include <AndromedaClient/GUI/CMenuWidgets.hpp>

void CHornetComponent::RenderMenu()
{
	auto* menu = GetAndromedaMenu();

	ImGui::BeginChild(XorStr("##HornetSnipeBlock"), ImVec2(-1.f, 0.f), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);
	ImGui::TextUnformatted(XorStr("Snipe"));
	ImGui::Separator();

	GetAndromedaMenu()->RenderCheckBox(XorStr("Active"), XorStr("##Hornet.Active"), SnipeActive);

	{
		const ImVec2 arrowMax = CMenuWidgets::RenderExpandableLabel(XorStr("FOV"), XorStr("##HornetFovPopup"));
		if (CMenuWidgets::BeginAnimatedPopup(XorStr("##HornetFovPopup"), arrowMax))
		{
			menu->RenderSliderFloat({XorStr("Value"), XorStr("##Hornet.Fov"), SnipeFov, 10.f, 500.f, 70.f});
			menu->RenderCheckBox(XorStr("Show"), XorStr("##Hornet.ShowFov"), SnipeShowFov);
			CMenuWidgets::RenderColor(XorStr("Color"), SnipeFovColor, 503);
			ImGui::EndPopup();
		}
	}

	{
		const ImVec2 arrowMax = CMenuWidgets::RenderExpandableLabel(XorStr("Smooth"), XorStr("##HornetSmoothPopup"));
		if (CMenuWidgets::BeginAnimatedPopup(XorStr("##HornetSmoothPopup"), arrowMax))
		{
			menu->RenderSliderFloat({XorStr("X"), XorStr("##Hornet.SmoothX"), SnipeSmoothX, 1.f, 30.f, 40.f});
			menu->RenderSliderFloat({XorStr("Y"), XorStr("##Hornet.SmoothY"), SnipeSmoothY, 1.f, 30.f, 40.f});
			ImGui::EndPopup();
		}
	}

	ImGui::EndChild();
}