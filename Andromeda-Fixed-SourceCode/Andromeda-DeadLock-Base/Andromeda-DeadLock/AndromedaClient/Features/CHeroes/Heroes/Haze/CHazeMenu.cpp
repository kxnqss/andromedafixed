#include "CHazeComponent.hpp"

#include <Common/Common.hpp>
#include <ImGui/imgui.h>
#include <AndromedaClient/GUI/CAndromedaMenu.hpp>
#include <AndromedaClient/GUI/CMenuWidgets.hpp>

void CHazeComponent::RenderMenu()
{
	auto* menu = GetAndromedaMenu();

	ImGui::BeginChild(XorStr("##Haze.Dagger.Block"), ImVec2(-1.f, 0.f), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);
	ImGui::TextUnformatted(XorStr("Sleep Dagger"));
	ImGui::Separator();

	menu->RenderCheckBox(XorStr("Active"), XorStr("##Haze.Dagger.Active"), DaggerActive);

	{
		const ImVec2 arrowMax = CMenuWidgets::RenderExpandableLabel(XorStr("FOV"), XorStr("##Haze.Dagger.FovPopup"));
		if (CMenuWidgets::BeginAnimatedPopup(XorStr("##Haze.Dagger.FovPopup"), arrowMax))
		{
			menu->RenderSliderFloat({XorStr("Value"), XorStr("##Haze.Dagger.Fov"), DaggerFov, 10.f, 500.f, 70.f});
			menu->RenderCheckBox(XorStr("Show"), XorStr("##Haze.Dagger.ShowFov"), DaggerShowFov);
			CMenuWidgets::RenderColor(XorStr("Color"), DaggerFovColor, 600);
			ImGui::EndPopup();
		}
	}

	{
		const ImVec2 arrowMax = CMenuWidgets::RenderExpandableLabel(XorStr("Smooth"), XorStr("##Haze.Dagger.SmoothPopup"));
		if (CMenuWidgets::BeginAnimatedPopup(XorStr("##Haze.Dagger.SmoothPopup"), arrowMax))
		{
			menu->RenderSliderFloat({XorStr("X"), XorStr("##Haze.Dagger.SmoothX"), DaggerSmoothX, 1.f, 30.f, 40.f});
			menu->RenderSliderFloat({XorStr("Y"), XorStr("##Haze.Dagger.SmoothY"), DaggerSmoothY, 1.f, 30.f, 40.f});
			ImGui::EndPopup();
		}
	}

	ImGui::EndChild();
}