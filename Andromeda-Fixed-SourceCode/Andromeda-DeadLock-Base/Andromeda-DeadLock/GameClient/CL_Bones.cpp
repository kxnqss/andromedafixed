#include "CL_Bones.hpp"

#include <Common/DevLog.hpp>
#include <DeadLock/SDK/Types/CEntityData.hpp>

#include "HeroSkeletonPairs.hpp"

static CL_Bones g_CL_Bones{};

// ── Core cache resolution ──────────────────────────────────────────────────────
// Returns a pointer into the static g_HeroModelData table (no copies).
// Nullptr if the model has no entry (unknown hero or transform not yet extracted).

const ModelBoneData* CL_Bones::Resolve(C_CitadelPlayerPawn* pPawn)
{
	auto* pIdentity = pPawn->pEntityIdentity();
	if (!pIdentity)
		return nullptr;

	const uint32_t handle = pIdentity->Handle().m_Index;

	// We always need szModel to validate the L1 entry (entity handle stays the
	// same across hero transforms, but the model path changes).
	auto* pNode = pPawn->m_pGameSceneNode();
	if (!pNode)
		return nullptr;

	auto* pSkel = pNode->GetSkeletonInstance();
	if (!pSkel)
		return nullptr;

	const char* szModel = pSkel->m_modelState().m_ModelName().String();
	const auto modelKey = reinterpret_cast<uintptr_t>(szModel);

	if (!szModel || !szModel[0])
		return nullptr;

	// Level 1: entity handle → { modelKey, data ptr }
	// Valid only when modelKey matches — transforms invalidate this entry.
	if (const auto hit = m_HandleCache.find(handle); hit != m_HandleCache.end())
	{
		if (hit->second.modelKey == modelKey)
			return hit->second.pData;

		// Model changed (e.g. transform) — evict stale L1 entry and fall through.
		m_HandleCache.erase(hit);
	}

	// Level 2: interned model name ptr → data ptr  (once per unique model type)
	auto modelIt = m_ModelCache.find(modelKey);
	if (modelIt == m_ModelCache.end())
	{
		const auto tableIt = g_HeroModelData.find(szModel);
		if (tableIt == g_HeroModelData.end())
		{
			DEV_LOG("[CL_Bones] no entry for '%s'\n", szModel);
			m_ModelCache[modelKey] = nullptr;
		}
		else
		{
			m_ModelCache[modelKey] = &tableIt->second;
		}

		modelIt = m_ModelCache.find(modelKey);
	}

	const ModelBoneData* pData = modelIt->second;
	m_HandleCache[handle] = {modelKey, pData};
	return pData;
}

// ── Public API ─────────────────────────────────────────────────────────────────

const std::vector<BonePair>* CL_Bones::GetBonePairs(C_CitadelPlayerPawn* pPawn)
{
	const ModelBoneData* pData = Resolve(pPawn);
	return (pData && !pData->pairs.empty()) ? &pData->pairs : nullptr;
}

const std::vector<int16_t>* CL_Bones::GetHitboxBones(C_CitadelPlayerPawn* pPawn, HitboxSlot slot)
{
	const ModelBoneData* pData = Resolve(pPawn);
	if (!pData)
		return nullptr;

	const int idx = static_cast<int>(slot);
	if (idx < 0 || idx >= kHitboxSlotCount)
		return nullptr;

	const auto& vec = pData->slotBones[idx];
	return vec.empty() ? nullptr : &vec;
}

CL_Bones* GetCL_Bones()
{
	return &g_CL_Bones;
}