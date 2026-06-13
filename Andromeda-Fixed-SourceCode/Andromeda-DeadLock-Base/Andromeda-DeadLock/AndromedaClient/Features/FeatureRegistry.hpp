#pragma once

#include "IFeature.hpp"

namespace FeatureRegistry
{
inline std::vector<IFeature*>& GetRootFeatures()
{
	static std::vector<IFeature*> features;
	return features;
}

inline void Register(IFeature* f)
{
	GetRootFeatures().push_back(f);
}
}