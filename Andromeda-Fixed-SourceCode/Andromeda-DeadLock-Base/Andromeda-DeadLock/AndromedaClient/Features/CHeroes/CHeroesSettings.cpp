#include "CHeroes.hpp"

#include <Common/Common.hpp>

void CHeroes::Load(const rapidjson::Value& doc)
{
	if (!doc.IsObject() || !doc.HasMember(XorStr("Heroes")))
		return;

	const auto& v = doc[XorStr("Heroes")];

	for (auto* comp : m_Components)
		comp->Load(v);
}

void CHeroes::Save(JsonWriter& w)
{
	w.String(XorStr("Heroes"));
	w.StartObject();
	{
		for (auto* comp : m_Components)
			comp->Save(w);
	}
	w.EndObject();
}