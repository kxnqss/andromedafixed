#include "CL_CitadelPlayerController.hpp"

#include <DeadLock/SDK/Interface/CGameEntitySystem.hpp>

static CL_CitadelPlayerController g_CL_CitadelPlayerController{};

CCitadelPlayerController* CL_CitadelPlayerController::GetLocal()
{
	return CGameEntitySystem::GetLocalCitadelPlayerController();
}

CL_CitadelPlayerController* GetCL_CitadelPlayerController()
{
	return &g_CL_CitadelPlayerController;
}