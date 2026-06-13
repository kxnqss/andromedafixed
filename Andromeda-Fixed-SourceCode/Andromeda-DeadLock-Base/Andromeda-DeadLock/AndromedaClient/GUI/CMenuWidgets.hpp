#pragma once

#include <ImGui/imgui.h>

// Pure stateless ImGui widget helpers. No dependency on CAndromedaMenu state.
class CMenuWidgets
{
public:
	struct ColorPairDesc
	{
		const char* label;
		float* colorA;
		int uidA;
		float* colorB;
		int uidB;
	};

	// Combined color-swatch + checkbox on one row.
	struct ColorToggleDesc
	{
		const char* label;
		float* color;
		int uid;
		const char* toggleId;
		bool* value;
	};

	// Checkbox + arrow-button on one row; click arrow to open a floating popup.
	// Arrow rotates down while the popup is open.
	// Returns the bottom-right corner of the arrow button (pass to BeginAnimatedPopup).
	struct ExpandableToggleDesc
	{
		const char* label;
		const char* toggleId;
		const char* popupId;
		bool* value;
	};

	static void SectionHeader(const char* label);
	static void ColorSwatch(float* color, int uid);
	static void RenderColor(const char* label, float* color, int uid);
	static void RenderColorPair(const ColorPairDesc& desc);
	static void RenderColorToggle(const ColorToggleDesc& desc);
	static ImVec2 RenderExpandableToggle(const ExpandableToggleDesc& desc);

	// Arrow-only variant — no checkbox, just a label and an arrow button.
	// Click opens popupId. Returns bottom-right corner of the arrow button.
	static ImVec2 RenderExpandableLabel(const char* label, const char* popupId);

	// Replacement for the SetNextWindowPos/SizeConstraints/BgAlpha + BeginPopup block.
	// Positions the popup at anchorMax (top-right aligned), animates its height open.
	// Returns true while the popup is open; call ImGui::EndPopup() when it does.
	static bool BeginAnimatedPopup(const char* popupId, ImVec2 anchorMax, float width = 280.f);

	static void RenderKeyBind(const char* label, int& key);
	static void FormatKeyName(char* buf, int bufSz, int key);
};