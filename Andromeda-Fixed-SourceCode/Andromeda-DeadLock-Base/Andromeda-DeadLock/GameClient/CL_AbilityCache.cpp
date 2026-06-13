#include "CL_AbilityCache.hpp"

#include <DeadLock/SDK/Types/CEntityData.hpp>
#include <GameClient/CL_CitadelPlayerController.hpp>

C_CitadelBaseAbility* CL_AbilityCache::Get()
{
	auto* pCtrl = GetCL_CitadelPlayerController()->GetLocal();
	if (!pCtrl)
	{
		m_hCachedPawn = {INVALID_EHANDLE_INDEX};
		m_hCachedAbility = {INVALID_EHANDLE_INDEX};
		m_nCachedModelKey = 0;
		m_nLastModelKey = 0;
		return nullptr;
	}

	const CHandle hPawn = pCtrl->m_hHeroPawn();

	auto* pPawn = hPawn.Get<C_CitadelPlayerPawn>();
	if (!pPawn)
	{
		m_hCachedPawn = {INVALID_EHANDLE_INDEX};
		m_hCachedAbility = {INVALID_EHANDLE_INDEX};
		m_nCachedModelKey = 0;
		m_nLastModelKey = 0;
		return nullptr;
	}

	// Derive model key from the interned model name pointer — same strategy as CL_Bones.
	auto* pNode = pPawn->m_pGameSceneNode();
	auto* pSkel = pNode ? pNode->GetSkeletonInstance() : nullptr;
	const char* szModel = pSkel ? pSkel->m_modelState().m_ModelName().String() : nullptr;
	const uintptr_t nModelKey = reinterpret_cast<uintptr_t>(szModel);

	const uintptr_t nLastModelKey = m_nLastModelKey;
	m_nLastModelKey = nModelKey;

	// Hot path: same pawn handle and model — validate via CHandle (checks serial through entity
	// system, never dereferences a potentially stale raw pointer).
	if (hPawn == m_hCachedPawn && nModelKey == m_nCachedModelKey && m_hCachedAbility.IsValid())
	{
		auto* pAb = m_hCachedAbility.Get<C_CitadelBaseAbility>();
		if (pAb)
			return pAb;
	}

	m_hCachedPawn = hPawn;
	m_hCachedAbility = {INVALID_EHANDLE_INDEX};
	m_nCachedModelKey = 0;

	// If model just changed this frame, abilities may not have updated yet. Wait 1 frame.
	if (nModelKey != nLastModelKey)
		return nullptr;

	if (!szModel || !szModel[0])
		return nullptr;

	m_nCachedModelKey = nModelKey;

	auto& vec = pPawn->m_CCitadelAbilityComponent().m_vecAbilities();
	for (int i = 0; i < vec.Count(); ++i)
	{
		const CHandle hAb = vec[i];
		if (!hAb.IsValid())
			continue;

		auto* pAb = hAb.Get<C_CitadelBaseAbility>();
		if (!pAb)
			continue;

		auto* pIdent = pAb->pEntityIdentity();
		if (!pIdent || !pIdent->Handle().IsValid())
			continue;

		if (pAb->m_eAbilitySlot() == m_eSlot)
		{
			m_hCachedAbility = hAb;
			break;
		}
	}

	return m_hCachedAbility.Get<C_CitadelBaseAbility>();
}