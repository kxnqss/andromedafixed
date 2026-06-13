#include "SDK.hpp"

#include <DllLauncher.hpp>
#include <Common/MemoryEngine.hpp>

#include <DeadLock/SDK/Interface/Interface.hpp>
#include <DeadLock/SDK/Interface/CShemaSystemSDK.hpp>
#include <DeadLock/SDK/Interface/IEngineToClient.hpp>
#include <DeadLock/SDK/Interface/CInputSystem.hpp>
#include <DeadLock/SDK/Interface/CSoundOpSystem.hpp>
#include <DeadLock/SDK/Interface/CMaterialSystem2.hpp>
#include <DeadLock/SDK/Update/CGlobalVarsBase.hpp>
#include <DeadLock/SDK/Types/C_CitadelGameRules.hpp>

#define INCLUDE_DEADLOCK_SEARCH_FUNCTION(Interface, FuncName) \
	if (!##Interface##_Search::##FuncName##Fn.Search())       \
		bIsReady = false;

namespace SDK
{
CSchemaSystem* Interfaces::g_pSchemaSystem = nullptr;
IVEngineToClient* Interfaces::g_pEngineToClient = nullptr;
CGameEntitySystem* Interfaces::g_pGameEntitySystem = nullptr;
CInputSystem* Interfaces::g_pInputSystem = nullptr;
CSoundOpSystem* Interfaces::g_pSoundOpSystem = nullptr;
CMaterialSystem2* Interfaces::g_pMaterialSystem2 = nullptr;

CUserCmd**          Pointers::g_ppCUserCmd        = nullptr;
CGlobalVarsBase**   Pointers::g_ppGlobalVarsBase  = nullptr;
C_CitadelGameRules** Pointers::g_ppGameRules       = nullptr;

CUIEngine* Panorama::g_pCUIEngine = nullptr;

auto Interfaces::SchemaSystem() -> CSchemaSystem*
{
	if (!g_pSchemaSystem)
	{
		CreateInterfaceFn pfnFactory = CaptureFactory(SCHEMASYSTEM_DLL);
		g_pSchemaSystem = CaptureInterface<CSchemaSystem>(pfnFactory, XorStr(SCHEMA_SYSTEM_INTERFACE_VERSION));

		bool bIsReady = true;

		INCLUDE_DEADLOCK_SEARCH_FUNCTION(CSchemaSystem, GetAllTypeScope);

		if (!bIsReady)
			return nullptr;
	}

	return g_pSchemaSystem;
}

auto Interfaces::EngineToClient() -> IVEngineToClient*
{
	if (!g_pEngineToClient)
	{
		CreateInterfaceFn pfnFactory = CaptureFactory(ENGINE2_DLL);
		g_pEngineToClient = CaptureInterface<IVEngineToClient>(pfnFactory, XorStr(IVENGINE_TO_CLIENT_INTERFACE_VERSION));

		bool bIsReady = true;

		INCLUDE_DEADLOCK_SEARCH_FUNCTION(IVEngineToClient, IsInGame);
		INCLUDE_DEADLOCK_SEARCH_FUNCTION(IVEngineToClient, ExecuteClientCmd);

		if (!bIsReady)
			return nullptr;
	}

	return g_pEngineToClient;
}

auto Interfaces::GameEntitySystem() -> CGameEntitySystem*
{
	if (g_pGameEntitySystem)
		return g_pGameEntitySystem;

	auto ppGameEntitySystem = reinterpret_cast<uintptr_t>(FindPattern(CLIENT_DLL, XorStr("48 8B 0D ? ? ? ? 8B D0 E8 ? ? ? ? 44 8B 83 C0 00 00 00")));

	if (!ppGameEntitySystem)
		return nullptr;

	while (!g_pGameEntitySystem)
	{
		g_pGameEntitySystem = *GetPtrAddress<CGameEntitySystem**>(ppGameEntitySystem);
		if (!g_pGameEntitySystem)
			Sleep(500);
	}
}

auto Interfaces::InputSystem() -> CInputSystem*
{
	if (!g_pInputSystem)
	{
		CreateInterfaceFn pfnFactory = CaptureFactory(INPUTSYSTEM_DLL);
		g_pInputSystem = CaptureInterface<CInputSystem>(pfnFactory, XorStr(INPUT_SYSTEM_INTERFACE_VERSION));
	}

	return g_pInputSystem;
}

auto Interfaces::SoundOpSystem() -> CSoundOpSystem*
{
	if (!g_pSoundOpSystem)
	{
		CreateInterfaceFn pfnFactory = CaptureFactory(SOUNDSYSTEM_DLL);
		g_pSoundOpSystem = CaptureInterface<CSoundOpSystem>(pfnFactory, XorStr(INTERFACE_SOUNDOPSYSTEM));
	}

	return g_pSoundOpSystem;
}

auto Interfaces::MaterialSystem2() -> CMaterialSystem2*
{
	if (!g_pMaterialSystem2)
	{
		CreateInterfaceFn pfnFactory = CaptureFactory(XorStr(MATERIALSYSTEM2_DLL));
		if (pfnFactory)
			g_pMaterialSystem2 = CaptureInterface<CMaterialSystem2>(pfnFactory, XorStr(CMATERIALSYSTEM2_INTERFACE));
	}

	return g_pMaterialSystem2;
}

/*
00007FFA8708712 | mov rcx,qword ptr ds:[0x7FFA883A40A0]                 | CUserCmd**
00007FFA8708712 | call client.7FFA86EDA670                              | GetCUserCmdArray
00007FFA8708713 | mov rcx,r12                                           |
00007FFA8708713 | mov r13,rax                                           |
00007FFA8708713 | mov r15d,dword ptr ds:[rax+0x6318]                    | offset SequenceNumber
00007FFA8708714 | mov edx,r15d                                          |
00007FFA8708714 | call client.7FFA86EDA400                              | GetUserCmdBySequenceNumber
*/
// 48 8B ? ? ? ? ? E8 ? ? ? ? 48 8B CF 4C 8B E8 44 8B B8 ? ? ? ? 41 8B D7 E8
auto Pointers::GetFirstCUserCmdArray() -> CUserCmd**
{
	if (!g_ppCUserCmd)
	{
		auto ppCUserCmd = reinterpret_cast<uintptr_t>(FindPattern(CLIENT_DLL, XorStr("48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B CF 48 8B F0")));
		if (!ppCUserCmd)
			return nullptr;

		g_ppCUserCmd = *GetPtrAddress<CUserCmd***>(ppCUserCmd);
	}

	return g_ppCUserCmd;
}

// ─── Panorama ─────────────────────────────────────────────────────────────

static bool s_IsReadable( uintptr_t ptr ) noexcept
{
	if ( ptr < 0x10000 )
		return false;

	MEMORY_BASIC_INFORMATION mbi{};
	if ( !VirtualQuery( reinterpret_cast<LPCVOID>( ptr ), &mbi, sizeof( mbi ) ) )
		return false;

	return mbi.State == MEM_COMMIT
		&& !( mbi.Protect & PAGE_NOACCESS )
		&& !( mbi.Protect & PAGE_GUARD );
}

// Scans panoramauiclient.dll for MOV REG,[rip+disp32] patterns and validates
// the full access chain:
//   *storage          → CPanoramaUIEngine*  (vtable in panorama.dll)
//   engine   + 0x28   → CUIEngine*          (vtable in panorama.dll)
//   engine   + 0xC18  → CImageResourceManager* (vtable in panorama.dll)
// Returns the inner CUIEngine* on success, nullptr otherwise.
static CUIEngine* ScanForCUIEngine() noexcept
{
	HMODULE hUiClient = GetModuleHandleA( XorStr( PANORAMAUICLIENT_DLL ) );
	HMODULE hPanorama = GetModuleHandleA( XorStr( PANORAMA_DLL ) );
	if ( !hUiClient || !hPanorama )
		return nullptr;

	auto uiBase  = reinterpret_cast<uintptr_t>( hUiClient );
	auto panBase = reinterpret_cast<uintptr_t>( hPanorama );

	auto* panNt = reinterpret_cast<IMAGE_NT_HEADERS64*>( panBase + reinterpret_cast<IMAGE_DOS_HEADER*>( panBase )->e_lfanew );
	uintptr_t panEnd = panBase + panNt->OptionalHeader.SizeOfImage;

	auto InPanorama = [&]( uintptr_t p ) noexcept { return p >= panBase && p < panEnd; };
	auto IsRipRel   = []( uint8_t m ) noexcept    { return ( m & 0xC7 ) == 0x05; };

	auto* pNt   = reinterpret_cast<IMAGE_NT_HEADERS64*>( uiBase + reinterpret_cast<IMAGE_DOS_HEADER*>( uiBase )->e_lfanew );
	auto* pSect = IMAGE_FIRST_SECTION( pNt );

	for ( WORD s = 0; s < pNt->FileHeader.NumberOfSections; ++s, ++pSect )
	{
		if ( !( pSect->Characteristics & IMAGE_SCN_MEM_EXECUTE ) )
			continue;

		auto*    code = reinterpret_cast<const uint8_t*>( uiBase + pSect->VirtualAddress );
		uint32_t size = pSect->Misc.VirtualSize;

		for ( uint32_t i = 0; i + 6 < size; ++i )
		{
			if ( code[i] != 0x48 && code[i] != 0x4C ) continue;
			if ( code[i + 1] != 0x8B )                continue;
			if ( !IsRipRel( code[i + 2] ) )           continue;

			int32_t   disp    = *reinterpret_cast<const int32_t*>( code + i + 3 );
			uintptr_t storage = reinterpret_cast<uintptr_t>( code + i + 7 ) + disp;

			uintptr_t engine = *reinterpret_cast<const uintptr_t*>( storage );
			if ( !s_IsReadable( engine ) )                            continue;
			if ( !InPanorama( *reinterpret_cast<uintptr_t*>( engine ) ) ) continue;

			uintptr_t inner = *reinterpret_cast<uintptr_t*>( engine + 0x28 );
			if ( !s_IsReadable( inner ) )                              continue;
			if ( !InPanorama( *reinterpret_cast<uintptr_t*>( inner ) ) )  continue;

			if ( !s_IsReadable( inner + 0xC18 ) )                     continue;
			uintptr_t mgr = *reinterpret_cast<uintptr_t*>( inner + 0xC18 );
			if ( !s_IsReadable( mgr ) )                                continue;
			if ( !InPanorama( *reinterpret_cast<uintptr_t*>( mgr ) ) )    continue;

			auto* result = reinterpret_cast<CUIEngine*>( inner );
			DEV_LOG( "[panorama] CUIEngine found: panoramauiclient+0x%X  ptr=%p\n",
			         static_cast<uint32_t>( storage - uiBase ), result );
			return result;
		}
	}

	return nullptr;
}

auto Panorama::GetCUIEngine() -> CUIEngine*
{
	if ( !g_pCUIEngine )
		g_pCUIEngine = ScanForCUIEngine();

	return g_pCUIEngine;
}

// ─── GlobalVarsBase ────────────────────────────────────────────────────────
// Pattern: MOV RAX,[rip+disp32] — the RIP-relative disp resolves to the
// CGlobalVarsBase* storage in client.dll's .data section.
// Confirmed live: storage RVA = 0x2E55DF0 (client.dll base = 0x180000000 in IDA).
auto Pointers::GlobalVarsBase() -> CGlobalVarsBase*
{
	if ( !g_ppGlobalVarsBase )
	{
		auto pInsn = reinterpret_cast<uintptr_t>(
			FindPattern( CLIENT_DLL, XorStr( "48 8B 05 ? ? ? ? 44 8B B7 DC 00 00 00 8B 70 04 B8 FF FF 00 00 89 B7 DC 00 00 00 66 01 43 04" ) ) );
		if ( !pInsn )
			return nullptr;

		g_ppGlobalVarsBase = GetPtrAddress<CGlobalVarsBase**>( pInsn );
	}

	return g_ppGlobalVarsBase ? *g_ppGlobalVarsBase : nullptr;
}

// ─── GameRules ─────────────────────────────────────────────────────────────
// Pattern: first bytes of sub_1804E7200 starting at the MOV RAX,[rip+disp32]
// that loads the C_CitadelGameRules pointer.
// Confirmed live: storage RVA = 0x37228B0 (matches _globals.txt dwGameRules).
auto Pointers::GameRules() -> C_CitadelGameRules*
{
	if ( !g_ppGameRules )
	{
		auto pInsn = reinterpret_cast<uintptr_t>(
			FindPattern( CLIENT_DLL, XorStr( "48 8B 05 ? ? ? ? 48 8B F9 48 85 C0 74 ? 83 78 74 06" ) ) );
		if ( !pInsn )
			return nullptr;

		g_ppGameRules = GetPtrAddress<C_CitadelGameRules**>( pInsn );
	}

	return g_ppGameRules ? *g_ppGameRules : nullptr;
}

} // namespace SDK
