#include "CEspNameComponent.hpp"
#include "CEspHelpers.hpp"

#include <DeadLock/SDK/Types/CEntityData.hpp>

#include <AndromedaClient/GUI/CAndromedaMenu.hpp>
#include <AndromedaClient/GUI/CMenuWidgets.hpp>
#include <AndromedaClient/Fonts/CFontManager.hpp>
#include <AndromedaClient/Render/CRenderStackSystem.hpp>

// ── FW1 alignment helper (local) ──────────────────────────────────────────────

static int FW1FlagsForPosition(int position)
{
	switch (static_cast<EEspPosition>(position))
	{
		case EEspPosition::TopRight:
		case EEspPosition::BottomRight:
			return FW1_RIGHT;
		case EEspPosition::TopMiddle:
		case EEspPosition::BottomMiddle:
			return FW1_CENTER;
		default:
			return FW1_LEFT; // TopLeft, BottomLeft
	}
}

// ── JSON helpers ──────────────────────────────────────────────────────────────

static void LoadNameSettings(const rapidjson::Value& v, NameEspSettings& s)
{
	if (!v.IsObject())
		return;
	GetBoolJson(v, XorStr("Enabled"), s.Enabled);
	GetColorJson(v, XorStr("Color"), s.Color);
	GetFloatJson(v, XorStr("FontSize"), s.FontSize, 6.f, 64.f);
	GetIntJson(v, XorStr("Position"), s.Position, 0, 3);
	GetFloatJson(v, XorStr("OffsetX"), s.OffsetX, -50.f, 50.f);
	GetFloatJson(v, XorStr("OffsetY"), s.OffsetY, -50.f, 50.f);
}

static void SaveNameSettings(JsonWriter& w, const NameEspSettings& s)
{
	AddBoolJson(w, XorStr("Enabled"), s.Enabled);
	AddColorJson(w, XorStr("Color"), s.Color);
	AddFloatJson(w, XorStr("FontSize"), s.FontSize);
	AddIntJson(w, XorStr("Position"), s.Position);
	AddFloatJson(w, XorStr("OffsetX"), s.OffsetX);
	AddFloatJson(w, XorStr("OffsetY"), s.OffsetY);
}

// ── Load / Save ───────────────────────────────────────────────────────────────

void CEspNameComponent::Load(const rapidjson::Value& v)
{
	if (!v.HasMember(XorStr("Name")))
		return;
	const auto& name = v[XorStr("Name")];

	if (name.HasMember(XorStr("Enemy")))
		LoadNameSettings(name[XorStr("Enemy")], Enemy);
	if (name.HasMember(XorStr("Team")))
		LoadNameSettings(name[XorStr("Team")], Team);
}

void CEspNameComponent::Save(JsonWriter& w)
{
	w.String(XorStr("Name"));
	w.StartObject();
	{
		w.String(XorStr("Enemy"));
		w.StartObject();
		SaveNameSettings(w, Enemy);
		w.EndObject();
		w.String(XorStr("Team"));
		w.StartObject();
		SaveNameSettings(w, Team);
		w.EndObject();
	}
	w.EndObject();
}

// ── Menu ──────────────────────────────────────────────────────────────────────

void CEspNameComponent::RenderSide(NameEspSettings& s, bool bEnemy)
{
	auto* menu = GetAndromedaMenu();

	const char* popupId = bEnemy ? XorStr("##NamePopupEnemy") : XorStr("##NamePopupTeam");

	const ImVec2 arrowMax = CMenuWidgets::RenderExpandableToggle({
		XorStr("Name"),
		bEnemy ? XorStr("##Visual.NameEnemy") : XorStr("##Visual.NameTeam"),
		popupId,
		&s.Enabled
	});

	if (CMenuWidgets::BeginAnimatedPopup(popupId, arrowMax))
	{
		constexpr float kPad = 90.f;

		CMenuWidgets::RenderColor(XorStr("Color"), s.Color, bEnemy ? 10 : 11);

		menu->RenderSliderFloat({
			XorStr("Font Size"),
			bEnemy ? XorStr("##Visual.NameEnemyFontSize") : XorStr("##Visual.NameTeamFontSize"),
			s.FontSize, 6.f, 32.f, kPad
		});

		if (menu->RenderComboBox({
			XorStr("Position"),
			bEnemy ? XorStr("##Visual.NameEnemyPos") : XorStr("##Visual.NameTeamPos"),
			s.Position, kEspPositionItems, kEspPositionCount, kPad
		}))
		{
			s.OffsetX = 0.f;
			s.OffsetY = 0.f;
		}

		menu->RenderSliderFloat({
			XorStr("Offset X"),
			bEnemy ? XorStr("##Visual.NameEnemyOffX") : XorStr("##Visual.NameTeamOffX"),
			s.OffsetX, -50.f, 50.f, kPad
		});

		menu->RenderSliderFloat({
			XorStr("Offset Y"),
			bEnemy ? XorStr("##Visual.NameEnemyOffY") : XorStr("##Visual.NameTeamOffY"),
			s.OffsetY, -50.f, 50.f, kPad
		});

		ImGui::EndPopup();
	}
}

void CEspNameComponent::RenderEnemyMenu()
{
	RenderSide(Enemy, true);
}

void CEspNameComponent::RenderTeamMenu()
{
	RenderSide(Team, false);
}

// ── Rendering ─────────────────────────────────────────────────────────────────

void CEspNameComponent::OnRenderPlayer(const EspPlayerContext& ctx)
{
	const NameEspSettings& s = ctx.bTeam ? Team : Enemy;
	if (!s.Enabled || !ctx.boxValid)
		return;

	const char* szName = ctx.pCtrl->m_iszPlayerName();
	if (!szName || szName[0] == '\0')
		return;

	const ImVec2 pos = ComputeEspPos(ctx.box, s.Position, s.OffsetX, s.OffsetY);
	const int fw1Flag = FW1FlagsForPosition(s.Position);
	const ImColor col(s.Color[0], s.Color[1], s.Color[2], 1.f);

	GetRenderStackSystem()->DrawStringSize(
		&GetFontManager()->m_VerdanaFont,
		static_cast<int>(pos.x),
		static_cast<int>(pos.y),
		fw1Flag,
		col,
		s.FontSize,
		"%s", szName
		);
}