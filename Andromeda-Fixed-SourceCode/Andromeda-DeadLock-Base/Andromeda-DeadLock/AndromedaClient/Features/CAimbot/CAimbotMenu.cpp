#include "CAimbot.hpp"
#include "CAimbotBones.hpp"

#include <AndromedaClient/GUI/CAndromedaMenu.hpp>
#include <AndromedaClient/GUI/CMenuWidgets.hpp>

// ── Helpers ───────────────────────────────────────────────────────────────────

static void SetBit(int& mask, int bit, bool value)
{
	mask = value ? mask | (1 << bit) : mask & ~(1 << bit);
}

// ── Bones combo ───────────────────────────────────────────────────────────────

// Renders the "Target Bones" combo box.
//
// Anti-Frog constraint: when antiFrog is true, the "Head-only" state
// (BonesMask == kBoneMaskHead) is forbidden — Anti-Frog would immediately
// redirect head aim to Neck anyway, making the setting nonsensical.
// Any checkbox toggle that would produce that state is visually disabled,
// and an invalid saved state is silently corrected on first draw.
static void RenderBonesCombo(int& mask, bool antiFrog)
{
	// Silently fix an invalid "head-only + Anti-Frog" state that can arrive
	// from an old config or from enabling Anti-Frog after the fact.
	if (antiFrog && IsBonesMaskOnlyHead(mask))
		mask |= kBoneMaskNeck;

	char preview[32];
	FormatBonesMaskPreview(mask, preview, sizeof(preview));

	ImGui::AlignTextToFramePadding();
	ImGui::Text(XorStr("Target Bones"));
	ImGui::SameLine(130.f);
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x);
	const bool open = ImGui::BeginCombo(XorStr("##Aimbot.Bones"), preview);

	if (!open)
		return;

	for (int i = 0; i < kBoneCount; ++i)
	{
		const AimBoneEntry& bone = k_AimBones[i];
		bool sel = (mask & bone.mask) != 0;
		const int nextMask = sel ? (mask & ~bone.mask) : (mask | bone.mask);
		const bool block = antiFrog && IsBonesMaskOnlyHead(nextMask);

		if (block)
			ImGui::BeginDisabled();

		if (ImGui::Checkbox(bone.label, &sel))
			SetBit(mask, i, sel);

		if (!block)
			continue;

		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip(XorStr("Anti-Frog enabled\nAt least one other bone must stay enabled"));
	}

	ImGui::EndCombo();
}

// ── Menu ──────────────────────────────────────────────────────────────────────

void CAimbot::RenderMenu()
{
	auto* menu = GetAndromedaMenu();

	ImGui::BeginChild(XorStr("##AimbotContent"), ImVec2(-1.f, -1.f), true);

	CMenuWidgets::SectionHeader(XorStr("General"));
	{
		const ImVec2 arrowMax = CMenuWidgets::RenderExpandableToggle({
			XorStr("Active"),
			XorStr("##Aimbot.Active"),
			XorStr("##ActivePopup"),
			&Active
		});

		if (CMenuWidgets::BeginAnimatedPopup(XorStr("##ActivePopup"), arrowMax))
		{
			CMenuWidgets::RenderKeyBind(XorStr("Keybind"), AimKey);
			ImGui::EndPopup();
		}
	}

	{
		const ImVec2 arrowMax = CMenuWidgets::RenderExpandableLabel(XorStr("FOV"), XorStr("##FovPopup"));
		if (CMenuWidgets::BeginAnimatedPopup(XorStr("##FovPopup"), arrowMax))
		{
			menu->RenderSliderFloat({XorStr("Value"), XorStr("##Aimbot.Fov"), Fov, 10.f, 500.f, 70.f});
			menu->RenderCheckBox(XorStr("Show"), XorStr("##Aimbot.ShowFov"), ShowFov);
			CMenuWidgets::RenderColor(XorStr("Color"), FovColor, 300);
			ImGui::EndPopup();
		}
	}

	{
		const ImVec2 arrowMax = CMenuWidgets::RenderExpandableLabel(XorStr("Smooth"), XorStr("##SmoothPopup"));
		if (CMenuWidgets::BeginAnimatedPopup(XorStr("##SmoothPopup"), arrowMax))
		{
			menu->RenderSliderFloat({XorStr("X"), XorStr("##Aimbot.SensX"), SensitivityX, 1.f, 30.f, 40.f});
			menu->RenderSliderFloat({XorStr("Y"), XorStr("##Aimbot.SensY"), SensitivityY, 1.f, 30.f, 40.f});
			ImGui::EndPopup();
		}
	}

	{
		const ImVec2 arrowMax = CMenuWidgets::RenderExpandableLabel(XorStr("Movement"), XorStr("##MovementPopup"));
		if (CMenuWidgets::BeginAnimatedPopup(XorStr("##MovementPopup"), arrowMax))
		{
			menu->RenderSliderFloat({XorStr("Drift"), XorStr("##Aimbot.DriftS"), DriftScale, 0.f, 2.f, 60.f});
			menu->RenderSliderFloat({XorStr("Inertia"), XorStr("##Aimbot.InertiaS"), InertiaScale, 0.f, 2.f, 60.f});
			ImGui::EndPopup();
		}
	}

	CMenuWidgets::SectionHeader(XorStr("Other"));
	{
		const ImVec2 arrowMax = CMenuWidgets::RenderExpandableToggle({
			XorStr("Anti-Frog"),
			XorStr("##Aimbot.AntiFrog"),
			XorStr("##AntiFrogPopup"),
			&AntiFrog
		});

		if (CMenuWidgets::BeginAnimatedPopup(XorStr("##AntiFrogPopup"), arrowMax))
		{
			menu->RenderSliderFloat({XorStr("HS Threshold"), XorStr("##Aimbot.HsThr"), HsThreshold, 1.f, 99.f, 130.f});
			ImGui::EndPopup();
		}

		RenderBonesCombo(BonesMask, AntiFrog);
	}

	ImGui::EndChild();
}