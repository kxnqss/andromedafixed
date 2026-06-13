#include "CMisc.hpp"

#include <AndromedaClient/Fonts/FontAwesomeIcon.hpp>

static CMisc g_CMisc{};

void CMisc::OnStartSound(const Vector3& pos, int entityIdx, const char* szSoundName)
{
	for (auto* comp : m_Components)
		comp->OnStartSound(pos, entityIdx, szSoundName);
}

void CMisc::OnPostMove(CCitadelInput* pInput)
{
	for (auto* comp : m_Components)
		comp->OnPostMove(pInput);
}

const char* CMisc::GetName() const
{
	return "Misc";
}

const char8_t* CMisc::GetIcon() const
{
	return ICON_FA_PUZZLE_PIECE;
}

CMisc* GetMisc()
{
	return &g_CMisc;
}