#pragma once

#include <vector>

#include <DeadLock/SDK/Math/Vector3.hpp>
#include <AndromedaClient/Settings/JsonHelpers.hpp>

class CCitadelInput;
class CUserCmd;

class IFeature
{
public:
	virtual ~IFeature() = default;

	virtual const char* GetName() const = 0;

	virtual const char8_t* GetIcon() const
	{
		return u8"";
	}

	virtual void Load(const rapidjson::Value&) {}
	virtual void Save(JsonWriter&) {}
	virtual void RenderMenu() {}

	// Engine hook callbacks — no-op by default, override as needed.
	virtual void OnCreateMove(CCitadelInput*, CUserCmd*) {}
	virtual void OnPostMove(CCitadelInput*) {}
	virtual void OnRender() {}
	virtual void OnClientOutput() {}
	virtual void OnStartSound(const Vector3&, int, const char*) {}
};