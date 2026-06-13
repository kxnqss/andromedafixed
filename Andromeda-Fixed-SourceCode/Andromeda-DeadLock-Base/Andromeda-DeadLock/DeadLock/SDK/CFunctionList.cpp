#include "CFunctionList.hpp"

static CFunctionList g_CFunctionList{};

auto CFunctionList::OnInit() -> bool
{
	struct PatternSlot {
		IBasePattern* pPattern;
		bool bCritical;  // if true, failure aborts the cheat
	};

	std::vector<PatternSlot> vPatterns =
	{
		{ &CSkeletonInstance_CalcWorldSpaceBones,                true  },
		{ &ScreenTransform,                                      true  },
		{ &CCitadelInput_GetViewAngles,                         true  },
		{ &CGameEntitySystem_GetBaseEntity,                     true  },
		{ &CGameEntitySystem_GetLocalCitadelPlayerController,   true  },
		{ &IGameEvent_GetName,                                  true  },
		{ &GetCUserCmdTick,                                     true  },
		{ &GetCUserCmdArray,                                     true  },
		{ &GetCUserCmdBySequenceNumber,                         true  },
		{ &C_EnvSky_Update,                                      true  },
		{ &C_BaseEntity_GetBoneIdByName,                         true  },
		{ &C_BaseEntity_GetHitBoxSet,                            true  },
		{ &g_CCitadelCameraManager,                              true  },
		{ &C_BaseEntity_ComputeHitboxBounds,                     true  },
		{ &CMaterialSystem2_CreateMaterial,                      true  },
		{ &KeyValues3_LoadKV3,                                   true  },
		{ &CAbilityProperty_ComputeValue,                        false }, // optional — ability stat scaling
		{ &CAbilityProperty_ComputeScaled,                       false }, // optional — ability stat scaling
		{ &QueryEntityStat,                                      true  },
		{ &QueryEntityStatVs,                                    true  },
		{ &GetAbilityServiceContext,                             true  },
	};

	auto bAllCriticalMatch = true;
	int nFailedOptional = 0;

	for (auto& Slot : vPatterns)
	{
		if (!Slot.pPattern->Search(false))
		{
			if (Slot.bCritical)
			{
				bAllCriticalMatch = false;
				DEV_LOG("[error] CFunctionList::OnInit — CRITICAL FAILED: %s\n", Slot.pPattern->GetPatternName());
			}
			else
			{
				++nFailedOptional;
				DEV_LOG("[warn] CFunctionList::OnInit — OPTIONAL FAILED (skipping): %s\n", Slot.pPattern->GetPatternName());
			}
		}
	}

	if (!bAllCriticalMatch)
	{
		char szMsg[512];
		sprintf_s(szMsg, "Critical pattern(s) failed.\nCheck debug.log for details.");
		MessageBoxA(0, szMsg, "Andromeda Init Error", MB_ICONERROR);
		return false;
	}

	if (nFailedOptional > 0)
	{
		char szMsg[256];
		sprintf_s(szMsg, "%d optional pattern(s) failed.\nSome features (ability stat scaling) will be disabled.\nCheck debug.log for details.", nFailedOptional);
		MessageBoxA(0, szMsg, "Andromeda Init Warning", MB_ICONWARNING);
	}

	return true;
}

auto GetFunctionList() -> CFunctionList*
{
	return &g_CFunctionList;
}
