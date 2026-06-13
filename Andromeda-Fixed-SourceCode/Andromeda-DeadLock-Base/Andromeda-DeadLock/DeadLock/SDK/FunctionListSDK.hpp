#pragma once

#include <Common/Common.hpp>

#include <DeadLock/SDK/CFunctionList.hpp>
#include <DeadLock/SDK/Interface/Interface.hpp>
#include <DeadLock/SDK/Interface/CMaterialSystem2.hpp>
#include <DeadLock/SDK/Math/Vector3.hpp>
#include <DeadLock/SDK/Math/QAngle.hpp>
#include <DeadLock/SDK/Types/KeyValues3.hpp>
#include <DeadLock/SDK/Update/CCitadelCamera.hpp>

class CSkeletonInstance;
class CCitadelInput;
class C_BaseEntity;
class CGameEntitySystem;
class CCitadelPlayerController;
class IGameEvent;
class CUserCmdArray;
class CUserCmd;
class C_EnvSky;
class CHitBoxSet;


DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL( void , CSkeletonInstance_CalcWorldSpaceBones , ( CSkeletonInstance* pCSkeletonInstance , unsigned int mask ) , ( CSkeletonInstance* , unsigned int ) , ( pCSkeletonInstance , mask ) );
DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL( bool , ScreenTransform , ( const Vector3& vOrigin , Vector3& vOut ) , ( const Vector3& , Vector3& ) , ( vOrigin , vOut ) );
DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL( QAngle* , CCitadelInput_GetViewAngles , ( CCitadelInput* pCCitadelInput , int32_t slot ) , ( CCitadelInput* , int32_t ) , ( pCCitadelInput , slot ) );
DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL( void* , CGameEntitySystem_GetBaseEntity , ( CGameEntitySystem* pGameEntitySystem , int iIndex ) , ( CGameEntitySystem* , int ) , ( pGameEntitySystem , iIndex ) );
DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL( CCitadelPlayerController* , CGameEntitySystem_GetLocalCitadelPlayerController , ( int iSlot ) , ( int ) , ( iSlot ) );
DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL( const char* , IGameEvent_GetName , ( IGameEvent* pIGameEvent ) , ( IGameEvent* ) , ( pIGameEvent ) );
DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL( void , GetCUserCmdTick , ( CCitadelPlayerController* pCitadelPlayerController , int32_t* pOutputTick ) , ( CCitadelPlayerController* , int32_t* ) , ( pCitadelPlayerController , pOutputTick ) );
DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL( CUserCmdArray* , GetCUserCmdArray , ( CUserCmd** ppCUserCmd , int Tick ) , ( CUserCmd** , int ) , ( ppCUserCmd , Tick ) );
DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL( CUserCmd* , GetCUserCmdBySequenceNumber , ( CCitadelPlayerController* pCitadelPlayerController , uint32_t SequenceNumber ) , ( CCitadelPlayerController* , uint32_t ) , ( pCitadelPlayerController , SequenceNumber ) );
DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL( void* , C_EnvSky_Update , ( C_EnvSky* pC_EnvSky ) , ( C_EnvSky* ) , ( pC_EnvSky ) );
DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL( int , C_BaseEntity_GetBoneIdByName , ( C_BaseEntity* pC_BaseEntity , const char* szName ) , ( C_BaseEntity* , const char* ) , ( pC_BaseEntity , szName ) );
DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL( CHitBoxSet* , C_BaseEntity_GetHitBoxSet , ( C_BaseEntity* pC_BaseEntity , uint32_t Index = 0 ) , ( C_BaseEntity* , uint32_t ) , ( pC_BaseEntity , Index ) );
DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL(bool, C_BaseEntity_ComputeHitboxBounds, (C_BaseEntity * pC_BaseEntity, Vector3 &mins, Vector3 &maxs), (C_BaseEntity *, Vector3 &, Vector3 &), (pC_BaseEntity, mins, maxs));

// CMaterialSystem2::CreateMaterial — verified in IDA (sub_18003BFB0, vtable slot 29).
// Signature: (void* /*unused=nullptr*/, CMaterial2**, const char* szName, KeyValues3*, int /*0*/, unsigned char /*1*/)
DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL( void , CMaterialSystem2_CreateMaterial , ( CMaterial2** ppOut , const char* szName , KeyValues3* pKV3 ) , ( void* , CMaterial2** , const char* , KeyValues3* , int , unsigned char ) , ( nullptr , ppOut , szName , pKV3 , 0 , 1 ) );

// tier0.dll :: KeyValues3::LoadKV3
// bool LoadKV3(KeyValues3*, CUtlString* pError, const char* pszText, const KV3ID_t&, const char* pszFilename, unsigned int nFlags)
DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL( bool , KeyValues3_LoadKV3 , ( KeyValues3* pKV3 , void* pError , const char* pszText , const KV3ID_t* pFormat , const char* pszFile , unsigned int nFlags ) , ( KeyValues3* , void* , const char* , const KV3ID_t* , const char* , unsigned int ) , ( pKV3 , pError , pszText , pFormat , pszFile , nFlags ) );

// ── Ability property computation ──────────────────────────────────────────────
// CAbilityProperty::ComputeValue  (client.dll  sub_18043C2A0)
// Returns the upgrade-adjusted base float value for a property.
//   prop        : CitadelAbilityProperty_t* from CitadelAbilityVData::FindProperty()
//   upgradeBits : C_CitadelBaseAbility::m_nUpgradeInfo()
//   sourceIdx   : value tier (0 = default; 1-3 = upgrade tiers)
DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL( float , CAbilityProperty_ComputeValue ,
	( const void* prop , int upgradeBits , int sourceIdx ) ,
	( const void* , int , int ) ,
	( prop , upgradeBits , sourceIdx ) );

