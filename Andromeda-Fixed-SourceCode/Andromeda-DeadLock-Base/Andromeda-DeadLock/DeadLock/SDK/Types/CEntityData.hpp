#pragma once

#include <Common/Common.hpp>
#include <DeadLock/SDK/FunctionListSDK.hpp>

// Stat index enum from server schema dump (sdk/server/_enums.hpp)
enum class EStatsType : int32_t {
    EWeaponDPS = 0,
    EMeleeDamage_DEPRECATED = 1,
    EMaxHealth = 2,
    EClipSize = 3,
    EBaseHealthRegen = 4,
    EExternalHealthRegen = 5,
    EHealthRegen = 6,
    EMaxMoveSpeed = 7,
    ESprintSpeed = 8,
    ECrouchSpeed = 9,
    EMoveAcceleration = 10,
    EClipSizeIncrease = 11,
    EBulletArmorDamageReduction = 12,
    EBulletShieldHealth = 13,
    ETechArmorDamageReduction = 14,
    ETechShieldHealth_DEPRECATED = 15,
    ELightMeleeDamage = 16,
    EHeavyMeleeDamage = 17,
    EWeaponRange = 18,
    EWeaponRecoilReduction = 19,
    EFireRate = 20,
    EWeaponPower = 21,
    EWeaponPowerScale = 22,
    EBulletDamage = 23,
    ETechPowerAmp_DEPRECATED = 24,
    ETechPowerAmpBonus_DEPRECATED = 25,
    ERoundsPerSecond = 26,
    ERoundsPerSecondInverse = 27,
    EBaseWeaponDamageIncrease = 28,
    EBaseMeleeDamageIncrease = 29,
    EAirJumpCount = 30,
    EProcBuildUpRateScale = 31,
    ETechCooldown = 32,
    ETechCooldownBetweenChargeUses = 33,
    ETechRange = 34,
    ETechRadius = 35,
    EProjectileRadius_DEPRECATED = 36,
    EMeleeRange = 37,
    EReloadSpeed = 38,
    EMaxChargesIncrease = 39,
    EHealingOutput = 40,
    ETechDuration = 41,
    EWeaponSpreadScale = 42,
    EMeleeScalingFromWeaponPower_DEPRECATED = 43,
    EHealthAttribute_DEPRECATED = 44,
    EArmorAttribute_DEPRECATED = 45,
    EFireRateAttribute_DEPRECATED = 46,
    EWeaponPowerAttribute_DEPRECATED = 47,
    ETechDamageAttribute_DEPRECATED = 48,
    EReloadTime = 49,
    EStamina = 50,
    EStaminaCooldown_DEPRECATED = 51,
    EBuildUpRate = 52,
    EBaseWeaponDamagePerShot = 53,
    ETechLifesteal = 54,
    ETechLifestealNonHero_DEPRECATED = 55,
    EBulletLifesteal = 56,
    EDamageScale = 57,
    EChannelDuration = 58,
    ETechPower = 59,
    EArmorPower = 60,
    ETechDamageScale = 61,
    EWeaponDamageScale = 62,
    EMeleeDamageScale = 63,
    ELevelUpBaseWeaponDamageIncrease = 64,
    ELevelUpBaseMeleeDamageIncrease = 65,
    ELevelUpBaseHealthIncrease = 66,
    EStaminaRegenPerSecond = 67,
    EAbilityResourceMax = 68,
    EAbilityResourceRegenPerSecond = 69,
    ECycleTime = 70,
    EMeleeTravelDistanceScale = 71,
    EAirMoveDistanceScale = 72,
    ECritDamageReceivedScale = 73,
    EWeaponFalloffMinRange = 74,
    EWeaponFalloffMaxRange = 75,
    EBulletSpeed = 76,
    EBulletSpeedIncrease = 77,
    EStaminaRegenIncrease = 78,
    EStaminaCooldown = 79,
    EDebuffResist = 80,
    ECritDamageBonusScale = 81,
    EMeleeResist = 82,
    ELevelUpBoons = 83,
    EParryCooldown = 84,
    EHeroBulletLifestealEffectiveness = 85,
    EHeroSpiritLifestealEffectiveness = 86,
    EOOCHealthRegen = 87,
    ESlowResistance = 88,
    EStaminaRegenPercent = 89,
    EItemCooldown = 90,
    EGroundDashDistanceInMeters = 91,
    EGroundDashDuration = 92,
    EAirDashDistanceInMeters = 93,
    EAirDashDuration = 94,
    EDashSpeedInMeters = 95,
    EEnableAbilityCharges = 96,
    EStatsCount = 97,
    EStatsInvalid = 97,
};


