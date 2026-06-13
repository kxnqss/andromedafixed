#include "CEspChamsComponent.hpp"

#include <AndromedaClient/GUI/CAndromedaMenu.hpp>
#include <AndromedaClient/GUI/CMenuWidgets.hpp>

// ── Load / Save ───────────────────────────────────────────────────────────────

static void LoadChamsSettings(const rapidjson::Value& v, const char* key, ChamsSettings& s)
{
	if (!v.HasMember(key))
		return;
	const auto& o = v[key];
	GetBoolJson(o, XorStr("Enabled"), s.Enabled);
}

static void SaveChamsSettings(JsonWriter& w, const char* key, const ChamsSettings& s)
{
	w.String(key);
	w.StartObject();
	{
		AddBoolJson(w, XorStr("Enabled"), s.Enabled);
	}
	w.EndObject();
}

void CEspChamsComponent::Load(const rapidjson::Value& v)
{
	if (!v.HasMember(XorStr("Chams")))
		return;
	const auto& o = v[XorStr("Chams")];
	LoadChamsSettings(o, XorStr("Enemy"), Enemy);
	LoadChamsSettings(o, XorStr("Team"), Team);
}

void CEspChamsComponent::Save(JsonWriter& w)
{
	w.String(XorStr("Chams"));
	w.StartObject();
	{
		SaveChamsSettings(w, XorStr("Enemy"), Enemy);
		SaveChamsSettings(w, XorStr("Team"), Team);
	}
	w.EndObject();
}

// ── Menu ──────────────────────────────────────────────────────────────────────

static void RenderChamsSide(ChamsSettings& s, const char* toggleId)
{
	const float h = ImGui::GetFrameHeight();
	const float lp = ImGui::GetStyle().FramePadding.x;

	ImGui::AlignTextToFramePadding();
	ImGui::Text(XorStr("Chams"));
	ImGui::SameLine(ImGui::CalcTextSize(XorStr("Chams")).x + 10.f);
	ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - h - lp, 0.f));
	ImGui::SameLine(0.f, 0.f);
	ImGui::Checkbox(toggleId, &s.Enabled);
}

void CEspChamsComponent::RenderEnemyMenu()
{
	RenderChamsSide(Enemy, XorStr("##Visual.ChamsEnemy"));
}

void CEspChamsComponent::RenderTeamMenu()
{
	RenderChamsSide(Team, XorStr("##Visual.ChamsTeam"));
}