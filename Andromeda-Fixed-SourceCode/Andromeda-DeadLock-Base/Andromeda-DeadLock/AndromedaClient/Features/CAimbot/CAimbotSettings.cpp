#include "CAimbot.hpp"

void CAimbot::Load(const rapidjson::Value& doc)
{
	if (!doc.IsObject() || !doc.HasMember(XorStr("Aimbot")))
		return;
	const auto& v = doc[XorStr("Aimbot")];

	GetBoolJson(v, XorStr("Active"), Active);
	GetIntJson(v, XorStr("AimKey"), AimKey, 0, 0xFE);
	GetFloatJson(v, XorStr("SensitivityX"), SensitivityX, 1.f, 30.f);
	GetFloatJson(v, XorStr("SensitivityY"), SensitivityY, 1.f, 30.f);
	GetFloatJson(v, XorStr("InertiaScale"), InertiaScale, 0.f, 2.f);
	GetFloatJson(v, XorStr("DriftScale"), DriftScale, 0.f, 2.f);
	GetFloatJson(v, XorStr("Fov"), Fov, 10.f, 500.f);
	GetBoolJson(v, XorStr("ShowFov"), ShowFov);
	GetColorJson(v, XorStr("FovColor"), FovColor);
	GetIntJson(v, XorStr("BonesMask"), BonesMask, 0, 0x1F);
	GetBoolJson(v, XorStr("AntiFrog"), AntiFrog);
	GetFloatJson(v, XorStr("HsThreshold"), HsThreshold, 1.f, 99.f);
}

void CAimbot::Save(JsonWriter& w)
{
	w.String(XorStr("Aimbot"));
	w.StartObject();
	{
		AddBoolJson(w, XorStr("Active"), Active);
		AddIntJson(w, XorStr("AimKey"), AimKey);
		AddFloatJson(w, XorStr("SensitivityX"), SensitivityX);
		AddFloatJson(w, XorStr("SensitivityY"), SensitivityY);
		AddFloatJson(w, XorStr("InertiaScale"), InertiaScale);
		AddFloatJson(w, XorStr("DriftScale"), DriftScale);
		AddFloatJson(w, XorStr("Fov"), Fov);
		AddBoolJson(w, XorStr("ShowFov"), ShowFov);
		AddColorJson(w, XorStr("FovColor"), FovColor);
		AddIntJson(w, XorStr("BonesMask"), BonesMask);
		AddBoolJson(w, XorStr("AntiFrog"), AntiFrog);
		AddFloatJson(w, XorStr("HsThreshold"), HsThreshold);
	}
	w.EndObject();
}