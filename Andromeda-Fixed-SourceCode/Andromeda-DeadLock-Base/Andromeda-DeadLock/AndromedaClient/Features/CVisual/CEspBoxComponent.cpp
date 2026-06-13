#include "CEspBoxComponent.hpp"

#include <algorithm>

#include <DeadLock/SDK/Types/CEntityData.hpp>
#include <DeadLock/SDK/Math/Math.hpp>

#include <AndromedaClient/GUI/CAndromedaMenu.hpp>
#include <AndromedaClient/GUI/CMenuWidgets.hpp>
#include <AndromedaClient/Fonts/CFontManager.hpp>
#include <AndromedaClient/Render/CRenderStackSystem.hpp>

// ── JSON helpers ──────────────────────────────────────────────────────────────

static void LoadBoxSettings(const rapidjson::Value& v, BoxEspSettings& s)
{
	if (!v.IsObject())
		return;
	GetBoolJson(v, XorStr("Enabled"), s.Enabled);
	GetColorJson(v, XorStr("Color"), s.Color);
	GetBoolJson(v, XorStr("HealthBar"), s.HealthBar);
	GetColorJson(v, XorStr("HpFull"), s.HpFull);
	GetColorJson(v, XorStr("HpLow"), s.HpLow);
	GetIntJson(v, XorStr("HpWidth"), s.HpWidth, 4, 8);
}

static void SaveBoxSettings(JsonWriter& w, const BoxEspSettings& s)
{
	AddBoolJson(w, XorStr("Enabled"), s.Enabled);
	AddColorJson(w, XorStr("Color"), s.Color);
	AddBoolJson(w, XorStr("HealthBar"), s.HealthBar);
	AddColorJson(w, XorStr("HpFull"), s.HpFull);
	AddColorJson(w, XorStr("HpLow"), s.HpLow);
	AddIntJson(w, XorStr("HpWidth"), s.HpWidth);
}

// ── Load / Save ───────────────────────────────────────────────────────────────

void CEspBoxComponent::Load(const rapidjson::Value& v)
{
	if (!v.HasMember(XorStr("Box")))
		return;
	const auto& box = v[XorStr("Box")];

	if (box.HasMember(XorStr("Enemy")))
		LoadBoxSettings(box[XorStr("Enemy")], Enemy);
	if (box.HasMember(XorStr("Team")))
		LoadBoxSettings(box[XorStr("Team")], Team);

	if (box.HasMember(XorStr("Trooper")))
	{
		const auto& t = box[XorStr("Trooper")];
		GetBoolJson(t, XorStr("Enabled"), Trooper.Enabled);
		GetColorJson(t, XorStr("Color"), Trooper.Color);
	}
}

void CEspBoxComponent::Save(JsonWriter& w)
{
	w.String(XorStr("Box"));
	w.StartObject();
	{
		w.String(XorStr("Enemy"));
		w.StartObject();
		SaveBoxSettings(w, Enemy);
		w.EndObject();
		w.String(XorStr("Team"));
		w.StartObject();
		SaveBoxSettings(w, Team);
		w.EndObject();
		w.String(XorStr("Trooper"));
		w.StartObject();
		{
			AddBoolJson(w, XorStr("Enabled"), Trooper.Enabled);
			AddColorJson(w, XorStr("Color"), Trooper.Color);
		}
		w.EndObject();
	}
	w.EndObject();
}

// ── Menu ──────────────────────────────────────────────────────────────────────

static void RenderBoxSide(BoxEspSettings& s, bool bEnemy)
{
	const char* popupId = bEnemy ? XorStr("##HpPopupEnemy") : XorStr("##HpPopupTeam");
	const ImVec2 arrowMax = CMenuWidgets::RenderExpandableToggle({
		XorStr("Health"),
		bEnemy ? XorStr("##Visual.HpEnemy") : XorStr("##Visual.HpTeam"),
		popupId,
		&s.HealthBar
	});

	if (CMenuWidgets::BeginAnimatedPopup(popupId, arrowMax))
	{
		constexpr float kPad = 90.f;

		CMenuWidgets::RenderColor(XorStr("HP Full"), s.HpFull, bEnemy ? 3 : 8);
		CMenuWidgets::RenderColor(XorStr("HP Low"), s.HpLow, bEnemy ? 4 : 9);

		{
			const float lp = ImGui::GetStyle().FramePadding.x;
			ImGui::AlignTextToFramePadding();
			ImGui::Text(XorStr("Width"));
			ImGui::SameLine(kPad);
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - lp);
			ImGui::SliderInt(
				bEnemy ? XorStr("##Visual.HpWidthEnemy") : XorStr("##Visual.HpWidthTeam"),
				&s.HpWidth, 4, 8, "%d", ImGuiSliderFlags_AlwaysClamp
				);
			ImGui::PopItemWidth();
		}

		ImGui::EndPopup();
	}

	ImGui::Separator();
	CMenuWidgets::RenderColorToggle({
		XorStr("Esp Box"),
		s.Color,
		bEnemy ? 1 : 5,
		bEnemy ? XorStr("##Visual.EspEnemyBox") : XorStr("##Visual.EspTeamBox"),
		&s.Enabled
	});
}

