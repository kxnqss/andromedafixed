#include "CVisual.hpp"

void CVisual::Load(const rapidjson::Value& doc)
{
	if (!doc.IsObject() || !doc.HasMember(XorStr("Visuals")))
		return;

	const auto& v = doc[XorStr("Visuals")];

	GetBoolJson(v, XorStr("Active"), Active);
	GetBoolJson(v, XorStr("SoundStepEsp"), SoundStepEsp);
	GetColorJson(v, XorStr("SoundStepEspColor"), SoundStepEspColor);

	for (auto* comp : m_Components)
		comp->Load(v);
}

void CVisual::Save(JsonWriter& w)
{
	w.String(XorStr("Visuals"));
	w.StartObject();
	{
		AddBoolJson(w, XorStr("Active"), Active);
		AddBoolJson(w, XorStr("SoundStepEsp"), SoundStepEsp);
		AddColorJson(w, XorStr("SoundStepEspColor"), SoundStepEspColor);

		for (auto* comp : m_Components)
			comp->Save(w);
	}
	w.EndObject();
}