#include "CL_CitadelPlayerPawn.hpp"
#include "CL_CitadelPlayerController.hpp"

#include <DeadLock/SDK/Types/CEntityData.hpp>

static CL_CitadelPlayerPawn g_CL_CitadelPlayerPawn{};

C_CitadelPlayerPawn* CL_CitadelPlayerPawn::GetLocal()
{
	if (auto* pLocalCitadelPlayerController = GetCL_CitadelPlayerController()->GetLocal(); pLocalCitadelPlayerController)
	{
		auto* pLocalCitadelPlayerPawn = pLocalCitadelPlayerController->m_hHeroPawn().Get<C_CitadelPlayerPawn>();

		if (pLocalCitadelPlayerPawn && pLocalCitadelPlayerPawn->IsCitadelPlayerPawn())
			return pLocalCitadelPlayerPawn;
	}

	return nullptr;
}

CL_CitadelPlayerPawn* GetCL_CitadelPlayerPawn()
{
	return &g_CL_CitadelPlayerPawn;
}