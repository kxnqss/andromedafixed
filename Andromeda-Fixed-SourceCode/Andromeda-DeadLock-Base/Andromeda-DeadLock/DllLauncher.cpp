#include "DllLauncher.hpp"

#include <algorithm>
#include <string>
#include <winternl.h>

#include <Common/CrashLog.hpp>
#include <Common/Helpers/StringHelper.hpp>

#include <DeadLock/CSDK_Loader.hpp>
#include <DeadLock/CHook_Loader.hpp>
#include <DeadLock/SDK/CFunctionList.hpp>

#include <AndromedaClient/CAndromedaGUI.hpp>
#include <AndromedaClient/CAndromedaClient.hpp>

#include <AndromedaClient/Features/FeatureRegistryInit.hpp>
#include <AndromedaClient/Settings/CSettingsJson.hpp>

static CDllLauncher g_CDllLauncher{};

auto CDllLauncher::OnDllMain(LPVOID lpReserved, HINSTANCE hInstace) -> void
{
#if ENABLE_MANUAL_MAP == 0 || DEVELOPER_BUILD
	char szDllDir[MAX_PATH];

	GetModuleFileNameA(hInstace, szDllDir, MAX_PATH);

	m_DllDir = szDllDir;
	m_DllDir = m_DllDir.substr(0, m_DllDir.find_last_of('\\'));
	m_DllDir += '\\';
#else
	m_DllDir = "D:\\";
#endif

#if ENABLE_MANUAL_MAP == 1
	if (lpReserved)
	{
		ManualMapParam_t* pParam = reinterpret_cast<ManualMapParam_t*>(lpReserved);

		if (pParam)
		{
			m_DllDir = pParam->m_dllDir;
			m_DllDir = m_DllDir.substr(0, m_DllDir.find_last_of('\\') + 1);
		}
	}
#endif

	m_hDllImage = hInstace;

	m_SizeofImage = GetSizeOfImageInternal();
	m_BaseOfCode = GetBaseOfCodeInternal();

	char szGameFile[MAX_PATH] = {0};
	GetModuleFileNameA(0, szGameFile, MAX_PATH);

	m_CSGODir = szGameFile;
	m_CSGODir = m_CSGODir.substr(0, m_CSGODir.find_last_of("\\/"));
	m_CSGODir += '\\';

	memset(szGameFile, 0, MAX_PATH);

	CreateThread(0, 0, StartCheatTheard, lpReserved, 0, 0);
}

auto CDllLauncher::OnDestroy() -> void
{
	if (!m_bDestroyed)
	{
		GetDevLog()->Destroy();
		GetHook_Loader()->DestroyHooks();
		GetAndromedaGUI()->OnDestroy();
		GetCrashLog()->DestroyVectorExceptionHandler();

		m_bDestroyed = true;
	}
}

auto WINAPI CDllLauncher::StartCheatTheard(LPVOID lpThreadParameter) -> DWORD
{
	GetDevLog()->Init();
	GetCrashLog()->InitVectorExceptionHandler();

	DEV_LOG("[+] StartCheatThread: %s\n", ansi_to_utf8(GetDllDir()).c_str());

	if (!GetHook_Loader()->InitalizeMH())
	{
		DEV_LOG("[error] Hook_Loader::InitalizeMH\n");
		MessageBoxA(0, "MinHook initialization (MH_Initialize) failed.\nThis is usually caused by a conflicting hook library.", "Andromeda Init Error", MB_ICONERROR);
		return 0;
	}

	if (!GetFunctionList()->OnInit())
	{
		DEV_LOG("[error] FunctionList::OnInit\n");
		char szBuf[512];
		sprintf_s(szBuf, "Function pattern scanning failed.\nOne or more IDA signatures no longer match.\n\nCheck debug.log (next to the DLL) for the failed pattern(s).\nSend me the first failed pattern name and I will fix it.");
		MessageBoxA(0, szBuf, "Andromeda Init Error", MB_ICONERROR);
		return 0;
	}

	if (!GetSDK_Loader()->LoadSDK())
	{
		DEV_LOG("[error] CSDK_Loader::LoadSDK\n");
		MessageBoxA(0, "SDK interface loading failed.\nA required game interface (SchemaSystem, EngineToClient, GameEntitySystem, InputSystem, SoundOpSystem) or a pointer could not be resolved.\nCheck debug.log for details.", "Andromeda Init Error", MB_ICONERROR);
		return 0;
	}

	if (!GetHook_Loader()->InstallSecondHook())
	{
		DEV_LOG("[error] Hook_Loader::InstallSecondHook\n");
		MessageBoxA(0, "Hook installation failed.\nOne or more hook patterns no longer match the game binary, or the DXGI fallback failed.\nThe game may have been updated.\nCheck debug.log for details.", "Andromeda Init Error", MB_ICONERROR);
		return 0;
	}

	FeatureRegistryInit::Init();

	GetSettingsJson()->UpdateConfigList();

	{
		const auto& configs = GetSettingsJson()->GetConfigList();
		if (!configs.empty())
		{
			const std::string last = GetSettingsJson()->GetLastConfig();
			const bool found = !last.empty() && std::find(configs.begin(), configs.end(), last) != configs.end();

			GetSettingsJson()->LoadConfig(found ? last : configs[0]);
		}
	}

	while (!GetAndromedaGUI()->IsInited())
		Sleep(100);

	GetAndromedaClient()->OnInit();

	return 0;
}

auto GetDllDir() -> std::string&
{
	return GetDllLauncher()->m_DllDir;
}

auto GetCSGODir() -> std::string
{
	return GetDllLauncher()->m_CSGODir;
}

auto GetDllLauncher() -> CDllLauncher*
{
	return &g_CDllLauncher;
}
