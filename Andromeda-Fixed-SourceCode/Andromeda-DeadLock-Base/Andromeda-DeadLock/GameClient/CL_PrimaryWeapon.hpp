#pragma once

#include "CL_AbilityCache.hpp"

class CCitadel_Ability_PrimaryWeapon;

class CL_PrimaryWeapon final
{
public:
	CCitadel_Ability_PrimaryWeapon* Get();

private:
	CL_AbilityCache m_Cache{EAbilitySlots_t::ESlot_Weapon_Primary};
};

CL_PrimaryWeapon* GetCL_PrimaryWeapon();