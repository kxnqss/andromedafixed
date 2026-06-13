#include "CHornetComponent.hpp"

#include <Common/Common.hpp>

void CHornetComponent::Load(const rapidjson::Value& doc)
{
	if (!doc.IsObject() || !doc.HasMember(XorStr("Hornet")))
		return;

	const auto& value = doc[XorStr("Hornet")];

	if (value.HasMember("Snipe"))
	{
		const auto& snipeV = value[XorStr("Snipe")];

		GetBoolJson(snipeV, XorStr("Active"), SnipeActive);
		GetFloatJson(snipeV, XorStr("Fov"), SnipeFov, 10.f, 500.f);
		GetBoolJson(snipeV, XorStr("ShowFov"), SnipeShowFov);
		GetColorJson(snipeV, XorStr("FovColor"), SnipeFovColor);
		GetFloatJson(snipeV, XorStr("SmoothX"), SnipeSmoothX, 1.f, 30.f);
		GetFloatJson(snipeV, XorStr("SmoothY"), SnipeSmoothY, 1.f, 30.f);
	}
}

void CHornetComponent::Save(JsonWriter& w)
{
	w.String(XorStr("Hornet"));
	w.StartObject();
	{
		w.String(XorStr("Snipe"));
		w.StartObject();
		{
			AddBoolJson(w, XorStr("Active"), SnipeActive);
			AddFloatJson(w, XorStr("Fov"), SnipeFov);
			AddBoolJson(w, XorStr("ShowFov"), SnipeShowFov);
			AddColorJson(w, XorStr("FovColor"), SnipeFovColor);
			AddFloatJson(w, XorStr("SmoothX"), SnipeSmoothX);
			AddFloatJson(w, XorStr("SmoothY"), SnipeSmoothY);
		}
		w.EndObject();
	}
	w.EndObject();
}