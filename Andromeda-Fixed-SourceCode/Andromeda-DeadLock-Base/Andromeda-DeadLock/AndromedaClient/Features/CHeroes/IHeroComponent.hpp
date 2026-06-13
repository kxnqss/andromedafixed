#pragma once
#include <cstdint>
#include <AndromedaClient/Settings/JsonHelpers.hpp>

class CCitadelInput;
class CUserCmd;

class IHeroComponent
{
public:
	virtual ~IHeroComponent() = default;

	virtual uint32_t GetHeroID() const                       = 0;
	virtual void     Load(const rapidjson::Value& v)         = 0;
	virtual void     Save(JsonWriter& w)                     = 0;
	virtual void     RenderMenu()                            = 0;

	virtual void     OnCreateMove(CCitadelInput*, CUserCmd*) {}
	virtual void     OnPostMove(CCitadelInput*)              {}
	virtual void     OnRender()                              {}
};