// CAbilityProperty::ComputeScaled  (client.dll  sub_18082DC10)
// Returns true and writes the fully stat-scaled value into *outValue.
// Internally builds the entity stat context (tech power, items, level etc.)
// from the pawn entity identified by casterHandle.
//   casterHandle : owner pawn's raw handle int  (ability->m_hOwnerEntity().GetEntryIndex() or raw int)
//   upgradeBits  : C_CitadelBaseAbility::m_nUpgradeInfo()
//   prop         : CitadelAbilityProperty_t* from FindProperty()
//   unitStr      : unit suffix passed to the tooltip ("m" = metric, nullptr = unitless)
//   baseValue    : CAbilityProperty_ComputeValue(prop, upgradeBits, 0)
//   outValue     : receives the final computed value
//   outDelta     : receives the range delta (may be 0 for scalars)
DECLARATE_DEADLOCK_FUNCTION_SDK_FASTCALL( bool , CAbilityProperty_ComputeScaled ,
	( int casterHandle , int upgradeBits , const void* prop , const char* unitStr ,
	  float baseValue , char flag1 , char flag2 , float* outValue , float* outDelta ) ,
	( int , int , const void* , const char* , float , char , char , float* , float* ) ,
	( casterHandle , upgradeBits , prop , unitStr , baseValue , flag1 , flag2 , outValue , outDelta ) );

inline auto CGameEntitySystem_GetHighestEntityIndex( CGameEntitySystem* pGameEntitySystem , int& HighestIdx ) -> void
{
	// [D] client.dll pattern: FF 89 ? ? ? ? EB ? 48 85 F6
	// NOTE: RTTI shows field_0x20D0 for func14 but pattern-scan gives 0x20A0
	static constexpr uintptr_t k_HighestEntityIndex = 0x20A0;
	HighestIdx = *(int32_t*)( (uintptr_t)pGameEntitySystem + k_HighestEntityIndex );
}

// Returns a pointer to the g_CCitadelCameraManager singleton.
// Resolved at init time via pattern: 48 8D 3D ? ? ? ? 8B D9  (LEA RDI,[g_CCitadelCameraManager])
// SEARCH_TYPE_PTR2 resolves the RIP-relative disp32 to the absolute address of the static object.
inline auto GetCCitadelCameraManager() -> CCitadelCameraManager*
{
	return reinterpret_cast<CCitadelCameraManager*>( GetFunctionList()->g_CCitadelCameraManager.GetFunction() );
}

// ── Entity stat computation (EStatsType) ─────────────────────────────────────
// sub_180819320 — self-contained query: builds context internally from pEntity,
// reads pEntity+0x348 (mod manager), already null-checks both. Returns 0.0 when
// pEntity is null or has no mod manager — completely crash-safe.
//
// sub_180819170 — same but incorporates an attacker entity for context-sensitive
// stats (e.g. bullet damage relative to the local player's upgrades).
//
// Both functions return double; the stat float lives in the low 32 bits.
// nStat is an EStatsType cast to int (avoids circular include with CEntityData.hpp).
inline float GetEntityStat( int nStat, void* pEntity, int nTeam = 0 )
{
	using Fn = double ( __fastcall* )( int, void*, int );
	auto* fn = static_cast<Fn>( GetFunctionList()->QueryEntityStat.GetFunction() );
	if ( !fn ) return 0.f;
	const double v = fn( nStat, pEntity, nTeam );
	return *reinterpret_cast<const float*>( &v );
}

inline float GetEntityStatVs( int nStat, void* pEntity, void* pAttacker )
{
	using Fn = double ( __fastcall* )( int, void*, void* );
	auto* fn = static_cast<Fn>( GetFunctionList()->QueryEntityStatVs.GetFunction() );
	if ( !fn ) return 0.f;
	const double v = fn( nStat, pEntity, pAttacker );
	return *reinterpret_cast<const float*>( &v );
}

// ── Ability property service context ─────────────────────────────────────────
// GetAbilityServiceContext resolves to &unk_182D3C980 via SEARCH_TYPE_PTR2
// (no function call — GetFunction() is the direct data pointer).
//
//   ctx[2] = *(ctx+8)  — entity handle for sub_1807BBE50 (stat-provider builder)
//   ctx[3] = *(ctx+12) — entity handle for sub_1807A8430 (97-stat buffer filler)
//
// The game writes a valid player handle here during HUD/tooltip evaluation each
// frame.  GetEntityStat (sub_180816A70 path) can trigger sub_18079E680(ctx) which
// resets these slots to -1.  With -1 there, sub_1807A8430 exits early and leaves
// the 97-stat buffer uninitialised → NaN for any spirit-scaled property.
inline void PrepareAbilityPropertyContext( int32_t entityHandle )
{
	auto* pCtx = static_cast<int32_t*>( GetFunctionList()->GetAbilityServiceContext.GetFunction() );
	if ( !pCtx ) return;
	pCtx[ 2 ] = entityHandle;   // serviceCtx + 8  (sub_1807BBE50)
	pCtx[ 3 ] = entityHandle;   // serviceCtx + 12 (sub_1807A8430)
}

