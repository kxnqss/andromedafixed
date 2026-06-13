#pragma once

#include <ImGui/imgui.h>
#include <DeadLock/SDK/Math/Rect_t.hpp>

#include "IEspComponent.hpp"

// ── Shared ESP position helpers ───────────────────────────────────────────────
// Include this file in any component CPP that needs screen-space positioning.

inline const char* kEspPositionItems[] = {
	"Top Left", "Top Mid", "Top Right", "Bottom Left", "Bottom Mid", "Bottom Right"
};
inline constexpr int kEspPositionCount = 6;

inline ImVec2 ComputeEspPos(const Rect_t& box, int position, float offX, float offY)
{
	const float cx   = (box.x + box.w) * 0.5f;
	const ImVec2 bMin = {box.x, box.y};
	const ImVec2 bMax = {box.w, box.h};

	switch (static_cast<EEspPosition>(position))
	{
		case EEspPosition::TopMiddle:    return {cx     + offX, bMin.y + offY};
		case EEspPosition::TopRight:     return {bMax.x + offX, bMin.y + offY};
		case EEspPosition::BottomLeft:   return {bMin.x + offX, bMax.y + offY};
		case EEspPosition::BottomMiddle: return {cx     + offX, bMax.y + offY};
		case EEspPosition::BottomRight:  return {bMax.x + offX, bMax.y + offY};
		default:                         return {bMin.x + offX, bMin.y + offY}; // TopLeft
	}
}