#include "Color.hpp"

#include "CBaseTypes.hpp"
#include "CHandle.hpp"

#include "CUtlString.hpp"
#include "CUtlSymbolLarge.hpp"
#include "CUtlVector.hpp"
#include "CUtlStringToken.hpp"
#include "CStrongHandle.hpp"

#include <DeadLock/SDK/Math/Rect_t.hpp>
#include <DeadLock/SDK/CSchemaOffset.hpp>
#include <DeadLock/SDK/Interface/CShemaSystemSDK.hpp>

class CSkeletonInstance;

struct alignas( 16 ) CBoneData
{
	Vector3 position;
	float scale;
	Vector3 rotation;
};

class CModel
{
private:
	PAD(0x130 + 0x38);

public:
	const char** m_szBoneNames;
	uint32 m_nBoneCount;
};

class CHitBox
{
private:
	PAD(0x70);

public:
	SCHEMA_OFFSET("CHitBox", "m_name", m_name, CUtlString);
	SCHEMA_OFFSET("CHitBox", "m_sBoneName", m_sBoneName, CUtlString);
};

class CHitBoxSet
{
public:
	SCHEMA_OFFSET("CHitBoxSet", "m_HitBoxes", m_HitBoxes, CUtlVector< CHitBox >);
};

class CModelState
{
private:
	PAD(0x80);

public:
	CBoneData* m_pBones;

public:
	SCHEMA_OFFSET("CModelState", "m_hModel", m_hModel, CStrongHandle<CModel>);
	SCHEMA_OFFSET("CModelState", "m_ModelName", m_ModelName, CUtlSymbolLarge);
};

class PlayerDataGlobal_t
{
public:
	SCHEMA_OFFSET("PlayerDataGlobal_t", "m_iLevel", m_iLevel, int32);
	SCHEMA_OFFSET("PlayerDataGlobal_t", "m_iHealthMax", m_iHealthMax, int32);
	SCHEMA_OFFSET("PlayerDataGlobal_t", "m_flHealthRegen", m_flHealthRegen, float32);
	SCHEMA_OFFSET("PlayerDataGlobal_t", "m_nHeroID", m_nHeroID, HeroID_t);
	SCHEMA_OFFSET("PlayerDataGlobal_t", "m_iHealth", m_iHealth, int32);
	SCHEMA_OFFSET("PlayerDataGlobal_t", "m_bAlive", m_bAlive, bool);
};

class CCollisionProperty
{
public:
	SCHEMA_OFFSET("CCollisionProperty", "m_vecMins", m_vecMins, Vector3);
	SCHEMA_OFFSET("CCollisionProperty", "m_vecMaxs", m_vecMaxs, Vector3);
};

class CPlayerPawnComponent
{
public:
};

class CBaseModifier
{
public:
	SCHEMA_OFFSET("CBaseModifier", "m_nAbilitySubclassID", m_nAbilitySubclassID, uint32);
	SCHEMA_OFFSET("CBaseModifier", "m_flLastAppliedTime",  m_flLastAppliedTime,  float);
	SCHEMA_OFFSET("CBaseModifier", "m_flCreationTime",     m_flCreationTime,     float);
	SCHEMA_OFFSET("CBaseModifier", "m_flDuration",         m_flDuration,         float);
	SCHEMA_OFFSET("CBaseModifier", "m_iAttributes",        m_iAttributes,        uint8);
	SCHEMA_OFFSET("CBaseModifier", "m_iTeam",              m_iTeam,              uint8);
	SCHEMA_OFFSET("CBaseModifier", "m_iStackCount",        m_iStackCount,        int16);
	SCHEMA_OFFSET("CBaseModifier", "m_eDestroyReason",     m_eDestroyReason,     uint8);
	SCHEMA_OFFSET("CBaseModifier", "m_bDisabled",          m_bDisabled,          bool);
	SCHEMA_OFFSET("CBaseModifier", "m_bInAuraRange",       m_bInAuraRange,       bool);

	// Returns the MSVC RTTI decorated type name for this modifier instance.
	// Format: ".?AVCCitadel_Modifier_VoidSphere@@"
	// Uses vtable[-1] (Complete Object Locator) to locate the TypeDescriptor.
	// Works universally for any subclass without hardcoded addresses.
	const char* GetRTTIName() const
	{
		const uintptr_t vtable = *reinterpret_cast<const uintptr_t*>(this);
		if (!vtable) return nullptr;

		// MSVC x64: COL pointer sits at vtable[-1] (8 bytes before first slot)
		const uintptr_t col = *reinterpret_cast<const uintptr_t*>(vtable - sizeof(uintptr_t));
		if (!col) return nullptr;

		// COL+20: RVA of the COL itself — subtract from COL address to get module base
		const uint32_t  selfRVA = *reinterpret_cast<const uint32_t*>(col + 20);
		const uintptr_t modBase = col - selfRVA;

		// COL+12: RVA of the TypeDescriptor; TypeDescriptor+16: decorated name string
		const uint32_t tdRVA = *reinterpret_cast<const uint32_t*>(col + 12);
		return reinterpret_cast<const char*>(modBase + tdRVA + 16);
	}

