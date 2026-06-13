#include "CMenuWidgets.hpp"

#include <Common/Common.hpp>
#include <unordered_map>

// ── Popup animation state ─────────────────────────────────────────────────────

namespace
{
std::unordered_map<ImGuiID, float> s_PopupAnims;

float& GetPopupAnimH(const char* popupId)
{
	return s_PopupAnims[ImGui::GetID(popupId)];
}
} // namespace

// ── Key name lookup table ─────────────────────────────────────────────────────

static constexpr struct
{
	int vk;
	const char* name;
} kKeyNames[] = {
	{0x01, "LMB"}, {0x02, "RMB"}, {0x04, "MMB"}, {0x05, "X1"}, {0x06, "X2"}, {0x08, "Back"}, {0x09, "Tab"},
	{0x0D, "Enter"}, {0x10, "Shift"}, {0x11, "Ctrl"}, {0x12, "Alt"}, {0x14, "CapsLock"}, {0x1B, "Escape"}, {0x20, "Space"},
	{0x21, "PgUp"}, {0x22, "PgDn"}, {0x23, "End"}, {0x24, "Home"}, {0x25, "Left"}, {0x26, "Up"}, {0x27, "Right"},
	{0x28, "Down"}, {0x2E, "Del"}, {0x70, "F1"}, {0x71, "F2"}, {0x72, "F3"}, {0x73, "F4"}, {0x74, "F5"},
	{0x75, "F6"}, {0x76, "F7"}, {0x77, "F8"}, {0x78, "F9"}, {0x79, "F10"}, {0x7A, "F11"}, {0x7B, "F12"},
	{0xA0, "LShift"}, {0xA1, "RShift"}, {0xA2, "LCtrl"}, {0xA3, "RCtrl"}, {0xA4, "LAlt"}, {0xA5, "RAlt"},
};

// ── CMenuWidgets ──────────────────────────────────────────────────────────────

void CMenuWidgets::SectionHeader(const char* label)
{
	ImGui::Spacing();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.75f, 0.2f, 1.f));
	ImGui::Text(label);
	ImGui::PopStyleColor();
	ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.9f, 0.75f, 0.2f, 0.4f));
	ImGui::Separator();
	ImGui::PopStyleColor();
	ImGui::Spacing();
}

void CMenuWidgets::ColorSwatch(float* color, int uid)
{
	constexpr ImGuiColorEditFlags kBtnFlags = ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder;
	constexpr ImGuiColorEditFlags kPickFlags = ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoLabel;

	const float h = ImGui::GetFrameHeight();
	ImGui::PushID(uid);

	if (ImGui::ColorButton(XorStr("##c"), ImVec4(color[0], color[1], color[2], 1.f), kBtnFlags, ImVec2(h, h)))
		ImGui::OpenPopup(XorStr("##cp"));

	if (ImGui::BeginPopup(XorStr("##cp")))
	{
		ImGui::ColorPicker3(XorStr("##pk"), color, kPickFlags);
		ImGui::EndPopup();
	}

	ImGui::PopID();
}

void CMenuWidgets::RenderColor(const char* label, float* color, int uid)
{
	const float h = ImGui::GetFrameHeight();
	const float lp = ImGui::GetStyle().FramePadding.x;

	ImGui::AlignTextToFramePadding();
	ImGui::Text(XorStr("%s"), label);
	ImGui::SameLine(ImGui::CalcTextSize(label).x + 10.f);
	ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - h - lp, 0.f));
	ImGui::SameLine(0.f, 0.f);
	ColorSwatch(color, uid);
}

void CMenuWidgets::RenderColorPair(const ColorPairDesc& desc)
{
	constexpr float kGap = 3.f;
	const float h = ImGui::GetFrameHeight();
	const float lp = ImGui::GetStyle().FramePadding.x;

	ImGui::AlignTextToFramePadding();
	ImGui::Text(XorStr("%s"), desc.label);
	ImGui::SameLine(ImGui::CalcTextSize(desc.label).x + 10.f);
	ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - h * 2.f - kGap - lp, 0.f));
	ImGui::SameLine();
	ColorSwatch(desc.colorA, desc.uidA);
	ImGui::SameLine(0.f, kGap);
	ColorSwatch(desc.colorB, desc.uidB);
}

void CMenuWidgets::RenderColorToggle(const ColorToggleDesc& desc)
{
	constexpr float kGap = 3.f;
	const float h = ImGui::GetFrameHeight();
	const float lp = ImGui::GetStyle().FramePadding.x;

	ImGui::AlignTextToFramePadding();
	ImGui::Text(XorStr("%s"), desc.label);
	ImGui::SameLine(ImGui::CalcTextSize(desc.label).x + 10.f);
	ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - h * 2.f - kGap - lp, 0.f));
	ImGui::SameLine(0.f, 0.f);

	ImGui::Checkbox(desc.toggleId, desc.value);
	ImGui::SameLine(0.f, kGap);
	ColorSwatch(desc.color, desc.uid);
}

