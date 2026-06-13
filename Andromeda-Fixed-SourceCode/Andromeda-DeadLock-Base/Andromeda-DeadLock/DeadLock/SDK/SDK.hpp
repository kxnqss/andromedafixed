#pragma once

#include <Common/Common.hpp>
#include <DeadLock/SDK/Interface/CPanoramaInterface.hpp>

class CSchemaSystem;
class IVEngineToClient;
class CGameEntitySystem;
class CInputSystem;
class CUserCmd;
class CSoundOpSystem;
class CMaterialSystem2;
class CGlobalVarsBase;
class C_CitadelGameRules;

#define CLIENT_DLL				"client.dll"
#define ENGINE2_DLL				"engine2.dll"
#define NAVSYSTEM_DLL			"navsystem.dll"
#define GAMEOVERLAYRENDER64_DLL "gameoverlayrenderer64.dll"
#define SCHEMASYSTEM_DLL		"schemasystem.dll"
#define INPUTSYSTEM_DLL			"inputsystem.dll"
#define SOUNDSYSTEM_DLL			"soundsystem.dll"
#define PANORAMA_DLL			"panorama.dll"
#define PANORAMAUICLIENT_DLL	"panoramauiclient.dll"
#define SCENESYSTEM_DLL			"scenesystem.dll"
#define MATERIALSYSTEM2_DLL		"materialsystem2.dll"
#define TIER0_DLL				"tier0.dll"

namespace SDK
{
	class Interfaces
	{
	public:
		static auto SchemaSystem() -> CSchemaSystem*;
		static auto EngineToClient() -> IVEngineToClient*;
		static auto GameEntitySystem() -> CGameEntitySystem*;
		static auto InputSystem() -> CInputSystem*;
		static auto SoundOpSystem() -> CSoundOpSystem*;
		static auto MaterialSystem2() -> CMaterialSystem2*;

	private:
		static CSchemaSystem* g_pSchemaSystem;
		static IVEngineToClient* g_pEngineToClient;
		static CGameEntitySystem* g_pGameEntitySystem;
		static CInputSystem* g_pInputSystem;
		static CSoundOpSystem* g_pSoundOpSystem;
		static CMaterialSystem2* g_pMaterialSystem2;
	};

	class Pointers
	{
	public:
		static auto GetFirstCUserCmdArray() -> CUserCmd**;
		static auto GlobalVarsBase()         -> CGlobalVarsBase*;
		static auto GameRules()              -> C_CitadelGameRules*;

	private:
		static CUserCmd**           g_ppCUserCmd;
		static CGlobalVarsBase**    g_ppGlobalVarsBase;
		static C_CitadelGameRules** g_ppGameRules;
	};

	// ─── Panorama ────────────────────────────────────────────────────────────
	//
	//   panoramauiclient.dll global  →  CPanoramaUIEngine  (static .data)
	//   CPanoramaUIEngine + 0x28     →  CUIEngine*         (returned here)
	//
	// All panel and image-manager functionality lives on CUIEngine itself.
	//
	class Panorama
	{
	public:
		// Scans panoramauiclient.dll once and returns the inner CUIEngine*.
		// Result is cached; returns nullptr until the engine is initialised.
		static auto GetCUIEngine() -> CUIEngine*;

	private:
		static CUIEngine* g_pCUIEngine;
	};
}
