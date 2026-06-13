#include "CHook_Loader.hpp"

#include <Common/MemoryEngine.hpp>
#include <MinHook/MinHook.h>
#include <d3d11.h>
#pragma comment( lib, "d3d11.lib" )

#include <DeadLock/SDK/SDK.hpp>

#include <DeadLock/Hook/Hook_Present.hpp>
#include <DeadLock/Hook/Hook_ResizeBuffers.hpp>
#include <DeadLock/Hook/Hook_CreateSwapChain.hpp>
#include <DeadLock/Hook/Hook_FireEventClientSide.hpp>
#include <DeadLock/Hook/Hook_MouseInputEnabled.hpp>
#include <DeadLock/Hook/Hook_IsRelativeMouseMode.hpp>
#include <DeadLock/Hook/Hook_OnAddEntity.hpp>
#include <DeadLock/Hook/Hook_OnRemoveEntity.hpp>
#include <DeadLock/Hook/Hook_CreateMove.hpp>
#include <DeadLock/Hook/Hook_ParseMessage.hpp>
#include <DeadLock/Hook/Hook_OnClientOutput.hpp>
#include <DeadLock/Hook/Hook_GetMatricesForView.hpp>
#include <DeadLock/Hook/Hook_DrawModel.hpp>

static CHook_Loader g_CHook_Loader{};

auto CHook_Loader::InitalizeMH() -> bool
{
	return MH_Initialize() == MH_OK;
}

