#pragma once

#include <DeadLock/SDK/Types/CHandle.hpp>
#include <DeadLock/SDK/Types/CEntityData.hpp>

class C_CitadelBaseAbility;

// Per-pawn ability cache. Finds the ability in the given EAbilitySlots_t slot
// and caches the result keyed by (pawn handle, model pointer).
// Invalidates automatically when the pawn or hero model changes.
class CL_AbilityCache final
{
public:
	explicit CL_AbilityCache(EAbilitySlots_t eSlot) : m_eSlot(eSlot) {}

	C_CitadelBaseAbility* Get();

private:
	EAbilitySlots_t m_eSlot;
	CHandle m_hCachedPawn = {INVALID_EHANDLE_INDEX};
	CHandle m_hCachedAbility = {INVALID_EHANDLE_INDEX};
	uintptr_t m_nCachedModelKey = 0;
	uintptr_t m_nLastModelKey = 0;
};