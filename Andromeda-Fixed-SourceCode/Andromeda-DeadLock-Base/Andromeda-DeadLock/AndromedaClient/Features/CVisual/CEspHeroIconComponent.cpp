#include "CEspHeroIconComponent.hpp"
#include "CEspHelpers.hpp"

#include <DeadLock/SDK/Types/CEntityData.hpp>
#include <DeadLock/SDK/Types/CHeroID.hpp>

#include <AndromedaClient/GUI/CAndromedaMenu.hpp>
#include <AndromedaClient/GUI/CMenuWidgets.hpp>
#include <AndromedaClient/Render/CRenderStackSystem.hpp>
#include <AndromedaClient/Render/CPanoramaImageCache.hpp>

// ── JSON helpers ──────────────────────────────────────────────────────────────

static void LoadIconSettings(const rapidjson::Value& v, HeroIconSettings& s)
{
	if (!v.IsObject())
		return;
	GetBoolJson(v, XorStr("Enabled"), s.Enabled);
	GetFloatJson(v, XorStr("Size"), s.Size, 4.f, 128.f);
	GetIntJson(v, XorStr("Position"), s.Position, 0, 3);
	GetFloatJson(v, XorStr("OffsetX"), s.OffsetX, -50.f, 50.f);
	GetFloatJson(v, XorStr("OffsetY"), s.OffsetY, -50.f, 50.f);
}

static void SaveIconSettings(JsonWriter& w, const HeroIconSettings& s)
{
	AddBoolJson(w, XorStr("Enabled"), s.Enabled);
	AddFloatJson(w, XorStr("Size"), s.Size);
	AddIntJson(w, XorStr("Position"), s.Position);
	AddFloatJson(w, XorStr("OffsetX"), s.OffsetX);
	AddFloatJson(w, XorStr("OffsetY"), s.OffsetY);
}

// ── Load / Save ───────────────────────────────────────────────────────────────

void CEspHeroIconComponent::Load(const rapidjson::Value& v)
{
	if (!v.HasMember(XorStr("HeroIcon")))
		return;
	const auto& icon = v[XorStr("HeroIcon")];

	if (icon.HasMember(XorStr("Enemy")))
		LoadIconSettings(icon[XorStr("Enemy")], Enemy);
	if (icon.HasMember(XorStr("Team")))
		LoadIconSettings(icon[XorStr("Team")], Team);
}

void CEspHeroIconComponent::Save(JsonWriter& w)
{
	w.String(XorStr("HeroIcon"));
	w.StartObject();
	{
		w.String(XorStr("Enemy"));
		w.StartObject();
		SaveIconSettings(w, Enemy);
		w.EndObject();
		w.String(XorStr("Team"));
		w.StartObject();
		SaveIconSettings(w, Team);
		w.EndObject();
	}
	w.EndObject();
}

// ── Menu ──────────────────────────────────────────────────────────────────────

void CEspHeroIconComponent::RenderSide(HeroIconSettings& s, bool bEnemy)
{
	auto* menu = GetAndromedaMenu();

	const char* popupId = bEnemy ? XorStr("##IconPopupEnemy") : XorStr("##IconPopupTeam");

	const ImVec2 arrowMax = CMenuWidgets::RenderExpandableToggle({
		XorStr("Icon"),
		bEnemy ? XorStr("##Visual.EspEnemyIcon") : XorStr("##Visual.EspTeamIcon"),
		popupId,
		&s.Enabled
	});

	if (CMenuWidgets::BeginAnimatedPopup(popupId, arrowMax))
	{
		constexpr float kPad = 90.f;

		menu->RenderSliderFloat({
			XorStr("Size"),
			bEnemy ? XorStr("##Visual.IconEnemySize") : XorStr("##Visual.IconTeamSize"),
			s.Size, 4.f, 64.f, kPad
		});

		if (menu->RenderComboBox({
			XorStr("Position"),
			bEnemy ? XorStr("##Visual.IconEnemyPos") : XorStr("##Visual.IconTeamPos"),
			s.Position, kEspPositionItems, kEspPositionCount, kPad
		}))
		{
			s.OffsetX = 0.f;
			s.OffsetY = 0.f;
		}

		menu->RenderSliderFloat({
			XorStr("Offset X"),
			bEnemy ? XorStr("##Visual.IconEnemyOffX") : XorStr("##Visual.IconTeamOffX"),
			s.OffsetX, -50.f, 50.f, kPad
		});

		menu->RenderSliderFloat({
			XorStr("Offset Y"),
			bEnemy ? XorStr("##Visual.IconEnemyOffY") : XorStr("##Visual.IconTeamOffY"),
			s.OffsetY, -50.f, 50.f, kPad
		});

		ImGui::EndPopup();
	}
}

void CEspHeroIconComponent::RenderEnemyMenu()
{
	RenderSide(Enemy, true);
}

void CEspHeroIconComponent::RenderTeamMenu()
{
	RenderSide(Team, false);
}

// ── Rendering ─────────────────────────────────────────────────────────────────

void CEspHeroIconComponent::OnRenderPlayer(const EspPlayerContext& ctx)
{
	const HeroIconSettings& s = ctx.bTeam ? Team : Enemy;
	if (!s.Enabled || !ctx.boxValid)
		return;

	if (!ctx.bAlive)
		return;

	auto& data = ctx.pCtrl->m_PlayerDataGlobal();

	const char* szPath = HeroImagePathFromID(data.m_nHeroID().m_Value);
	if (!szPath)
		return;

	auto* pSRV = CPanoramaImageCache::GetTexture(szPath);
	if (!pSRV)
		return;

	const ImVec2 pos = ComputeEspPos(ctx.box, s.Position, s.OffsetX, s.OffsetY);
	GetRenderStackSystem()->DrawImage(pSRV, pos, ImVec2(s.Size, s.Size));
}