auto CHook_Loader::InstallSecondHook() -> bool
{
	m_Hooks =
	{
		/*
		sub_180095150 -> PresentOverlay
		sub_18008ED80(*(_QWORD *)(v4 + 64), sub_180095150, &qword_180162258, 1, "DXGISwapChain_Present");
		These three gameoverlayrenderer64.dll patterns are marked optional (skipIfNotFound=true,
		skipError=true) because Steam updates this DLL independently from the game.
		If they fail, InstallDXGIHooksFallback() installs Present/ResizeBuffers via DXGI VTable.
		*/
		{ { XorStr( "Hook::PresentOverlay" ) , XorStr( "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC ? 41 8B E8" ) , GAMEOVERLAYRENDER64_DLL } , &Hook_Present , reinterpret_cast<LPVOID*>( &Present_o ) , true , true },
		/*
		sub_180095520 -> ResizeBuffers
		sub_18008ED80(*(_QWORD *)(v4 + 104), sub_180095520, &qword_180162260, 1, "DXGISwapChain_ResizeBuffers");
		*/
		{ { XorStr( "Hook::ResizeBuffers" ) , XorStr( "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 54 41 56 41 57 48 83 EC ? 44 8B E2" ) , GAMEOVERLAYRENDER64_DLL } , &Hook_ResizeBuffers , reinterpret_cast<LPVOID*>( &ResizeBuffers_o ) , true , true },
		/*
		sub_1800AA7B0("IWrapDXGIFactory::CreateSwapChain called\n");
		*/
		{ { XorStr( "Hook::CreateSwapChain" ) , XorStr( "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 48 8B F9 49 8B F1 48 8D ? ? ? ? ? 49 8B D8 48 8B EA E8 ? ? ? ? 48 8D ? ? ? ? ? E8 ? ? ? ? 48 8D ? ? ? ? ? E8 ? ? ? ? 48 8D ? ? ? ? ? E8 ? ? ? ? 48 8B ? ? ? ? ? 4C 8B CE 4C 8B C3 48 8B D5 48 8B CF FF D0 8B D8 85 C0 78 18 48 85 F6 74 13 48 83 3E 00 74 0D 48 8B D5 48 8B CE E8 ? ? ? ? 8B C3 48 8B 5C 24 30 48 8B 6C 24 38 48 8B 74 24 40 48 83 C4 20 5F C3 CC CC CC CC CC CC CC CC 48 83 EC 38 48 8B 01 4C 8D 44 24 40" ) , GAMEOVERLAYRENDER64_DLL } , &Hook_CreateSwapChain , reinterpret_cast<LPVOID*>( &CreateSwapChain_o ) , true , true },
		{ { XorStr( "Hook::FireEventClientSide" ) , XorStr( "40 53 41 54 41 56 48 83 EC ? 4C 8B F2" ) , CLIENT_DLL } , &Hook_FireEventClientSide , reinterpret_cast<LPVOID*>( &FireEventClientSide_o ) },
		{ { XorStr( "Hook::MouseInputEnabled" ) , XorStr( "48 83 EC 28 E8 ? ? ? ? 84 C0 0F 85" ) , CLIENT_DLL } , &Hook_MouseInputEnabled , reinterpret_cast<LPVOID*>( &MouseInputEnabled_o ) },
		{ { XorStr( "Hook::IsRelativeMouseMode" ) , XorStr( "48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 0F B6 F2" ) , INPUTSYSTEM_DLL } , &Hook_IsRelativeMouseMode , reinterpret_cast<LPVOID*>( &IsRelativeMouseMode_o ) },
		{ { XorStr( "Hook::OnAddEntity" ) , XorStr( "48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 81" ) , CLIENT_DLL } , &Hook_OnAddEntity , reinterpret_cast<LPVOID*>( &OnAddEntity_o ) },
		{ { XorStr( "Hook::OnRemoveEntity" ) , XorStr( "48 89 74 24 ? 57 48 83 EC ? 41 B9 ? ? ? ? 41 8B C0 41 23 C1 48 8B F2 41 83 F8 ? 48 8B F9 44 0F 45 C8 41 81 F9 ? ? ? ? 73 ? FF 89" ) , CLIENT_DLL } , &Hook_OnRemoveEntity , reinterpret_cast<LPVOID*>( &OnRemoveEntity_o ) },
		{ { XorStr( "Hook::CreateMove" ) , XorStr( "85 D2 0F 85 ? ? ? ? 48 8B C4 44 88 40 18" ) , CLIENT_DLL } , &Hook_CreateMove , reinterpret_cast<LPVOID*>( &CreateMove_o ) },
		{ { XorStr( "Hook::ParseMessage" ) , XorStr( "40 56 57 41 57 48 83 EC ? 4C 8B F9" ) , ENGINE2_DLL } , &Hook_ParseMessage , reinterpret_cast<LPVOID*>( &ParseMessage_o ) },
		{ { XorStr( "Hook::OnClientOutput" ) , XorStr( "48 89 5C 24 ? 55 56 57 41 54 41 56 48 83 EC ? 48 8D 05" ) , ENGINE2_DLL } , &Hook_OnClientOutput , reinterpret_cast<LPVOID*>( &OnClientOutput_o ) },
		{ { XorStr( "Hook::GetMatricesForView" ) , XorStr( "48 8B C4 48 89 68 ? 48 89 70 ? 57" ) , CLIENT_DLL } , &Hook_GetMatricesForView , reinterpret_cast<LPVOID*>( &GetMatricesForView_o ) },
		{ { XorStr( "Hook::Draw" ) , XorStr( "48 8B C4 53 57 41 54 48 81 EC D0 00 00" ) , SCENESYSTEM_DLL } , &Hook_DrawModel , reinterpret_cast<LPVOID*>( &DrawModel_o ) },
	};

	if ( !InstallHooks() )
		return false;

	// If the Steam overlay patterns failed (different Steam client version),
	// fall back to hooking IDXGISwapChain::Present / ResizeBuffers via VTable.
	if ( !Present_o )
	{
		DEV_LOG( "[!] Hook::PresentOverlay not installed — trying DXGI VTable fallback\n" );

		if ( !InstallDXGIHooksFallback() )
		{
			DEV_LOG( "[error] Hook_Loader::InstallSecondHook: DXGI fallback also failed\n" );
			return false;
		}
	}

	return true;
}

