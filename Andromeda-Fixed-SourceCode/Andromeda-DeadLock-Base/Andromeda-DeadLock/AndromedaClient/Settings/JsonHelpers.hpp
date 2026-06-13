#pragma once

#include <algorithm>
#include <ostream>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/ostreamwrapper.h>

using JsonWriter = rapidjson::PrettyWriter<rapidjson::OStreamWrapper>;

// ── Read helpers ──────────────────────────────────────────────────────────────

inline void GetBoolJson(const rapidjson::Value& v, const char* key, bool& out)
{
	if (v.IsObject() && v.HasMember(key))
	{
		const auto& val = v[key];
		if (val.IsBool()) out = val.GetBool();
	}
}

inline void GetIntJson(const rapidjson::Value& v, const char* key, int& out, int min, int max)
{
	if (v.IsObject() && v.HasMember(key))
	{
		const auto& val = v[key];
		if (val.IsInt()) out = std::clamp(val.GetInt(), min, max);
	}
}

inline void GetFloatJson(const rapidjson::Value& v, const char* key, float& out, float min, float max)
{
	if (v.IsObject() && v.HasMember(key))
	{
		const auto& val = v[key];
		if (val.IsNumber()) out = std::clamp(val.GetFloat(), min, max);
	}
}

inline void GetColorJson(const rapidjson::Value& v, const char* key, float* out)
{
	if (v.IsObject() && v.HasMember(key))
	{
		const auto& val = v[key];
		if (val.IsArray() && val.Size() == 3)
		{
			for (rapidjson::SizeType i = 0; i < 3; ++i)
				out[i] = std::clamp(val[i].GetFloat(), 0.f, 1.f);
		}
	}
}

// ── Write helpers ─────────────────────────────────────────────────────────────

inline void AddBoolJson(JsonWriter& w, const char* key, bool val)
{
	w.String(key); w.Bool(val);
}

inline void AddIntJson(JsonWriter& w, const char* key, int val)
{
	w.String(key); w.Int(val);
}

inline void AddFloatJson(JsonWriter& w, const char* key, float val)
{
	w.String(key); w.Double(static_cast<double>(val));
}

inline void AddColorJson(JsonWriter& w, const char* key, const float* val)
{
	w.String(key);
	w.StartArray();
	w.Double(static_cast<double>(val[0]));
	w.Double(static_cast<double>(val[1]));
	w.Double(static_cast<double>(val[2]));
	w.EndArray();
}