	// Returns true if the runtime class of this modifier exactly matches shortName.
	// MSVC decorated format: ".?AVShortName@@" — we skip ".?AV" and check the trailing '@'.
	// Example: pMod->IsClass("CCitadel_Modifier_VoidSphere")
	bool IsClass(const char* shortName) const
	{
		if (!shortName) return false;
		const char* rtti = GetRTTIName();
		if (!rtti) return false;

		// Skip ".?AV" prefix (4 chars)
		const char* nameStart = rtti + 4;
		const size_t len = strlen(shortName);
		return strncmp(nameStart, shortName, len) == 0 && nameStart[len] == '@';
	}
};

class CModifierProperty
{
public:
	// List of all modifier instances on the entity (active or passive).
	SCHEMA_OFFSET("CModifierProperty", "m_vecModifiers", m_vecModifiers, CUtlVectorFixed<CBaseModifier*>);
};

// CNetworkAbilityVec — concrete type for CNetworkUtlVectorBase<CHandle> ability vectors.
// Layout confirmed by dezlock-dump types.hpp: count@+0x00, data_ptr@+0x08 (24 bytes total).
// NOTE: C_NetworkUtlVectorBase is #defined to CUtlVector in CBaseTypes.hpp (different layout),
// so we use a distinct non-macro-conflicting name here.
struct CNetworkAbilityVec
{
	int32_t m_nCount; // +0x00
	int32_t m_nCapacity; // +0x04
	CHandle* m_pData; // +0x08
	uint64_t _pad; // +0x10  (total: 24 bytes)

	int Count() const
	{
		return m_nCount;
	}

	bool IsEmpty() const
	{
		return m_nCount <= 0;
	}

	CHandle& operator[](int i)
	{
		return m_pData[i];
	}
};

class IHandleEntity
{
public:
	virtual ~IHandleEntity() {}
};

class CEntityIdentity
{
public:
	SCHEMA_OFFSET_CUSTOM(pBaseEntity, 0x0, C_BaseEntity*);
	SCHEMA_OFFSET_CUSTOM(Handle, 0x10, CHandle);

public:
	// entity2.dll no longer registers its schema — use hardcoded offsets from dump
	SCHEMA_OFFSET_CUSTOM(Name, 0x18, CUtlSymbolLarge);
	SCHEMA_OFFSET_CUSTOM(DesingerName, 0x20, CUtlSymbolLarge);
};

class CEntityInstance : public IHandleEntity
{
public:
	auto GetSchemaClassBinding() -> CSchemaClassBinding*
	{
		CSchemaClassBinding* pBinding = nullptr;

		VirtualFn(void)(CEntityInstance*, CSchemaClassBinding**);
		vget<Fn>(this, index::CSchemaSystem::SchemaClassInfo)(this, &pBinding);

		return pBinding;
	}

public:
	SCHEMA_OFFSET("CEntityInstance", "m_pEntity", pEntityIdentity, CEntityIdentity*);
};

class CGameSceneNode
{
public:
	SCHEMA_OFFSET("CGameSceneNode", "m_bDormant", m_bDormant, bool);
	SCHEMA_OFFSET("CGameSceneNode", "m_vecAbsOrigin", m_vecAbsOrigin, Vector3);

public:
	auto GetBonePosition(int32 BoneIndex, Vector3& BonePos) -> bool;

public:
	auto GetSkeletonInstance() -> CSkeletonInstance*
	{
		return reinterpret_cast<CSkeletonInstance*>(this);
	}
};

class CSkeletonInstance : public CGameSceneNode
{
public:
	SCHEMA_OFFSET("CSkeletonInstance", "m_modelState", m_modelState, CModelState);

public:
	auto CalcWorldSpaceBones(unsigned int Mask) -> void;
};

class CNetworkVelocityVector
{
public:
	SCHEMA_OFFSET("CNetworkVelocityVector", "m_vecX", m_vecX, float32);
	SCHEMA_OFFSET("CNetworkVelocityVector", "m_vecY", m_vecY, float32);
	SCHEMA_OFFSET("CNetworkVelocityVector", "m_vecZ", m_vecZ, float32);