auto CHook_Loader::InstallDXGIHooksFallback() -> bool
{
	// ── 1. Create a temporary invisible window ────────────────────────────
	const char* kWndClass = XorStr( "AndromedaDXGITemp" );

	WNDCLASSEXA wc   = {};
	wc.cbSize        = sizeof( WNDCLASSEXA );
	wc.lpfnWndProc   = DefWindowProcA;
	wc.hInstance     = GetModuleHandleA( nullptr );
	wc.lpszClassName = kWndClass;
	RegisterClassExA( &wc );

	HWND hWnd = CreateWindowExA( 0 , kWndClass , XorStr( "" ) , WS_OVERLAPPEDWINDOW ,
	                              0 , 0 , 8 , 8 , nullptr , nullptr ,
	                              GetModuleHandleA( nullptr ) , nullptr );

	if ( !hWnd )
	{
		DEV_LOG( "[error] InstallDXGIHooksFallback: CreateWindowExA failed (%lu)\n" , GetLastError() );
		UnregisterClassA( kWndClass , GetModuleHandleA( nullptr ) );
		return false;
	}

	// ── 2. Create dummy D3D11 device + swap chain ─────────────────────────
	DXGI_SWAP_CHAIN_DESC scd         = {};
	scd.BufferCount                   = 1;
	scd.BufferDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferUsage                   = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.SampleDesc.Count              = 1;
	scd.SwapEffect                    = DXGI_SWAP_EFFECT_DISCARD;
	scd.Windowed                      = TRUE;
	scd.OutputWindow                  = hWnd;

	ID3D11Device*        pDevice      = nullptr;
	ID3D11DeviceContext* pContext     = nullptr;
	IDXGISwapChain*      pSwapChain   = nullptr;
	D3D_FEATURE_LEVEL    featureLevel = {};

	const HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr , D3D_DRIVER_TYPE_HARDWARE , nullptr , 0 ,
		nullptr , 0 , D3D11_SDK_VERSION ,
		&scd , &pSwapChain , &pDevice , &featureLevel , &pContext );

	DestroyWindow( hWnd );
	UnregisterClassA( kWndClass , GetModuleHandleA( nullptr ) );

	if ( FAILED( hr ) || !pSwapChain )
	{
		DEV_LOG( "[error] InstallDXGIHooksFallback: D3D11CreateDeviceAndSwapChain failed (0x%lX)\n" , hr );
		if ( pDevice )  pDevice->Release();
		if ( pContext ) pContext->Release();
		return false;
	}

	// ── 3. Extract VTable entries ─────────────────────────────────────────
	//   IDXGISwapChain VTable layout (standard DXGI):
	//   [8]  = Present
	//   [13] = ResizeBuffers
	void** pVTable     = *reinterpret_cast<void***>( pSwapChain );
	void*  pPresentFn  = pVTable[ 8 ];
	void*  pResizeFn   = pVTable[ 13 ];

	pSwapChain->Release();
	pDevice->Release();
	pContext->Release();

	// ── 4. Hook Present ───────────────────────────────────────────────────
	if ( !Present_o )
	{
		const MH_STATUS st = MH_CreateHook(
			pPresentFn ,
			reinterpret_cast<LPVOID>( &Hook_Present ) ,
			reinterpret_cast<LPVOID*>( &Present_o ) );

		if ( st != MH_OK )
		{
			DEV_LOG( "[error] InstallDXGIHooksFallback: MH_CreateHook(Present) = %d\n" , st );
			return false;
		}

		MH_EnableHook( pPresentFn );
		DEV_LOG( "[+] InstallDXGIHooksFallback: Present hooked via VTable[8] @ %p\n" , pPresentFn );
	}

	// ── 5. Hook ResizeBuffers ─────────────────────────────────────────────
	if ( !ResizeBuffers_o )
	{
		const MH_STATUS st = MH_CreateHook(
			pResizeFn ,
			reinterpret_cast<LPVOID>( &Hook_ResizeBuffers ) ,
			reinterpret_cast<LPVOID*>( &ResizeBuffers_o ) );

		if ( st != MH_OK )
		{
			DEV_LOG( "[!] InstallDXGIHooksFallback: MH_CreateHook(ResizeBuffers) = %d (non-fatal)\n" , st );
		}
		else
		{
			MH_EnableHook( pResizeFn );
			DEV_LOG( "[+] InstallDXGIHooksFallback: ResizeBuffers hooked via VTable[13] @ %p\n" , pResizeFn );
		}
	}

	return true;
}

auto CHook_Loader::InstallHooks() -> bool
{
	for ( auto& Hook : m_Hooks )
	{
		while ( !GetModuleHandleA( Hook.m_Pattern.GetDllName() ) )
			   Sleep( 1 );

		if ( !Hook.m_Pattern.Search( Hook.m_bSkipError ) )
		{
			if ( !Hook.m_bSkipError )
				DEV_LOG( "[error] Hook #1 -> '%s'\n" , Hook.m_Pattern.GetPatternName() );

			if ( !Hook.m_bSkipIfNotFound )
				return false;

			continue;
		}

		auto Status = MH_CreateHook( Hook.m_Pattern.GetFunction() , Hook.m_pDetour , Hook.m_pOriginal );
		if ( Status != MH_OK )
		{
			DEV_LOG( "[error] Hook #2 [%i] -> '%s'\n" , Status , Hook.m_Pattern.GetPatternName() );
			return false;
		}
	}

	MH_EnableHook( MH_ALL_HOOKS );

	m_Hooks.clear();
	return true;
}

auto CHook_Loader::DestroyHooks() -> void
{
	MH_DisableHook( MH_ALL_HOOKS );
	MH_Uninitialize();
}

auto GetHook_Loader() -> CHook_Loader*
{
	return &g_CHook_Loader;
}
