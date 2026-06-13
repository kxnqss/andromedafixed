#pragma once

#include "FeatureRegistry.hpp"

// Full headers required so the compiler sees IFeature inheritance for each class.
#include <AndromedaClient/Features/CAimbot/CAimbot.hpp>
#include <AndromedaClient/Features/CVisual/CVisual.hpp>
#include <AndromedaClient/Features/CMisc/CMisc.hpp>
#include <AndromedaClient/Features/CHeroes/CHeroes.hpp>

namespace FeatureRegistryInit
{
// Call once at startup, before LoadConfig, after all static singletons are constructed.
// Order determines tab order: Aimbot → Visuals → Heroes → Misc.
inline void Init()
{
	FeatureRegistry::Register(GetAimbot());
	FeatureRegistry::Register(GetVisual());
	FeatureRegistry::Register(GetHeroes());
	FeatureRegistry::Register(GetMisc());
}
}