	float x() const { return const_cast<CNetworkVelocityVector*>(this)->m_vecX(); }
	float y() const { return const_cast<CNetworkVelocityVector*>(this)->m_vecY(); }
	float z() const { return const_cast<CNetworkVelocityVector*>(this)->m_vecZ(); }
};

class C_BaseEntity : public CEntityInstance
{
public:
	SCHEMA_OFFSET("C_BaseEntity", "m_pGameSceneNode", m_pGameSceneNode, CGameSceneNode*);
	SCHEMA_OFFSET("C_BaseEntity", "m_iTeamNum", m_iTeamNum, uint8);
	SCHEMA_OFFSET("C_BaseEntity", "m_fFlags", m_fFlags, uint32);
	SCHEMA_OFFSET("C_BaseEntity", "m_MoveType", m_MoveType, MoveType_t);
	SCHEMA_OFFSET("C_BaseEntity", "m_vecVelocity", m_vecVelocity, CNetworkVelocityVector);
	SCHEMA_OFFSET("C_BaseEntity", "m_CBodyComponent", m_CBodyComponent, void*);
	SCHEMA_OFFSET("C_BaseEntity", "m_pRenderComponent", m_pRenderComponent, void*);
	SCHEMA_OFFSET("C_BaseEntity", "m_hOwnerEntity", m_hOwnerEntity, CHandle);
	SCHEMA_OFFSET("C_BaseEntity", "m_pModifierProp", m_pModifierProp, CModifierProperty*);

	// Returns true when a modifier from the given ability token is active on this entity.
	// Active modifiers have flDuration > 0; passive/permanent ones have flDuration == -1.
	// If modifierClass is provided, the modifier's MSVC RTTI type must also match exactly.
	// Example: IsModifierActive(k_EtherealShift, "CCitadel_Modifier_Bubble")
	bool IsModifierActive(const CUtlStringToken& token, const char* modifierClass = nullptr) const
	{
		auto* pModProp = const_cast<C_BaseEntity*>(this)->m_pModifierProp();
		if (!pModProp)
			return false;

		const auto& vec = pModProp->m_vecModifiers();
		const int nCount = vec.size;

		if (nCount <= 0 || nCount > 128)
			return false;

		for (int i = 0; i < nCount; ++i)
		{
			auto* pMod = vec[i];
			if (!pMod)
				continue;

			if (pMod->m_nAbilitySubclassID() != token.m_nHashCode)
				continue;

			if (pMod->m_flDuration() <= 0.f)
				continue;

			if (modifierClass && !pMod->IsClass(modifierClass))
				continue;

			return true;
		}

		return false;
	}

	auto IsCitadelPlayerController() -> bool;
	auto IsCitadelPlayerPawn() -> bool;
	auto IsNpcTrooper() -> bool;
	auto IsCitadelTeam() -> bool;
	auto IsCitadelFissureWall() -> bool;
	auto GetOrigin() -> const Vector3&;
	auto GetBoneIdByName(const char* szName) -> int;
	auto GetHitBoxSet() -> CHitBoxSet*;
	auto ComputeHitboxBounds(Vector3& mins, Vector3& maxs) -> bool;
	auto ComputeHitboxBounds(Rect_t& out) -> bool;
};

class C_BaseModelEntity : public C_BaseEntity
{
public:
	auto GetBoundingBox(Rect_t& out) -> bool;

public:
	SCHEMA_OFFSET("C_BaseModelEntity", "m_Collision", m_Collision, CCollisionProperty);
};

class CBaseAnimGraph : public C_BaseModelEntity
{
public:
};

class C_BaseFlex : public CBaseAnimGraph
{
public:
};

class C_BaseCombatCharacter : public C_BaseFlex
{
public:
};

class C_BaseToggle : public C_BaseModelEntity
{
public:
};

class C_BaseTrigger : public C_BaseToggle
{
public:
};

class C_PostProcessingVolume : public C_BaseTrigger
{
public:
	SCHEMA_OFFSET("C_PostProcessingVolume", "m_flMinExposure", m_flMinExposure, float32);
	SCHEMA_OFFSET("C_PostProcessingVolume", "m_flMaxExposure", m_flMaxExposure, float32);
	SCHEMA_OFFSET("C_PostProcessingVolume", "m_bExposureControl", m_bExposureControl, bool);
};

class CPlayer_CameraServices : public CPlayerPawnComponent
{
public:
	SCHEMA_OFFSET("CPlayer_CameraServices", "m_hActivePostProcessingVolume", m_hActivePostProcessingVolume, CHandle);
	// C_PostProcessingVolume

