#pragma once

#include "HeroSkeletonTypes.hpp"
#include <Common/Common.hpp>

class C_CitadelPlayerPawn;

class CL_Bones final
{
public:
	// Returns bone pairs for this pawn (nullptr if model unknown).
	const std::vector<BonePair>* GetBonePairs(C_CitadelPlayerPawn* pPawn);

	// Returns the ordered bone-index list for the given hitbox slot.
	// The first element is the most significant bone (largest hitbox radius).
	// Returns nullptr if the model is unknown or the slot is empty.
	const std::vector<int16_t>* GetHitboxBones(C_CitadelPlayerPawn* pPawn, HitboxSlot slot);

private:
	const ModelBoneData* Resolve(C_CitadelPlayerPawn* pPawn);

	struct HandleEntry
	{
		uintptr_t modelKey = 0;
		const ModelBoneData* pData = nullptr;
	};

	std::unordered_map<uint32_t, HandleEntry> m_HandleCache;
	std::unordered_map<uintptr_t, const ModelBoneData*> m_ModelCache;
};

CL_Bones* GetCL_Bones();