ImVec2 CMenuWidgets::RenderExpandableLabel(const char* label, const char* popupId)
{
	const float h = ImGui::GetFrameHeight();
	const float lp = ImGui::GetStyle().FramePadding.x;

	ImGui::AlignTextToFramePadding();
	ImGui::Text(XorStr("%s"), label);
	ImGui::SameLine(ImGui::CalcTextSize(label).x + 10.f);
	ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - h - lp, 0.f));
	ImGui::SameLine(0.f, 0.f);
	ImGui::PushID(popupId);
	const bool bClicked = ImGui::ArrowButton(XorStr("##arr"), ImGuiDir_Right);
	const ImVec2 arrowMax = ImGui::GetItemRectMax();
	ImGui::PopID();

	if (bClicked)
		ImGui::OpenPopup(popupId);

	return arrowMax;
}

ImVec2 CMenuWidgets::RenderExpandableToggle(const ExpandableToggleDesc& desc)
{
	constexpr float kGap = 3.f;
	const float h = ImGui::GetFrameHeight();
	const float lp = ImGui::GetStyle().FramePadding.x;

	ImGui::AlignTextToFramePadding();
	ImGui::Text(XorStr("%s"), desc.label);
	ImGui::SameLine(ImGui::CalcTextSize(desc.label).x + 10.f);
	ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - h * 2.f - kGap - lp, 0.f));
	ImGui::SameLine(0.f, 0.f);
	ImGui::Checkbox(desc.toggleId, desc.value);
	ImGui::SameLine(0.f, kGap);
	ImGui::PushID(desc.popupId);
	const bool bClicked = ImGui::ArrowButton(XorStr("##arr"), ImGuiDir_Right);
	const ImVec2 arrowMax = ImGui::GetItemRectMax();
	ImGui::PopID();

	if (bClicked)
		ImGui::OpenPopup(desc.popupId);

	return arrowMax;
}

bool CMenuWidgets::BeginAnimatedPopup(const char* popupId, ImVec2 anchorMax, float width)
{
	constexpr float kSpeed = 1300.f;
	constexpr float kFullH = 1800.f;

	if (!ImGui::IsPopupOpen(popupId))
	{
		GetPopupAnimH(popupId) = 0.f;
		return false;
	}

	float& animH = GetPopupAnimH(popupId);
	animH += ImGui::GetIO().DeltaTime * kSpeed;
	const float maxH = (animH >= kFullH) ? FLT_MAX : animH;

	ImGui::SetNextWindowPos(anchorMax, ImGuiCond_Appearing, ImVec2(1.f, 0.f));
	ImGui::SetNextWindowSizeConstraints(ImVec2(width, 0.f), ImVec2(width, maxH));
	ImGui::SetNextWindowBgAlpha(0.97f);

	return ImGui::BeginPopup(popupId);
}

void CMenuWidgets::RenderKeyBind(const char* label, int& key)
{
	static bool s_Capturing = false;
	static int* s_pTarget = nullptr;
	static bool s_SnapShot = false;
	static bool s_HeldSnap[256] = {};

	constexpr float kBtnW = 70.f;

	ImGui::AlignTextToFramePadding();
	ImGui::Text(XorStr("%s"), label);
	ImGui::SameLine(ImGui::CalcTextSize(label).x + 10.f);
	ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - kBtnW - ImGui::GetStyle().FramePadding.x, 0.f));
	ImGui::SameLine(0.f, 0.f);

	const bool bActive = (s_Capturing && s_pTarget == &key);

	char buf[32];
	if (bActive)
		snprintf(buf, sizeof(buf), XorStr("[...] "));
	else if (key == 0)
		snprintf(buf, sizeof(buf), XorStr("None"));
	else
		FormatKeyName(buf, sizeof(buf), key);

	if (ImGui::Button(buf, ImVec2(70.f, 0.f)) && !s_Capturing)
	{
		s_Capturing = true;
		s_pTarget = &key;
		s_SnapShot = false;
		memset(s_HeldSnap, 0, sizeof(s_HeldSnap));
	}

	if (!bActive)
		return;

	if (!s_SnapShot)
	{
		for (int i = 1; i < 256; ++i)
			s_HeldSnap[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
		s_SnapShot = true;
		return;
	}

	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
	{
		s_Capturing = false;
		s_pTarget = nullptr;
		return;
	}

	for (int i = 1; i < 256; ++i)
	{
		if (i == VK_ESCAPE)
			continue;
		if ((GetAsyncKeyState(i) & 0x8000) && !s_HeldSnap[i])
		{
			key = i;
			s_Capturing = false;
			s_pTarget = nullptr;
			break;
		}
	}
}

void CMenuWidgets::FormatKeyName(char* buf, int bufSz, int key)
{
	for (const auto& e : kKeyNames)
		if (e.vk == key)
		{
			snprintf(buf, bufSz, "%s", e.name);
			return;
		}

	if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9'))
		snprintf(buf, bufSz, "%c", key);
	else
		snprintf(buf, bufSz, "0x%02X", key);
}