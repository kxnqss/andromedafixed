#include "CHeroes.hpp"

#include <AndromedaClient/Fonts/FontAwesomeIcon.hpp>
#include <DeadLock/SDK/Types/CEntityData.hpp>
#include <DeadLock/SDK/Update/CUserCmd.hpp>
#include <GameClient/CL_CitadelPlayerController.hpp>

static CHeroes g_CHeroes{};

const char* CHeroes::GetName() const
{
	return "Heroes";
}

const char8_t* CHeroes::GetIcon() const
{
	return ICON_FA_USER;
}

CHeroes* GetHeroes()
{
	return &g_CHeroes;
}

static uint32_t GetLocalHeroID()
{
	auto* pCtrl = GetCL_CitadelPlayerController()->GetLocal();
	if (!pCtrl)
		return 0u;
	return pCtrl->m_PlayerDataGlobal().m_nHeroID().m_Value;
}

void CHeroes::OnCreateMove(CCitadelInput* pInput, CUserCmd* pUserCmd)
{
	const uint32_t nHeroID = GetLocalHeroID();
	if (!nHeroID)
		return;

	for (auto* pComp : m_Components)
		if (pComp->GetHeroID() == nHeroID)
			pComp->OnCreateMove(pInput, pUserCmd);
}

void CHeroes::OnPostMove(CCitadelInput* pInput)
{
	const uint32_t nHeroID = GetLocalHeroID();
	if (!nHeroID)
		return;

	for (auto* pComp : m_Components)
		if (pComp->GetHeroID() == nHeroID)
			pComp->OnPostMove(pInput);
}

void CHeroes::OnRender()
{
	const uint32_t nHeroID = GetLocalHeroID();
	if (!nHeroID)
		return;

	for (auto* pComp : m_Components)
		if (pComp->GetHeroID() == nHeroID)
			pComp->OnRender();
}