	// RTTI offset 0x48 — float[2] { pitch, yaw }, see Offsets.hpp: CPlayer_CameraServices_m_flRecoilPunch
	SCHEMA_OFFSET_CUSTOM(m_flRecoilPunch, 0x48, float);
};

// client.dll: CPlayer_ObserverServices (+0x4C m_hObserverTarget)
class CPlayer_ObserverServices
{
public:
	SCHEMA_OFFSET("CPlayer_ObserverServices", "m_hObserverTarget", m_hObserverTarget, CHandle);
};

class C_BasePlayerPawn : public C_BaseCombatCharacter
{
public:
	SCHEMA_OFFSET("C_BasePlayerPawn", "m_pCameraServices", m_pCameraServices, CPlayer_CameraServices*);
	SCHEMA_OFFSET("C_BasePlayerPawn", "m_vOldOrigin", m_vOldOrigin, Vector3);
	SCHEMA_OFFSET("C_BasePlayerPawn", "m_pObserverServices", m_pObserverServices, CPlayer_ObserverServices*);
};

// CCitadelAbilityComponent is an inline struct embedded inside player/NPC entities.
// m_vecAbilities schema entry lives in server.dll but is indexed by CSchemaOffset
// (which reads all loaded DLL type-scopes including server.dll).
class CCitadelAbilityComponent
{
public:
	// server.dll schema: CCitadelAbilityComponent::m_vecAbilities (+0x80, 24 bytes)
	SCHEMA_OFFSET("CCitadelAbilityComponent", "m_vecAbilities", m_vecAbilities, CNetworkAbilityVec);
};

class CCitadelWeaponInfo
{
public:
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flBulletDamage", m_flBulletDamage, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_iBullets", m_iBullets, int32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flCycleTime", m_flCycleTime, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_iClipSize", m_iClipSize, int32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flRange", m_flRange, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flRangeWhileZoomed", m_flRangeWhileZoomed, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flDamageFalloffStartRange", m_flDamageFalloffStartRange, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flDamageFalloffEndRange", m_flDamageFalloffEndRange, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flDamageFalloffBias", m_flDamageFalloffBias, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flDamageFalloffStartScale", m_flDamageFalloffStartScale, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flDamageFalloffEndScale", m_flDamageFalloffEndScale, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flBulletSpeed", m_flBulletSpeed, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flBulletSpeedRandomFactor", m_flBulletSpeedRandomFactor, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flBulletGravityScale", m_flBulletGravityScale, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flBulletRadius", m_flBulletRadius, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flBulletRadiusVsWorld", m_flBulletRadiusVsWorld, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flBulletLifetime", m_flBulletLifetime, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flBulletInheritShooterVelocityScale", m_flBulletInheritShooterVelocityScale, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_Spread", m_Spread, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flScatterYawScale", m_flScatterYawScale, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flVerticalPunch", m_flVerticalPunch, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flHorizontalPunch", m_flHorizontalPunch, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_reloadDuration", m_reloadDuration, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flZoomMoveSpeedPercent", m_flZoomMoveSpeedPercent, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flShootMoveSpeedPercent", m_flShootMoveSpeedPercent, float32)
	SCHEMA_OFFSET("CCitadelWeaponInfo", "m_flReloadMoveSpeedPercent", m_flReloadMoveSpeedPercent, float32)
};

// ── CitadelAbilityProperty_t ──────────────────────────────────────────────────
// One entry in CitadelAbilityVData::m_mapAbilityProperties.
// Use CAbilityProperty_ComputeValue / CAbilityProperty_ComputeScaled (FunctionListSDK.hpp)
// to get calculated values — DO NOT read fields directly.
struct CitadelAbilityProperty_t {};

// ── CAbilityPropertiesMap_t ───────────────────────────────────────────────────
// Mirrors the memory layout of CUtlOrderedMap<CUtlString, CitadelAbilityProperty_t>
// (40 bytes, confirmed by dezlock-dump schema — m_mapAbilityProperties @ VData+0xE68).
// Node array stride = 200 bytes:
//   node+0x10 : char*  key  (first field of CUtlString)
//   node+0x18 : CitadelAbilityProperty_t  value
struct CAbilityPropertiesMap_t
{
	uint32_t               _pad0;            // +0x00  functor / alignment
	uint32_t               _pad1;            // +0x04
	int32_t                m_nTotalElements; // +0x08  capacity (NOT the active count)
	int32_t                m_nNumElements;   // +0x0C  count | 0x80000000 flag
	const uint8_t*         m_pElements;      // +0x10  CUtlRBTree node array
	int32_t                m_nRoot;          // +0x18  RBTree root index
	int32_t                _pad2;            // +0x1C
	uint64_t               _pad3;            // +0x20
	// total: 0x28 = 40 bytes

