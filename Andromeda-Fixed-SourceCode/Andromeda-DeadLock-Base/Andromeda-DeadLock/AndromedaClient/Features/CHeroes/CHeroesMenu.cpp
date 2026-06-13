#include "CHeroes.hpp"

#include <Common/Common.hpp>
#include <ImGui/imgui.h>
#include <DeadLock/SDK/Types/CHeroID.hpp>
#include <AndromedaClient/Render/CPanoramaImageCache.hpp>
#include <GameClient/CL_CitadelPlayerController.hpp>

static constexpr float kTabBarH = 64.f;
static constexpr float kIconSize = 48.f;

void CHeroes::SyncTabToCurrentHero()
{
	const int nCurFrame = ImGui::GetFrameCount();
	if (nCurFrame <= m_nLastMenuFrame + 1)
		return;

	auto* pCtrl = GetCL_CitadelPlayerController()->GetLocal();
	if (!pCtrl)
		return;

	const uint32_t nHeroID = pCtrl->m_PlayerDataGlobal().m_nHeroID().m_Value;
	for (int i = 0; i < static_cast<int>(m_Components.size()); ++i)
	{
		if (m_Components[i]->GetHeroID() == nHeroID)
		{
			m_nActiveHero = i;
			return;
		}
	}
}

void CHeroes::RenderMenu()
{
	SyncTabToCurrentHero();
	m_nLastMenuFrame = ImGui::GetFrameCount();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.f, 4.f));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.f, 4.f));
	ImGui::BeginChild(XorStr("##HeroTabBar"), ImVec2(-1.f, kTabBarH), true);

	for (int i = 0; i < static_cast<int>(m_Components.size()); ++i)
	{
		IHeroComponent* comp = m_Components[i];
		const char* szPath = HeroImagePathFromID(comp->GetHeroID());
		auto* pSRV = szPath ? CPanoramaImageCache::GetTexture(szPath) : nullptr;

		const bool bSelected = m_nActiveHero == i;

		ImGui::PushID(i);

		if (bSelected)
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

		if (pSRV)
		{
			if (ImGui::ImageButton("##hbtn", (ImTextureID)(ImU64)pSRV, ImVec2(kIconSize, kIconSize)))
				m_nActiveHero = i;
		}
		else
		{
			if (ImGui::Button("##hbtn", ImVec2(kIconSize, kIconSize)))
				m_nActiveHero = i;
		}

		if (bSelected)
			ImGui::PopStyleColor();

		ImGui::PopID();
		ImGui::SameLine();
	}

	ImGui::EndChild();
	ImGui::PopStyleVar(2);

	ImGui::BeginChild(XorStr("##HeroContent"), ImVec2(-1.f, -1.f), true);
	m_Components[m_nActiveHero]->RenderMenu();
	ImGui::EndChild();
}