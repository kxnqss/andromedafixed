#include "CAndromedaClient.hpp"
#include "CAndromedaGUI.hpp"

#include "Fonts/CFontManager.hpp"

#include <DeadLock/SDK/SDK.hpp>
#include <DeadLock/SDK/Interface/IEngineToClient.hpp>

#include <DeadLock/SDK/Update/CCitadelInput.hpp>

#include <GameClient/CEntityCache/CEntityCache.hpp>
#include <GameClient/CL_CitadelPlayerController.hpp>

#include <AndromedaClient/Features/FeatureRegistry.hpp>

#include <AndromedaClient/GUI/CAndromedaMenu.hpp>
#include <AndromedaClient/Settings/CSettingsJson.hpp>
#include <AndromedaClient/Render/CRenderStackSystem.hpp>

static CAndromedaClient g_CAndromedaClient{};

void CAndromedaClient::OnInit()
{
	const uint32_t loaded = GetSettingsJson()->GetConfigLoadedIndex();
	if (loaded != UINT_MAX)
		GetAndromedaMenu()->SetConfigSelected(loaded);
}

void CAndromedaClient::OnFireEventClientSide(IGameEvent* pGameEvent) {}

void CAndromedaClient::OnAddEntity(CEntityInstance* pInst, CHandle handle)
{
	GetEntityCache()->OnAddEntity(pInst, handle);
}

void CAndromedaClient::OnRemoveEntity(CEntityInstance* pInst, CHandle handle)
{
	GetEntityCache()->OnRemoveEntity(pInst, handle);
}

void CAndromedaClient::OnStartSound(const Vector3& Pos, const int SourceEntityIndex, const char* szSoundName)
{
	for (auto* f : FeatureRegistry::GetRootFeatures())
		f->OnStartSound(Pos, SourceEntityIndex, szSoundName);
}

void CAndromedaClient::OnClientOutput()
{
	if (!GetAndromedaGUI()->IsInited() || !SDK::Interfaces::EngineToClient()->IsInGame())
		return;

	for (auto* f : FeatureRegistry::GetRootFeatures())
		f->OnClientOutput();
}

void CAndromedaClient::OnRender()
{
	if (GetAndromedaGUI()->IsVisible())
		GetAndromedaMenu()->OnRenderMenu();

	GetFontManager()->FirstInitFonts();
	GetFontManager()->m_VerdanaFont.DrawString(1, 1, ImColor(1.f, 1.f, 0.f), FW1_LEFT, XorStr(CHEAT_NAME));

	GetRenderStackSystem()->OnRenderStack();

	for (auto* f : FeatureRegistry::GetRootFeatures())
		f->OnRender();
}

void CAndromedaClient::OnCreateMove(CCitadelInput* pCitadelInput, CUserCmd* pUserCmd)
{
	if (!GetAndromedaGUI()->IsInited())
		return;

	if (auto* pLocalCtrl = GetCL_CitadelPlayerController()->GetLocal(); pLocalCtrl)
	{
		auto* pUserCmd2 = pCitadelInput->GetUserCmd(pLocalCtrl);
		for (auto* f : FeatureRegistry::GetRootFeatures())
			f->OnCreateMove(pCitadelInput, pUserCmd2);
	}
}

void CAndromedaClient::OnPostMove(CCitadelInput* pCitadelInput)
{
	if (!GetAndromedaGUI()->IsInited())
		return;

	for (auto* f : FeatureRegistry::GetRootFeatures())
		f->OnPostMove(pCitadelInput);
}

CAndromedaClient* GetAndromedaClient()
{
	return &g_CAndromedaClient;
}