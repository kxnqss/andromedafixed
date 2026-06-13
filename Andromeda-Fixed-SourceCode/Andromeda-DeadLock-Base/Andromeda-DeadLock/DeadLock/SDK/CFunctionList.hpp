#pragma once

#include <vector>
#include <Common/Common.hpp>

#include <DeadLock/SDK/SDK.hpp>
#include <DeadLock/CBasePattern.hpp>

#define DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL(Ret,Function,Param,UsingParam,CallParam)\
inline Ret Function Param\
{\
	using Fn = Ret ( __fastcall* ) UsingParam;\
	Fn Original = static_cast<Fn>( GetFunctionList()->##Function##.GetFunction() );\
	return Original##CallParam##;\
}

class CFunctionList final
{
public:
	auto OnInit() -> bool;

public:
	CBasePattern CSkeletonInstance_CalcWorldSpaceBones = { VmpStr( "CSkeletonInstance::CalcWorldSpaceBones" ) , VmpStr( "48 89 4C 24 ? 55 53 56 57 41 54 41 55 41 56 41 57 B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8D 6C 24 ? 48 8B 81" ) , CLIENT_DLL , 0 , SEARCH_TYPE_NONE };
	CBasePattern ScreenTransform = { VmpStr( "ScreenTransform" ) , VmpStr( "33 C0 48 39 05 ? ? ? ? 0F 84" ) , CLIENT_DLL , 0 , SEARCH_TYPE_NONE };
	CBasePattern CCitadelInput_GetViewAngles = { VmpStr( "CCitadelInput::GetViewAngles" ) , VmpStr( "E8 ? ? ? ? EB ? 48 8B 01 48 8D 54 24" ) , CLIENT_DLL , 0 , SEARCH_TYPE_CALL };
	CBasePattern CGameEntitySystem_GetBaseEntity = { VmpStr( "CGameEntitySystem::GetBaseEntity" ) , VmpStr( "4C 8D 49 ? 81 FA ? ? ? ? 77" ) , CLIENT_DLL };
	CBasePattern CGameEntitySystem_GetLocalCitadelPlayerController = { VmpStr( "CGameEntitySystem::GetLocalCitadelPlayerController" ) , VmpStr( "E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 3B C3 0F 84" ) , CLIENT_DLL , 0 , SEARCH_TYPE_CALL };
	CBasePattern IGameEvent_GetName = { VmpStr( "IGameEvent::GetName" ) , VmpStr( "8B 41 14 0F BA E0 1E 73 05 48 8D 41 18 C3" ) , CLIENT_DLL , 0 , SEARCH_TYPE_NONE };
	CBasePattern GetCUserCmdTick = { VmpStr( "GetCUserCmdTick" ) , VmpStr( "48 83 EC ? 4C 8B 0D ? ? ? ? 4C 8B DA" ) , CLIENT_DLL , 0 , SEARCH_TYPE_NONE };
	CBasePattern GetCUserCmdArray = { VmpStr( "GetCUserCmdArray" ) , VmpStr( "48 89 4C 24 ? 41 56 41 57" ) , CLIENT_DLL , 0 , SEARCH_TYPE_NONE };
	CBasePattern GetCUserCmdBySequenceNumber = { VmpStr( "GetCUserCmdBySequenceNumber" ) , VmpStr( "40 53 48 83 EC ? 8B DA E8 ? ? ? ? 4C 8B C0" ) , CLIENT_DLL , 0 , SEARCH_TYPE_NONE };
	CBasePattern C_EnvSky_Update = { VmpStr( "C_EnvSky::Update" ) , VmpStr( "40 53 48 83 EC 30 48 8B D9 E8 ? ? ? ? 48 8B 43" ) , CLIENT_DLL , 0 , SEARCH_TYPE_NONE };
	CBasePattern C_BaseEntity_GetBoneIdByName = { VmpStr( "C_BaseEntity::GetBoneIdByName" ) , VmpStr( "40 53 48 83 EC 20 48 8B 89 ? ? ? ? 48 8B DA 48 8B 01 FF 50 ? 48 8B C8" ) , CLIENT_DLL , 0 , SEARCH_TYPE_NONE };
	CBasePattern C_BaseEntity_GetHitBoxSet = { VmpStr( "C_BaseEntity::GetHitBoxSet" ) , VmpStr( "48 89 5C 24 ? 48 89 74 24 ? 57 48 81 EC 40 01 00 00 8B DA" ) , CLIENT_DLL , 0 , SEARCH_TYPE_NONE };
	CBasePattern g_CCitadelCameraManager = { VmpStr( "CCitadelCameraManager" ) , VmpStr( "48 8D 3D ? ? ? ? 8B D9" ) , CLIENT_DLL , 0 , SEARCH_TYPE_PTR2 };
	CBasePattern C_BaseEntity_ComputeHitboxBounds = { VmpStr( "C_BaseEntity::ComputeHitboxBounds" ) , VmpStr( "48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 B8 C0 80 00 00" ) , CLIENT_DLL , 0 , SEARCH_TYPE_NONE };
	CBasePattern CMaterialSystem2_CreateMaterial = { VmpStr( "CMaterialSystem2::CreateMaterial" ) , VmpStr( "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 56 48 81 EC 10 01 00 00 48 8B 05 ? ? ? ? 48 8B F2 BA 90 06 00 00" ) , MATERIALSYSTEM2_DLL , 0 , SEARCH_TYPE_NONE };
	CBasePattern KeyValues3_LoadKV3 = { VmpStr( "KeyValues3::LoadKV3" ) , VmpStr( "48 89 5C 24 08 57 48 83 EC 70 4C 8B D1 48 C7 C0 FF FF FF FF 48 FF C0 41 80 3C 00 00 75 F6" ) , TIER0_DLL , 0 , SEARCH_TYPE_NONE };

	// CAbilityProperty::ComputeValue — computes upgrade-adjusted base value for a property.
	// sub_18043C2A0  RVA 0x43C2A0  client.dll
	// float ComputeValue(const CitadelAbilityProperty_t* prop, int upgradeBits, int sourceIdx)
	// Pattern: first 10 unique bytes of function body (no prologue — function has no standard frame setup).
	CBasePattern CAbilityProperty_ComputeValue = { VmpStr( "CAbilityProperty::ComputeValue" ) , VmpStr( "48 63 81 ? ? 00 00 4D 63 C8 4C 8B 81 ? ? 00 00 F3 42 0F 10 4C 89 ? 4D 8D 14 C0 4D 3B C2 74 ? F3 0F 10 15 ? ? ? ? F3 0F 10 1D ? ? ? ? 49 8B 08 48 8B 41 ? 85 50 ? 74 ? 8B 41 ? 83 F8 01 75 ? F3 42 0F 10 44 89 ? F3 0F 5E C2 F3 0F 58 C3 F3 0F 59 C8 EB ? 85 C0 75 ? F3 42 0F 58 4C 89 ? 49 83 C0 08 4D 3B C2 75 ? 0F 28 C1 C3" ) , CLIENT_DLL , 0 , SEARCH_TYPE_NONE };

	// CAbilityProperty::ComputeScaled — computes final stat-scaled value (includes tech power, items, etc.)
	// sub_18082DC10  RVA 0x82DC10  client.dll
	// bool ComputeScaled(int casterHandle, int upgradeBits, const CitadelAbilityProperty_t* prop,
	//                    const char* unitStr, float baseValue, char flag1, char flag2,
	//                    float* outValue, float* outDelta)
	CBasePattern CAbilityProperty_ComputeScaled = { VmpStr( "CAbilityProperty::ComputeScaled" ) , VmpStr( "48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 57 41 54 41 55 41 56 41 57 48 81 EC A0 05 00 00" ) , CLIENT_DLL , 0 , SEARCH_TYPE_NONE };

	// sub_180819320 — self-contained EStatsType query for a single entity.
	// Takes (statIdx, pEntity, nTeam) — builds the stat context internally,
	// reads pEntity+0x348 for the mod manager, already null-checks both.
	// Returns 0.0 (double) when pEntity==null or mod manager==null — never crashes.
	// double QueryEntityStat(int statIdx, void* pEntity, int nTeam)
	CBasePattern QueryEntityStat = { VmpStr( "QueryEntityStat" ) , VmpStr( "48 89 5C 24 08 48 89 74 24 18 57 48 81 EC A0 00 00 00 41 8B D8 8B F1 48 85 D2" ) , CLIENT_DLL , 0 , SEARCH_TYPE_NONE };

	// sub_180819170 — EStatsType query considering an attacker entity.
	// Takes (statIdx, pEntity, pAttacker) — like QueryEntityStat but incorporates
	// the attacker's entity handle into the stat context for context-sensitive stats.
	// double QueryEntityStatVs(int statIdx, void* pEntity, void* pAttacker)
	CBasePattern QueryEntityStatVs = { VmpStr( "QueryEntityStatVs" ) , VmpStr( "48 8B C4 55 57 41 56 48 81 EC A0 00 00 00 49 8B F8 44 8B F1 48 85 D2" ) , CLIENT_DLL , 0 , SEARCH_TYPE_NONE };

	// Global ability-property service context used by ComputeScaled.
	// sub_1807A8430 reads *(ctx+12) to resolve the "subject entity" for the 97-stat
	// buffer (v39), and sub_1807BBE50 reads *(ctx+8) for the stat-provider builder.
	//
	// Anchors on the UNIQUE one-time init block of the TLS-guarded singleton getter
	// (sub_1807A87F0).  The init block copies several source globals into the context
	// struct, registers a destructor via atexit, then does a final
	//   lea  rax, [context_global]   ← SEARCH_TYPE_PTR2 at nOffset=0x60
	//   add  rsp, 28h
	//   ret
	// Pattern uses only opcode bytes; all 4-byte RIP-relative displacements are
	// wildcarded (??) so the pattern survives code-layout changes across updates.
	// Byte count to the lea rax: 8+7+7+6+8+7+6+7+7+8+8+5+7+5 = 96 = 0x60.
	CBasePattern GetAbilityServiceContext = { VmpStr( "GetAbilityServiceContext" ) , VmpStr( "F2 0F 10 05 ?? ?? ?? ?? 48 8D 0D ?? ?? ?? ?? 0F 10 0D ?? ?? ?? ?? 8B 05 ?? ?? ?? ?? F2 0F 11 05 ?? ?? ?? ?? 0F 10 05 ?? ?? ?? ?? 89 05 ?? ?? ?? ?? 0F 11 0D ?? ?? ?? ?? 0F 11 05 ?? ?? ?? ?? F2 0F 10 05 ?? ?? ?? ?? F2 0F 11 05 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 48 83 C4 28 C3" ) , CLIENT_DLL , 0x60 , SEARCH_TYPE_PTR2 };

};

auto GetFunctionList() -> CFunctionList*;
