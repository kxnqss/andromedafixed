#include "CHazeComponent.hpp"

#include <Common/Common.hpp>

void CHazeComponent::Load(const rapidjson::Value& doc)
{
	if (!doc.IsObject() || !doc.HasMember(XorStr("Haze")))
		return;

	const auto& value = doc[XorStr("Haze")];
	if (!value.HasMember(XorStr("Dagger")))
		return;

	const auto& v = value[XorStr("Dagger")];

	GetBoolJson(v, XorStr("Active"), DaggerActive);
	GetFloatJson(v, XorStr("Fov"), DaggerFov, 10.f, 500.f);
	GetBoolJson(v, XorStr("ShowFov"), DaggerShowFov);
	GetColorJson(v, XorStr("FovColor"), DaggerFovColor);
	GetFloatJson(v, XorStr("SmoothX"), DaggerSmoothX, 1.f, 30.f);
	GetFloatJson(v, XorStr("SmoothY"), DaggerSmoothY, 1.f, 30.f);
}

void CHazeComponent::Save(JsonWriter& w)
{
	w.String(XorStr("Haze"));
	w.StartObject();
	{
		w.String(XorStr("Dagger"));
		w.StartObject();
		{
			AddBoolJson(w, XorStr("Active"), DaggerActive);
			AddFloatJson(w, XorStr("Fov"), DaggerFov);
			AddBoolJson(w, XorStr("ShowFov"), DaggerShowFov);
			AddColorJson(w, XorStr("FovColor"), DaggerFovColor);
			AddFloatJson(w, XorStr("SmoothX"), DaggerSmoothX);
			AddFloatJson(w, XorStr("SmoothY"), DaggerSmoothY);
		}
		w.EndObject();
	}
	w.EndObject();
}