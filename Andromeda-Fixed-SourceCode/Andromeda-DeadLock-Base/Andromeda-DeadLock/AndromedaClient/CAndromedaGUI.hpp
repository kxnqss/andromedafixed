#pragma once

#include <Common/Common.hpp>
#include <d3d11.h>

#include <ImGui/imgui.h>
#include <ImGui/Misc/freetype/imgui_freetype.h>

class IAndromedaGUI
{
public:
	virtual bool IsVisible() = 0;
	virtual bool IsInited() = 0;

public:
	virtual void OnPresent(IDXGISwapChain* pSwapChain) = 0;
	virtual void OnResizeBuffers(IDXGISwapChain* pSwapChain) = 0;
	virtual void OnRender(IDXGISwapChain* pSwapChain) = 0;
};

class CAndromedaGUI final : public IAndromedaGUI
{
public:
	void OnInit(IDXGISwapChain* pSwapChain);
	void OnDestroy();

public:
	void InitFont();

private:
	void InitIndigoStyle();
	void InitVermillionStyle();
	void InitClassicSteamStyle();

	enum EAndromedaGuiStyle
	{
		INDIGO,
		VERMILLION,
		CLASSIC_STEAM
	};

public:
	void UpdateStyle();

public:
	bool IsVisible() override
	{
		return m_bVisible;
	}

	bool IsInited() override
	{
		return m_bInit;
	}

public:
	void OnPresent(IDXGISwapChain* pSwapChain) override;
	void OnResizeBuffers(IDXGISwapChain* pSwapChain) override;
	void OnRender(IDXGISwapChain* pSwapChain) override;

public:
	void OnReopenGUI();

public:
	static LRESULT WINAPI GUI_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	ID3D11Device* GetDevice()
	{
		return m_pDevice;
	}

	ID3D11DeviceContext* GetDeviceContext()
	{
		return m_pDeviceContext;
	}

	// Actual back-buffer dimensions (may differ from the Win32 window client rect
	// when the game is launched with -w/-h args or uses internal upscaling).
	// Use this for World-to-Screen and any coordinate system that must match the
	// game's render viewport rather than the window size.
	ImVec2 GetViewportSize() const
	{
		return m_vViewportSize;
	}

public:
	void ClearRenderTargetView();

private:
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pDeviceContext = nullptr;
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;
	ID3D11RenderTargetView* m_pMainRenderTarget = nullptr;

private:
	ImGuiContext* m_pImGuiContext = nullptr;

private:
	std::string m_GuiFile;

private:
	HWND m_hDeadLockWindow = nullptr;
	WNDPROC m_WndProc_o = nullptr;

public:
	bool m_bInit = false;
	bool m_bVisible = false;
	bool m_bMainActive = false;

private:
	ImVec2 m_vecMousePosSave;
	ImVec2 m_vViewportSize{};

public:
	ImFont* m_pFontAwesomeIcons = nullptr;

private:
	struct FreeTypeBuild
	{
		enum class FontBuildMode { FreeType };

		FontBuildMode BuildMode = FontBuildMode::FreeType;

		bool WantRebuild = true;
		float RasterizerMultiply = 1.0f;
		unsigned int FreeTypeBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_MonoHinting;

		bool PreNewFrame();

		void ResetBuildFont()
		{
			WantRebuild = true;
		};
	};

	FreeTypeBuild* m_pFreeType_Font = nullptr;
};

CAndromedaGUI* GetAndromedaGUI();