#include "CShivComponent.hpp"

#include <Common/Common.hpp>

void CShivComponent::Load(const rapidjson::Value& doc)
{
	if (!doc.IsObject() || !doc.HasMember(XorStr("Shiv")))
		return;

	const auto& value = doc[XorStr("Shiv")];
	if (!value.HasMember(XorStr("Knives")))
		return;

	const auto& v = value[XorStr("Knives")];

	GetBoolJson(v, XorStr("Active"), KnivesActive);
	GetFloatJson(v, XorStr("Fov"), KnivesFov, 10.f, 500.f);
	GetBoolJson(v, XorStr("ShowFov"), KnivesShowFov);
	GetColorJson(v, XorStr("FovColor"), KnivesFovColor);
	GetFloatJson(v, XorStr("SmoothX"), KnivesSmoothX, 1.f, 30.f);
	GetFloatJson(v, XorStr("SmoothY"), KnivesSmoothY, 1.f, 30.f);
}

void CShivComponent::Save(JsonWriter& w)
{
	w.String(XorStr("Shiv"));
	w.StartObject();
	{
		w.String(XorStr("Knives"));
		w.StartObject();
		{
			AddBoolJson(w, XorStr("Active"), KnivesActive);
			AddFloatJson(w, XorStr("Fov"), KnivesFov);
			AddBoolJson(w, XorStr("ShowFov"), KnivesShowFov);
			AddColorJson(w, XorStr("FovColor"), KnivesFovColor);
			AddFloatJson(w, XorStr("SmoothX"), KnivesSmoothX);
			AddFloatJson(w, XorStr("SmoothY"), KnivesSmoothY);
		}
		w.EndObject();
	}
	w.EndObject();
}