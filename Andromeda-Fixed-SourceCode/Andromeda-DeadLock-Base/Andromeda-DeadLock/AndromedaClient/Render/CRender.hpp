#pragma once

#include <Common/Common.hpp>
#include <ImGui/imgui.h>

#include <DeadLock/SDK/Math/Math.hpp>

struct ID3D11ShaderResourceView;

class CRender final
{
public:
	void DrawLine(const ImVec2& Start, const ImVec2& End, const ImColor& Color, float thickness = 1.f);
	void DrawBox(const ImVec2& Min, const ImVec2& Max, const ImColor& Color);
	void DrawOutlineBox(const ImVec2& Min, const ImVec2& Max, const ImColor& Color);
	void DrawCoalBox(const ImVec2& Min, const ImVec2& Max, const ImColor& Color);
	void DrawOutlineCoalBox(const ImVec2& Min, const ImVec2& Max, const ImColor& Color);
	void DrawFillBox(const ImVec2& Min, const ImVec2& Max, const ImColor& Color);
	void DrawRectFilledMultiColor(const ImVec2& Min, const ImVec2& Max, const ImU32 col_upr_left, const ImU32 col_upr_right,
	                              const ImU32 col_bot_right, const ImU32 col_bot_left);
	void DrawCircle(const ImVec2& Center, float radius, const ImColor& Color);
	void DrawCircleFilled(const ImVec2& Center, float radius, const ImColor& Color);
	void DrawCircle3D(const Vector3& Center, float radius, const ImColor& Color);
	void DrawTriangleFilled(const ImVec2& Center, const ImVec2& Pos1, const ImVec2& Pos2, const ImColor& Color);

public:
	void DrawImage(ID3D11ShaderResourceView* pD3D11ShaderResourceView, const float x, const float y, const float w, const float h);
};

CRender* GetRender();