#pragma once

#include <DeadLock/SDK/Math/Vector3.hpp>
#include <AndromedaClient/Settings/JsonHelpers.hpp>

class CCitadelInput;

class IMiscComponent
{
public:
	virtual ~IMiscComponent() = default;

	virtual void Load(const rapidjson::Value& v) = 0;
	virtual void Save(JsonWriter& w) = 0;
	virtual void RenderMenu() = 0;

	virtual void OnStartSound(const Vector3&, int, const char*) {}
	virtual void OnPostMove(CCitadelInput*) {}
};