void CEspBoxComponent::RenderEnemyMenu()
{
	RenderBoxSide(Enemy, true);
}

void CEspBoxComponent::RenderTeamMenu()
{
	RenderBoxSide(Team, false);
}

void CEspBoxComponent::RenderOtherMenu()
{
	const ImVec2 arrowMax = CMenuWidgets::RenderExpandableToggle({
		XorStr("Trooper Box"),
		XorStr("##Visual.Trooper"),
		XorStr("##TrooperPopup"),
		&Trooper.Enabled
	});

	if (CMenuWidgets::BeginAnimatedPopup(XorStr("##TrooperPopup"), arrowMax))
	{
		CMenuWidgets::RenderColor(XorStr("Color"), Trooper.Color, 7);
		ImGui::EndPopup();
	}
}

// ── Rendering ─────────────────────────────────────────────────────────────────

void CEspBoxComponent::OnRenderPlayer(const EspPlayerContext& ctx)
{
	if (!ctx.pPawn || !ctx.bAlive || !ctx.boxValid)
		return;

	const BoxEspSettings& s = ctx.bTeam ? Team : Enemy;
	const ImVec2 bMin = {ctx.box.x, ctx.box.y};
	const ImVec2 bMax = {ctx.box.w, ctx.box.h};

	if (s.Enabled)
	{
		const ImColor col(s.Color[0], s.Color[1], s.Color[2], 1.f);
		GetRenderStackSystem()->DrawOutlineCoalBox(bMin, bMax, col);
	}

	if (!s.HealthBar)
		return;

	auto& data = ctx.pCtrl->m_PlayerDataGlobal();
	const int hp = data.m_iHealth();
	const int hpMax = data.m_iHealthMax();

	if (hpMax <= 0)
		return;

	const float pct = std::clamp(static_cast<float>(hp) / static_cast<float>(hpMax), 0.f, 1.f);
	const float barW = static_cast<float>(s.HpWidth);

	constexpr float kGap = 3.f;
	const float barX1 = ctx.box.x - kGap - barW;
	const float barX2 = ctx.box.x - kGap;
	const float barH = ctx.box.h - ctx.box.y;

	GetRenderStackSystem()->DrawFillBox({barX1, ctx.box.y}, {barX2, ctx.box.h}, ImColor(0.f, 0.f, 0.f, 0.6f));

	if (pct > 0.f)
	{
		const float* cFull = s.HpFull;
		const float* cLow = s.HpLow;

		const float r = cLow[0] + (cFull[0] - cLow[0]) * pct;
		const float g = cLow[1] + (cFull[1] - cLow[1]) * pct;
		const float b = cLow[2] + (cFull[2] - cLow[2]) * pct;

		const ImU32 colTop = ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, 1.f));
		const ImU32 colBot = ImGui::ColorConvertFloat4ToU32(ImVec4(cLow[0], cLow[1], cLow[2], 1.f));

		GetRenderStackSystem()->DrawRectFilledMultiColor(
			{barX1, ctx.box.h - barH * pct}, {barX2, ctx.box.h}, colTop, colTop, colBot, colBot);
	}

	GetRenderStackSystem()->DrawBox({barX1, ctx.box.y}, {barX2, ctx.box.h}, ImColor(0.f, 0.f, 0.f, 0.8f));
}

void CEspBoxComponent::OnRenderTrooper(C_NPC_Trooper* pTrooper, const Rect_t& box)
{
	if (!Trooper.Enabled)
		return;

	if (pTrooper->m_NPCState() > NPC_STATE_COMBAT)
		return;

	const ImColor color(Trooper.Color[0], Trooper.Color[1], Trooper.Color[2]);
	GetRenderStackSystem()->DrawBox({box.x, box.y}, {box.w, box.h}, color);

	Vector3 rootPos;
	if (!pTrooper->m_pGameSceneNode()->GetBonePosition(pTrooper->GetBoneIdByName(XorStr("root_motion")), rootPos))
		return;

	ImVec2 screen;
	rootPos.m_z += 30.f;
	if (!Math::WorldToScreen(rootPos, screen))
		return;

	GetRenderStackSystem()->DrawString(
		&GetFontManager()->m_VerdanaFont,
		static_cast<int>(screen.x), static_cast<int>(screen.y),
		FW1_CENTER, color, "X"
		);
}