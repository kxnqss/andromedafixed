#include "CRenderStackSystem.hpp"
#include "CRender.hpp"

static CRenderStackSystem g_CRenderStackSystem{};

void CRenderObjectDrawLine::OnRender()
{
	GetRender()->DrawLine(m_Start, m_End, m_Color, m_Thickness);
}

void CRenderObjectDrawBox::OnRender()
{
	GetRender()->DrawBox(m_Min, m_Max, m_Color);
}

void CRenderObjectDrawOutlineBox::OnRender()
{
	GetRender()->DrawOutlineBox(m_Min, m_Max, m_Color);
}

void CRenderObjectDrawCoalBox::OnRender()
{
	GetRender()->DrawCoalBox(m_Min, m_Max, m_Color);
}

void CRenderObjectDrawOutlineCoalBox::OnRender()
{
	GetRender()->DrawOutlineCoalBox(m_Min, m_Max, m_Color);
}

void CRenderObjectDrawFillBox::OnRender()
{
	GetRender()->DrawFillBox(m_Min, m_Max, m_Color);
}

void CRenderObjectDrawRectFilledMultiColor::OnRender()
{
	GetRender()->DrawRectFilledMultiColor(m_Min, m_Max, m_Col_upr_left, m_Col_upr_right, m_Col_bot_right, m_Col_bot_left);
}

void CRenderObjectDrawCircle::OnRender()
{
	GetRender()->DrawCircle(m_Center, m_flRadius, m_Color);
}

void CRenderObjectDrawCircleFilled::OnRender()
{
	GetRender()->DrawCircleFilled(m_Center, m_flRadius, m_Color);
}

void CRenderObjectDrawCircle3D::OnRender()
{
	GetRender()->DrawCircle3D(m_Center, m_flRadius, m_Color);
}

void CRenderObjectDrawTriangleFilled::OnRender()
{
	GetRender()->DrawTriangleFilled(m_Center, m_pos1, m_pos2, m_Color);
}

void CRenderObjectDrawString::OnRender()
{
	if (m_OverrideSize > 0.f)
	{
		m_pFont->DrawStringSize(m_x, m_y, m_Color, m_Flags, m_OverrideSize, "%s", m_Text.c_str());
		return;
	}

	m_pFont->DrawString(m_x, m_y, m_Color, m_Flags, "%s", m_Text.c_str());
}

void CRenderObjectDrawImage::OnRender()
{
	GetRender()->DrawImage(m_pSRV, m_Pos.x, m_Pos.y, m_Size.x, m_Size.y);
}

void CRenderStackSystem::DrawLine(const ImVec2& Start, const ImVec2& End, const ImColor& Color, const float thickness)
{
	std::scoped_lock lock(m_Lock);
	m_vecUpdateBuffer.emplace_back(std::make_shared<CRenderObjectDrawLine>(Start, End, Color, thickness));
}

void CRenderStackSystem::DrawBox(const ImVec2& Min, const ImVec2& Max, const ImColor& Color)
{
	std::scoped_lock lock(m_Lock);
	m_vecUpdateBuffer.emplace_back(std::make_shared<CRenderObjectDrawBox>(Min, Max, Color));
}

void CRenderStackSystem::DrawOutlineBox(const ImVec2& Min, const ImVec2& Max, const ImColor& Color)
{
	std::scoped_lock lock(m_Lock);
	m_vecUpdateBuffer.emplace_back(std::make_shared<CRenderObjectDrawOutlineBox>(Min, Max, Color));
}

void CRenderStackSystem::DrawCoalBox(const ImVec2& Min, const ImVec2& Max, const ImColor& Color)
{
	std::scoped_lock lock(m_Lock);
	m_vecUpdateBuffer.push_back(std::make_shared<CRenderObjectDrawCoalBox>(Min, Max, Color));
}

void CRenderStackSystem::DrawOutlineCoalBox(const ImVec2& Min, const ImVec2& Max, const ImColor& Color)
{
	std::scoped_lock lock(m_Lock);
	m_vecUpdateBuffer.emplace_back(std::make_shared<CRenderObjectDrawOutlineCoalBox>(Min, Max, Color));
}

void CRenderStackSystem::DrawFillBox(const ImVec2& Min, const ImVec2& Max, const ImColor& Color)
{
	std::scoped_lock lock(m_Lock);
	m_vecUpdateBuffer.emplace_back(std::make_shared<CRenderObjectDrawFillBox>(Min, Max, Color));
}

