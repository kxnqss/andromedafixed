#include "C_CitadelGameRules.hpp"

#include <DeadLock/SDK/SDK.hpp>
#include <DeadLock/SDK/Update/CGlobalVarsBase.hpp>

float C_CitadelGameRules::GetMatchTime()
{
	auto* pGlobalVars = SDK::Pointers::GlobalVarsBase();
	if ( !pGlobalVars )
		return 0.f;

	const float flCurrentTime  = pGlobalVars->m_flCurrentTime();
	const float flAdjustedTime = GetPauseAdjustedTime( flCurrentTime, 0 );
	return flAdjustedTime - m_fLevelStartTime();
}
