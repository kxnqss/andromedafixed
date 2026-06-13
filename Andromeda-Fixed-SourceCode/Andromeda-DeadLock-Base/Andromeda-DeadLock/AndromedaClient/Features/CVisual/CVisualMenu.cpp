#include "CVisual.hpp"

#include <AndromedaClient/GUI/CAndromedaMenu.hpp>
#include <AndromedaClient/GUI/CMenuWidgets.hpp>

void CVisual::RenderMenu()
{
	auto* menu = GetAndromedaMenu();

	ImGui::BeginChild(XorStr("##VisualContent"), ImVec2(-1.f, -1.f), true);

	CMenuWidgets::SectionHeader(XorStr("General"));
	menu->RenderCheckBox(XorStr("Active"), XorStr("##Visual.Active"), Active);

	ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8.f, 2.f));
	if (ImGui::BeginTable(XorStr("##EnemyTeam"), 2, ImGuiTableFlags_BordersInnerV))
	{
		int componentCount = static_cast<int>(m_Components.size());

		ImGui::TableNextColumn();
		CMenuWidgets::SectionHeader(XorStr("Enemy"));
		for (int i = 0; i < componentCount; ++i)
		{
			if (i > 0)
				ImGui::Separator();
			m_Components[i]->RenderEnemyMenu();
		}

		ImGui::TableNextColumn();
		CMenuWidgets::SectionHeader(XorStr("Team"));
		for (int i = 0; i < componentCount; ++i)
		{
			if (i > 0)
				ImGui::Separator();
			m_Components[i]->RenderTeamMenu();
		}

		ImGui::EndTable();
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	ImGui::Spacing();

	CMenuWidgets::SectionHeader(XorStr("Other"));
	{
		const ImVec2 arrowMax = CMenuWidgets::RenderExpandableToggle({
			XorStr("Sound Step"),
			XorStr("##Visual.SoundStep"),
			XorStr("##SoundStepPopup"),
			&SoundStepEsp
		});

		if (CMenuWidgets::BeginAnimatedPopup(XorStr("##SoundStepPopup"), arrowMax))
		{
			CMenuWidgets::RenderColor(XorStr("Color"), SoundStepEspColor, 0);
			ImGui::EndPopup();
		}
	}

	for (auto* comp : m_Components)
		comp->RenderOtherMenu();

	ImGui::EndChild();
}