	int32_t Count() const noexcept { return m_nNumElements & 0x7FFFFFFF; }

	const CitadelAbilityProperty_t* Find( const char* szName ) const noexcept
	{
		const int32_t count = Count();
		if ( !szName || !*szName || count <= 0 || count > 256 || !m_pElements )
			return nullptr;

		constexpr int32_t k_Stride = 224;
		for ( int32_t i = 0; i < count; ++i )
		{
			const auto* node = m_pElements + static_cast<ptrdiff_t>( i ) * k_Stride;
			const char* key  = *reinterpret_cast<const char* const*>( node + 0x10 );
			if ( key && strcmp( key, szName ) == 0 )
				return reinterpret_cast<const CitadelAbilityProperty_t*>( node + 0x18 );
		}
		return nullptr;
	}

	// Calls fn(const char* name, const CitadelAbilityProperty_t* prop) for every entry.
	template<typename Fn>
	void ForEach( Fn&& fn ) const noexcept
	{
		const int32_t count = Count();
		if ( count <= 0 || count > 256 || !m_pElements )
			return;

		constexpr int32_t k_Stride = 224;
		for ( int32_t i = 0; i < count; ++i )
		{
			const auto* node = m_pElements + static_cast<ptrdiff_t>( i ) * k_Stride;
			const char* key  = *reinterpret_cast<const char* const*>( node + 0x10 );
			const auto* val  = reinterpret_cast<const CitadelAbilityProperty_t*>( node + 0x18 );
			if ( key )
				fn( key, val );
		}
	}
};

class CitadelAbilityVData
{
public:
	SCHEMA_OFFSET("CitadelAbilityVData", "m_WeaponInfo", m_WeaponInfo, CCitadelWeaponInfo)
	SCHEMA_OFFSET("CitadelAbilityVData", "m_mapAbilityProperties", m_mapAbilityProperties, CAbilityPropertiesMap_t)

	const CitadelAbilityProperty_t* FindProperty( const char* szName ) noexcept
	{
		return m_mapAbilityProperties().Find( szName );
	}

	// Calls fn(const char* name, const CitadelAbilityProperty_t* prop) for every property.
	template<typename Fn>
	void EnumerateProperties( Fn&& fn ) noexcept
	{
		m_mapAbilityProperties().ForEach( std::forward<Fn>( fn ) );
	}
};

enum class EAbilitySlots_t : int16_t
{
	ESlot_Invalid            = -1,
	ESlot_Signature_1        = 0,
	ESlot_Signature_2        = 1,
	ESlot_Signature_3        = 2,
	ESlot_Signature_4        = 3,
	ESlot_ActiveItem_1       = 4,
	ESlot_ActiveItem_2       = 5,
	ESlot_ActiveItem_3       = 6,
	ESlot_ActiveItem_4       = 7,
	ESlot_Ability_Held       = 8,
	ESlot_Ability_ZipLine    = 9,
	ESlot_Ability_Mantle     = 10,
	ESlot_Ability_ClimbRope  = 11,
	ESlot_Ability_Jump       = 12,
	ESlot_Ability_Slide      = 13,
	ESlot_Ability_Teleport   = 14,
	ESlot_Ability_ZipLineBoost = 15,
	ESlot_Cosmetic_1         = 16,
	ESlot_Ability_Innate_1   = 17,
	ESlot_Ability_Innate_2   = 18,
	ESlot_Ability_Innate_3   = 19,
	ESlot_Weapon_Secondary   = 20,
	ESlot_Weapon_Primary     = 21,
	ESlot_Weapon_Melee       = 22,
	ESlot_None               = 23,
};

// Minimal base class for ability entities.
class C_CitadelBaseAbility : public C_BaseEntity
{
public:
	// m_pSubclassVData exists in schema under two classes with different offsets (0x10 and 0x390).
	// GetOffset() is not guaranteed to return the right one, so we pin the confirmed offset directly.
	// Offset 0x390 verified: schema entry at client.dll+0xE489B0 and live memory scan 2026-03-04.
	SCHEMA_OFFSET_CUSTOM(m_pSubclassVData, 0x390, CitadelAbilityVData*);
	SCHEMA_OFFSET("C_CitadelBaseAbility", "m_eAbilitySlot", m_eAbilitySlot, EAbilitySlots_t);
	// +0x758 — active upgrade info / bitmask [MNetworkEnable]. Bit N = upgrade tier N purchased.
	// Used by CAbilityProperty_ComputeValue / ComputeScaled to select the correct value tier.
	// NOTE: renamed from m_nUpgradeBits → m_nUpgradeInfo in a recent game patch.
	SCHEMA_OFFSET("C_CitadelBaseAbility", "m_nUpgradeInfo", m_nUpgradeInfo, int32_t);
	SCHEMA_OFFSET("C_CitadelBaseAbility", "m_flCooldownStart", m_flCooldownStart, float32);
	SCHEMA_OFFSET("C_CitadelBaseAbility", "m_flCooldownEnd", m_flCooldownEnd, float32);
	SCHEMA_OFFSET("C_CitadelBaseAbility", "m_iRemainingCharges", m_iRemainingCharges, int32_t);

