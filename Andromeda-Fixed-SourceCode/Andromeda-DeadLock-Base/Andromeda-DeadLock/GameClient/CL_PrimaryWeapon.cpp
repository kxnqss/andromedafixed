#include "CL_PrimaryWeapon.hpp"

#include <DeadLock/SDK/Types/CEntityData.hpp>

static CL_PrimaryWeapon g_CL_PrimaryWeapon{};

CCitadel_Ability_PrimaryWeapon* CL_PrimaryWeapon::Get()
{
	return static_cast<CCitadel_Ability_PrimaryWeapon*>(m_Cache.Get());
}

CL_PrimaryWeapon* GetCL_PrimaryWeapon()
{
	return &g_CL_PrimaryWeapon;
}