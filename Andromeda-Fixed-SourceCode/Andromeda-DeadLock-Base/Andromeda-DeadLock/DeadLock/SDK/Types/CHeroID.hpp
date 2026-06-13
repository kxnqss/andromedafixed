#pragma once

#include <map>
#include <optional>
#include <cstdint>

struct HeroInfo
{
	const char* slug;
	std::optional<const char*> imageName;
};

inline const std::map<uint32_t, HeroInfo> HERO_MAP = {
	{1, {"inferno", {}}},
	{2, {"gigawatt", {}}},
	{3, {"hornet", {}}},
	{4, {"ghost", "spectre"}},
	{6, {"atlas", "bull"}},
	{7, {"wraith", {}}},
	{8, {"forge", "engineer"}},
	{10, {"chrono", {}}},
	{11, {"dynamo", "sumo"}},
	{12, {"kelvin", {}}},
	{13, {"haze", {}}},
	{14, {"astro", {}}},
	{15, {"bebop", {}}},
	{16, {"nano", {}}},
	{17, {"orion", "archer"}},
	{18, {"krill", "digger"}},
	{19, {"shiv", {}}},
	{20, {"tengu", {}}},
	{21, {"kali", {}}},
	{25, {"warden", {}}},
	{27, {"yamato", {}}},
	{31, {"lash", {}}},
	{35, {"viscous", {}}},
	{38, {"gunslinger", {}}},
	{39, {"yakuza", {}}},
	{46, {"genericperson", {}}},
	{47, {"tokamak", {}}},
	{48, {"wrecker", {}}},
	{49, {"rutger", {}}},
	{50, {"synth", {}}},
	{51, {"thumper", {}}},
	{52, {"mirage", {}}},
	{53, {"slork", {}}},
	{54, {"cadence", {}}},
	{55, {"targetdummy", {}}},
	{56, {"bomber", {}}},
	{57, {"shieldguy", {}}},
	{58, {"viper", "kali"}},
	{59, {"vandal", {}}},
	{60, {"magician", {}}},
	{61, {"trapper", {}}},
	{62, {"operative", {}}},
	{63, {"vampirebat", {}}},
	{64, {"drifter", {}}},
	{65, {"priest", {}}},
	{66, {"frank", {}}},
	{67, {"bookworm", {}}},
	{68, {"boho", {}}},
	{69, {"doorman", {}}},
	{70, {"skyrunner", {}}},
	{71, {"swan", {}}},
	{72, {"punkgoat", {}}},
	{73, {"druid", {}}},
	{74, {"graf", {}}},
	{75, {"fortuna", {}}},
	{76, {"necro", {}}},
	{77, {"fencer", {}}},
	{78, {"airheart", {}}},
	{79, {"familiar", {}}},
	{80, {"werewolf", {}}},
	{81, {"unicorn", {}}},
	{82, {"opera", {}}},
};

inline const char* HeroSlugFromID(uint32_t nHeroID)
{
	auto it = HERO_MAP.find(nHeroID);
	if (it != HERO_MAP.end())
		return it->second.slug;
	return {};
}

inline const char* HeroImagePathFromID(uint32_t nHeroID)
{
	thread_local char buffer[256];

	auto it = HERO_MAP.find(nHeroID);
	if (it == HERO_MAP.end())
		return {};

	auto name = it->second.imageName.value_or(it->second.slug);
	snprintf(buffer, sizeof(buffer), "panorama/images/heroes/%s_mm_psd.vtex", name);
	return buffer;
}