	// Returns the fully computed ability property value (upgrade tiers + stat scaling).
	// Calls the game's own CAbilityProperty::ComputeValue + CAbilityProperty::ComputeScaled
	// so all item bonuses, spirit power, hero level etc. are accounted for automatically.
	// Returns the unscaled base value when stat provider is transiently null (item-buy window).
	// Returns 0.0f if the VData or property name is not found or the function pointers
	// are unavailable (pattern scan failed — feature degraded).
	//
	// Example:  float dmg = pAbility->GetPropertyValue("damage");
	float GetPropertyValue( const char* szPropName, int sourceIdx = 0 ) noexcept
	{
		auto* pVData = m_pSubclassVData();
		if ( !pVData || !szPropName || !*szPropName )
			return 0.0f;

		const auto* pProp = pVData->FindProperty( szPropName );
		if ( !pProp )
			return 0.0f;

		const int32_t upgradeBits = m_nUpgradeInfo();

		// CAbilityProperty::ComputeValue is optional; bail if the pattern didn't match.
		using FnComputeValue = float ( __fastcall* )( const void*, int, int );
		auto* fnComputeValue = static_cast<FnComputeValue>( GetFunctionList()->CAbilityProperty_ComputeValue.GetFunction() );
		if ( !fnComputeValue )
			return 0.0f;

		const float base = fnComputeValue( pProp, upgradeBits, sourceIdx );

		// CAbilityProperty::ComputeScaled is optional as well.
		using FnComputeScaled = bool ( __fastcall* )( int, int, const void*, const char*, float, char, char, float*, float* );
		auto* fnComputeScaled = static_cast<FnComputeScaled>( GetFunctionList()->CAbilityProperty_ComputeScaled.GetFunction() );
		if ( !fnComputeScaled )
			return base; // return unscaled base value

		// Ensure the ability service context has a valid entity handle before
		// ComputeScaled runs.  GetEntityStat (sub_180816A70 path) can trigger
		// sub_18079E680(serviceCtx) which resets serviceCtx+8/+12 to -1; with -1
		// sub_1807A8430 exits early and leaves the 97-stat buffer uninitialised,
		// producing NaN for any spirit-scaled property.
		const int32_t casterHandle = m_hOwnerEntity().m_Index;
		PrepareAbilityPropertyContext( casterHandle );

		float outValue = base, outDelta = 0.0f;
		fnComputeScaled( casterHandle, upgradeBits, pProp,
		                 nullptr, base, 0, 0, &outValue, &outDelta );
		return outValue;
	}
};

// Primary weapon ability entity (per-hero subclasses all inherit from this).
// All fields marked [MNetworkEnable][MNetworkUserGroup] are replicated only to
// the owning player, so they are valid on the client for our own weapon.
//
// Window timing (no curtime needed):
//   windowDelaySec = m_flReloadAvailableTime - m_flLastReloadStartTime
//   → both are GameTime_t floats; the DIFFERENCE is the delay from reload start
//     to window open in seconds, independent of absolute server time.
class CCitadel_Ability_PrimaryWeapon : public C_CitadelBaseAbility
{
public:
	// +0x11D8  GameTime_t  server time when the current reload started [MNetworkEnable][MNetworkUserGroup] (was 0x1118)
	SCHEMA_OFFSET("CCitadel_Ability_PrimaryWeapon", "m_flLastReloadStartTime", m_flLastReloadStartTime, float32);
	// +0x11DC  GameTime_t  server time when weapon can fire next; equals reload end time [MNetworkEnable][MNetworkUserGroup] (was 0x111C)
	SCHEMA_OFFSET("CCitadel_Ability_PrimaryWeapon", "m_flNextPrimaryAttack", m_flNextPrimaryAttack, float32);
	// +0x132C  bool        weapon is currently reloading [MNetworkEnable] (was 0x1264)
	SCHEMA_OFFSET("CCitadel_Ability_PrimaryWeapon", "m_bInReload", m_bInReload, bool);
	// +0x1330  GameTime_t  game time when a queued reload was requested (was 0x1268)
	SCHEMA_OFFSET("CCitadel_Ability_PrimaryWeapon", "m_reloadQueuedStartTime", m_reloadQueuedStartTime, float32);
	// +0x1334  GameTime_t  game time when the active-reload window opens [MNetworkEnable][MNetworkUserGroup] (was 0x126C)
	//          windowDelaySec = m_flReloadAvailableTime - m_flLastReloadStartTime (no curtime needed)
	SCHEMA_OFFSET("CCitadel_Ability_PrimaryWeapon", "m_flReloadAvailableTime", m_flReloadAvailableTime, float32);
	// +0x1338  bool        Active Reload item equipped + off cooldown + reload in progress [MNetworkEnable][MNetworkUserGroup] (was 0x1270)
	SCHEMA_OFFSET("CCitadel_Ability_PrimaryWeapon", "m_bCanActiveReload", m_bCanActiveReload, bool);
	// +0x1348  float       total accumulated pause time added to the current attack delay (freeze compensation) (was 0x1280)
	SCHEMA_OFFSET("CCitadel_Ability_PrimaryWeapon", "m_flAttackDelayPauseTotalTime", m_flAttackDelayPauseTotalTime, float32);
	// +0x134C  GameTime_t  server time when the current attack-delay pause ends (was 0x1284)
	SCHEMA_OFFSET("CCitadel_Ability_PrimaryWeapon", "m_flAttackDelayPauseEndTime", m_flAttackDelayPauseEndTime, float32);
};

