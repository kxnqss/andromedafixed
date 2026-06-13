#include "CMisc.hpp"

void CMisc::Load(const rapidjson::Value& doc)
{
	if (!doc.IsObject() || !doc.HasMember(XorStr("Misc")))
		return;

	const auto& v = doc[XorStr("Misc")];

	for (auto* comp : m_Components)
		comp->Load(v);

	GetIntJson(v, XorStr("MenuAlpha"), MenuAlpha, 100, 255);
	GetIntJson(v, XorStr("MenuStyle"), MenuStyle, 0, 2);
	GetBoolJson(v, XorStr("MenuSounds"), MenuSounds);
}

void CMisc::Save(JsonWriter& w)
{
	w.String(XorStr("Misc"));
	w.StartObject();
	{
		for (auto* comp : m_Components)
			comp->Save(w);

		AddIntJson(w, XorStr("MenuAlpha"), MenuAlpha);
		AddIntJson(w, XorStr("MenuStyle"), MenuStyle);
		AddBoolJson(w, XorStr("MenuSounds"), MenuSounds);
	}
	w.EndObject();
}