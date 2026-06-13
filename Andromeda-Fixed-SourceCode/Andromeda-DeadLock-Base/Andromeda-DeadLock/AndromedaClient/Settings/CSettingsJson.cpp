#include "CSettingsJson.hpp"
#include "DllLauncher.hpp"

#include <filesystem>
#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <AndromedaClient/Features/FeatureRegistry.hpp>

static CSettingsJson g_CSettingsJson{};

// ── Registry helpers ──────────────────────────────────────────────────────────

static constexpr auto kRegKey = "Software\\Andromeda";
static constexpr auto kRegValue = "LastConfig";

static void RegSaveLastConfig(const std::string& name)
{
	HKEY hKey = nullptr;
	if (RegCreateKeyExA(HKEY_CURRENT_USER, kRegKey, 0, nullptr,
	                    REG_OPTION_NON_VOLATILE, KEY_SET_VALUE,
	                    nullptr, &hKey, nullptr) != ERROR_SUCCESS)
		return;

	RegSetValueExA(hKey, kRegValue, 0, REG_SZ,
	               reinterpret_cast<const BYTE*>(name.c_str()),
	               static_cast<DWORD>(name.size() + 1));
	RegCloseKey(hKey);
}

static std::string RegLoadLastConfig()
{
	HKEY hKey = nullptr;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, kRegKey, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
		return {};

	char buf[256] = {};
	DWORD size = sizeof(buf);
	DWORD type = REG_SZ;
	const bool ok = RegQueryValueExA(hKey, kRegValue, nullptr, &type,
	                                 reinterpret_cast<BYTE*>(buf), &size) == ERROR_SUCCESS;
	RegCloseKey(hKey);
	return ok ? std::string(buf) : std::string{};
}

void CSettingsJson::LoadConfig(const std::string& JsonFile)
{
	auto ConfigLoadedIndex = 0u;
	const auto ConfigFilePath = GetDllDir() + JsonFile;

	for (const auto& Config : m_vecConfigList)
	{
		if (Config == JsonFile)
		{
			m_nConfigLoadedIndex = ConfigLoadedIndex;
			break;
		}
		ConfigLoadedIndex++;
	}

	std::ifstream ConfigFile(ConfigFilePath);

	rapidjson::IStreamWrapper StreamWrapper(ConfigFile);
	rapidjson::Document DocumentConfig;

	DocumentConfig.ParseStream(StreamWrapper);

	if (!DocumentConfig.HasParseError())
	{
		for (auto* f : FeatureRegistry::GetRootFeatures())
			f->Load(DocumentConfig);

		RegSaveLastConfig(JsonFile);
	}
	else
	{
		DEV_LOG("[error] LoadConfig: %s -> %s , %i\n", ConfigFilePath.c_str(),
		        rapidjson::GetParseError_En(DocumentConfig.GetParseError()),
		        DocumentConfig.GetErrorOffset());
	}

	DocumentConfig.Clear();
	ConfigFile.close();
}

void CSettingsJson::SaveConfig(const std::string& JsonFile)
{
	const auto ConfigFilePath = GetDllDir() + JsonFile;

	std::ofstream ConfigFile(ConfigFilePath);

	rapidjson::OStreamWrapper StreamWrapper(ConfigFile);
	rapidjson::PrettyWriter ConfigWriter(StreamWrapper);

	ConfigWriter.SetIndent('\t', 1);
	ConfigWriter.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
	ConfigWriter.SetMaxDecimalPlaces(2);

	ConfigWriter.StartObject();
	for (auto* f : FeatureRegistry::GetRootFeatures())
		f->Save(ConfigWriter);

	ConfigWriter.EndObject();
	ConfigFile.close();
}

void CSettingsJson::DeleteConfig(const std::string& JsonFile)
{
	const auto ConfigFilePath = GetDllDir() + JsonFile;
	DeleteFileA(ConfigFilePath.c_str());
}

void CSettingsJson::UpdateConfigList()
{
	m_vecConfigList.clear();

	for (const auto& Entry : std::filesystem::directory_iterator(GetDllDir().c_str()))
	{
		if (Entry.is_regular_file())
		{
			if (Entry.path().extension().string() == XorStr(".json"))
				m_vecConfigList.emplace_back(Entry.path().filename().string());
		}
	}
}

std::string CSettingsJson::GetLastConfig()
{
	return RegLoadLastConfig();
}

CSettingsJson* GetSettingsJson()
{
	return &g_CSettingsJson;
}