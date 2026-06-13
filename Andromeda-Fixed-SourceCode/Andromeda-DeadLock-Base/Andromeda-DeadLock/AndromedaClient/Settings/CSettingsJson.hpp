#pragma once

#include <string>
#include <vector>

class CSettingsJson final
{
public:
	using VecConfigList_t = std::vector<std::string>;

public:
	void LoadConfig(const std::string& JsonFile);
	void SaveConfig(const std::string& JsonFile);
	void DeleteConfig(const std::string& JsonFile);
	void UpdateConfigList();
	std::string GetLastConfig();

public:
	inline VecConfigList_t& GetConfigList()
	{
		return m_vecConfigList;
	}

	inline uint32_t GetConfigLoadedIndex()
	{
		return m_nConfigLoadedIndex;
	}

private:
	VecConfigList_t m_vecConfigList;
	uint32_t m_nConfigLoadedIndex = UINT_MAX;
};

CSettingsJson* GetSettingsJson();