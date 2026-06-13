#include "CEspSpectatorListComponent.hpp"

#include <ImGui/imgui.h>

#include <DeadLock/SDK/SDK.hpp>
#include <DeadLock/SDK/Types/CEntityData.hpp>
#include <DeadLock/SDK/Types/C_CitadelGameRules.hpp>

#include <GameClient/CEntityCache/CEntityCache.hpp>
#include <GameClient/CL_CitadelPlayerController.hpp>

#include <AndromedaClient/CAndromedaGUI.hpp>
#include <AndromedaClient/GUI/CAndromedaMenu.hpp>
#include <AndromedaClient/Fonts/FontAwesomeIcon.hpp>
#include <AndromedaClient/Features/CMisc/CMisc.hpp>

// ── Load / Save ───────────────────────────────────────────────────────────────

void CEspSpectatorListComponent::Load(const rapidjson::Value& v)
{
	if (!v.HasMember(XorStr("SpectatorList")))
		return;
	GetBoolJson(v[XorStr("SpectatorList")], XorStr("Enabled"), Enabled);
}

void CEspSpectatorListComponent::Save(JsonWriter& w)
{
	w.String(XorStr("SpectatorList"));
	w.StartObject();
	{
		AddBoolJson(w, XorStr("Enabled"), Enabled);
	}
	w.EndObject();
}

// ── Menu ──────────────────────────────────────────────────────────────────────

void CEspSpectatorListComponent::RenderOtherMenu()
{
	GetAndromedaMenu()->RenderCheckBox(XorStr("Spectator List"), XorStr("##Visual.SpectatorList"), Enabled);
}

// ── Helpers ───────────────────────────────────────────────────────────────────

static int GetObserverTargetIndex(C_BasePlayerPawn* pPawn)
{
	if (!pPawn)
		return -1;
	auto* pObs = pPawn->m_pObserverServices();
	if (!pObs)
		return -1;
	CHandle h = pObs->m_hObserverTarget();
	return h.IsValid() ? h.GetEntryIndex() : -1;
}

// ── GetSpectatorList ──────────────────────────────────────────────────────────

std::vector<SpectatorEntry_t> CEspSpectatorListComponent::GetSpectatorList()
{
	std::vector<SpectatorEntry_t> out;

	auto* pLocal = GetCL_CitadelPlayerController()->GetLocal();
	if (!pLocal)
		return out;

	auto* pEntityCache = GetEntityCache();
	if (!pEntityCache)
		return out;

	CHandle hLocalHeroPawn = pLocal->m_hHeroPawn();
	CHandle hLocalBasePawn = pLocal->m_hPawn();

	int localHeroIdx = hLocalHeroPawn.IsValid() ? hLocalHeroPawn.GetEntryIndex() : -1;
	int localBaseIdx = hLocalBasePawn.IsValid() ? hLocalBasePawn.GetEntryIndex() : -1;
	int localObsIdx = GetObserverTargetIndex(hLocalBasePawn.Get<C_BasePlayerPawn>());

	std::scoped_lock lock(pEntityCache->GetLock());

	for (const auto& cached : pEntityCache->GetEntitiesByType(CachedEntity_t::CITADEL_PLAYER_CONTROLLER))
	{
		auto* pCtrl = cached.m_Handle.Get<CCitadelPlayerController>();
		if (!pCtrl || pCtrl == pLocal)
			continue;

		int obsIdx = GetObserverTargetIndex(pCtrl->m_hPawn().Get<C_BasePlayerPawn>());
		if (obsIdx < 0)
			continue;

		const bool bWatchingHero = obsIdx == localHeroIdx;
		const bool bWatchingBase = obsIdx == localBaseIdx;
		const bool bWatchingSameTarget = localObsIdx >= 0 && obsIdx == localObsIdx;

		if (!bWatchingHero && !bWatchingBase && !bWatchingSameTarget)
			continue;

		const char* sz = pCtrl->m_iszPlayerName();
		SpectatorEntry_t e;
		e.name = (sz && strnlen(sz, 1) > 0) ? std::string(sz, strnlen(sz, 128)) : XorStr("Unknown");
		e.isEnemy = pCtrl->m_iTeamNum() != pLocal->m_iTeamNum();
		out.push_back(std::move(e));
	}
	return out;
}

// ── OnRender ──────────────────────────────────────────────────────────────────

void CEspSpectatorListComponent::OnRender()
{
	if (!Enabled || !ImGui::GetCurrentContext())
		return;

	float flNow = 0.f;
	if (auto* r = SDK::Pointers::GameRules())
		flNow = r->GetMatchTime();
	if (fabsf(flNow - m_flLastUpdate) >= k_flUpdateInterval)
	{
		m_flLastUpdate = flNow;
		m_Cached = GetSpectatorList();
	}

	const bool bMenuOpen = GetAndromedaGUI()->IsVisible();
	if (m_Cached.empty() && !bMenuOpen)
		return;

	const float fMenuAlpha = static_cast<float>(GetMisc()->MenuAlpha) / 255.f;
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, fMenuAlpha);
	ImGui::PushStyleColor(ImGuiCol_WindowShadow, ImVec4(0.f, 0.f, 0.f, 0.f));
	const float fSavedShadow = ImGui::GetStyle().WindowShadowSize;
	ImGui::GetStyle().WindowShadowSize = 0.f;

	ImGui::SetNextWindowSizeConstraints(ImVec2(220.f, 0.f), ImVec2(220.f, FLT_MAX));
	constexpr ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_AlwaysAutoResize;

	if (ImGui::Begin(XorStr("##SpectatorList"), nullptr, flags))
	{
		const float fTextH = ImGui::GetTextLineHeight();
		ImGui::PushFont(GetAndromedaGUI()->m_pFontAwesomeIcons);
		ImGui::SetWindowFontScale(0.5f);

		const float fIconH = ImGui::GetTextLineHeight();
		const float fIconOffset = (fTextH - fIconH) * 0.5f;
		const float fTextOffset = (fIconH - fTextH) * 0.5f;
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + fIconOffset);
		ImGui::TextUnformatted(reinterpret_cast<const char*>(ICON_FA_EYE));

		ImGui::SetWindowFontScale(1.f);
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + fTextOffset);
		ImGui::TextUnformatted(XorStr("Spectator List"));
		ImGui::Separator();

		for (const auto& e : m_Cached)
		{
			ImVec4 col = e.isEnemy ? ImVec4(1.f, 0.4f, 0.4f, 1.f) : ImVec4(0.4f, 1.f, 0.4f, 1.f);
			ImGui::PushStyleColor(ImGuiCol_Text, col);
			ImGui::TextUnformatted(e.name.c_str());
			ImGui::PopStyleColor();
		}
	}
	ImGui::End();

	ImGui::GetStyle().WindowShadowSize = fSavedShadow;
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}