void CRenderStackSystem::DrawRectFilledMultiColor(const ImVec2& Min, const ImVec2& Max, const ImU32 col_upr_left, const ImU32 col_upr_right,
                                                  const ImU32 col_bot_right, const ImU32 col_bot_left)
{
	std::scoped_lock lock(m_Lock);
	m_vecUpdateBuffer.emplace_back(
		std::make_shared<CRenderObjectDrawRectFilledMultiColor>(Min, Max, col_upr_left, col_upr_right, col_bot_right, col_bot_left));
}

void CRenderStackSystem::DrawCircle(const ImVec2& Center, float radius, const ImColor& Color)
{
	std::scoped_lock lock(m_Lock);
	m_vecUpdateBuffer.emplace_back(std::make_shared<CRenderObjectDrawCircle>(Center, radius, Color));
}

void CRenderStackSystem::DrawCircleFilled(const ImVec2& Center, const float radius, const ImColor& Color)
{
	std::scoped_lock lock(m_Lock);
	m_vecUpdateBuffer.emplace_back(std::make_shared<CRenderObjectDrawCircleFilled>(Center, radius, Color));
}

void CRenderStackSystem::DrawCircle3D(const Vector3& Center, float radius, const ImColor& Color)
{
	std::scoped_lock lock(m_Lock);
	m_vecUpdateBuffer.emplace_back(std::make_shared<CRenderObjectDrawCircle3D>(Center, radius, Color));
}

void CRenderStackSystem::DrawTriangleFilled(const ImVec2& Center, const ImVec2& pos1, const ImVec2& pos2, const ImColor& Color)
{
	std::scoped_lock lock(m_Lock);
	m_vecUpdateBuffer.emplace_back(std::make_shared<CRenderObjectDrawTriangleFilled>(Center, pos1, pos2, Color));
}

void CRenderStackSystem::DrawStringSize(CFont* pFont, const int X, const int Y, const int Flags, const ImColor& Color, float size,
                                        const char* fmt, ...)
{
	std::scoped_lock lock(m_Lock);
	char Buffer[g_BufferSize] = {};

	va_list va_alist;
	va_start(va_alist, fmt);
	vsnprintf(Buffer, sizeof(Buffer), fmt, va_alist);
	va_end(va_alist);

	m_vecUpdateBuffer.emplace_back(std::make_shared<CRenderObjectDrawString>(pFont, X, Y, Flags, Color, Buffer, size));
}

// TODO: SURIK std::format - kruto, ETO HUETA EBANAYA
void CRenderStackSystem::DrawImage(ID3D11ShaderResourceView* pSRV, const ImVec2& pos, const ImVec2& size)
{
	if (!pSRV)
		return;

	std::scoped_lock lock(m_Lock);
	m_vecUpdateBuffer.emplace_back(std::make_shared<CRenderObjectDrawImage>(pSRV, pos, size));
}

void CRenderStackSystem::DrawString(CFont* pFont, const int X, const int Y, const int Flags, const ImColor& Color, const char* fmt, ...)
{
	std::scoped_lock lock(m_Lock);

	char Buffer[g_BufferSize] = {0};

	va_list va_alist;
	va_start(va_alist, fmt);
	vsnprintf(Buffer, sizeof(Buffer), fmt, va_alist);
	va_end(va_alist);

	m_vecUpdateBuffer.emplace_back(std::make_shared<CRenderObjectDrawString>(pFont, X, Y, Flags, Color, Buffer));
}

void CRenderStackSystem::OnClientOutput()
{
	std::scoped_lock lock(m_Lock);

	if (m_vecUpdateBuffer.empty())
	{
		// Nothing has been drawn on this frame
		m_pSharedBuffer = std::make_shared<RenderObjectsVec_t>();
		m_bSharedBufferReady = true;
		return;
	}

	m_pSharedBuffer = std::make_shared<RenderObjectsVec_t>(std::move(m_vecUpdateBuffer));
	m_bSharedBufferReady = true;
}

void CRenderStackSystem::OnRenderStack()
{
	if (m_bSharedBufferReady.exchange(false))
	{
		m_pRenderBuffer = std::move(m_pSharedBuffer.load());
	}

	if (m_pRenderBuffer)
	{
		auto& bufferCopy = *m_pRenderBuffer;

		for (auto& order : bufferCopy)
		{
			order->OnRender();
		}
	}
}

CRenderStackSystem* GetRenderStackSystem()
{
	return &g_CRenderStackSystem;
}