class CCitadel_Ability_Chrono_KineticCarbine : public C_CitadelBaseAbility
{
public:

};

class CCitadel_Ability_Hornet_Snipe : public C_CitadelBaseAbility
{
public:
	// +0x1874  GameTime_t  server time when scope was entered; 0 when not scoped
	SCHEMA_OFFSET("CCitadel_Ability_Hornet_Snipe", "m_flScopeStartTime", m_flScopeStartTime, float);
	SCHEMA_OFFSET("CCitadel_Ability_Hornet_Snipe", "m_iSnipeKills", m_iSnipeKills, int32_t);
};

class CCitadelPlayerPawnBase : public C_BasePlayerPawn
{};

class C_CitadelPlayerPawn : public CCitadelPlayerPawnBase
{
public:
	SCHEMA_OFFSET("C_CitadelPlayerPawn", "m_angClientCamera", m_angClientCamera, QAngle);
	SCHEMA_OFFSET("C_CitadelPlayerPawn", "m_angEyeAngles", m_angEyeAngles, QAngle);
	// client.dll schema: C_CitadelPlayerPawn::m_CCitadelAbilityComponent (+0x14C0, inline struct)
	SCHEMA_OFFSET("C_CitadelPlayerPawn", "m_CCitadelAbilityComponent", m_CCitadelAbilityComponent, CCitadelAbilityComponent);


};

class CBasePlayerController : public C_BaseEntity
{
public:
	SCHEMA_OFFSET("CBasePlayerController", "m_hPawn", m_hPawn, CHandle); // C_BasePlayerPawn
	PSCHEMA_OFFSET("CBasePlayerController", "m_iszPlayerName", m_iszPlayerName, char); // char[128]
};

class CCitadelPlayerController : public CBasePlayerController
{
public:
	SCHEMA_OFFSET("CCitadelPlayerController", "m_hHeroPawn", m_hHeroPawn, CHandle); // C_CitadelPlayerPawn
	SCHEMA_OFFSET("CCitadelPlayerController", "m_PlayerDataGlobal", m_PlayerDataGlobal, PlayerDataGlobal_t);
};

class C_AI_BaseNPC : public C_BaseCombatCharacter
{
public:
	SCHEMA_OFFSET("C_AI_BaseNPC", "m_NPCState", m_NPCState, NPC_STATE);
};

class C_AI_CitadelNPC : public C_AI_BaseNPC
{
public:

};

class C_NPC_Trooper : public C_AI_CitadelNPC
{
public:
};

class C_EnvSky : public C_BaseModelEntity
{
public:
	SCHEMA_OFFSET("C_EnvSky", "m_vTintColor", m_vTintColor, Color);
	SCHEMA_OFFSET("C_EnvSky", "m_vTintColorLightingOnly", m_vTintColorLightingOnly, Color);
};

// ── CSceneAnimatableObject ────────────────────────────────────────────────────
// Scene-system render object; one instance per rendered entity per draw call.
//   +0x000  vtable
//   ...
//   +0x0C0  hOwner  — entity owner handle

class CSceneAnimatableObject
{
	PAD(0xC0);
	SCHEMA_OFFSET_CUSTOM(hOwner, 0xC0, CHandle);
};
