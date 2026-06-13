#pragma once

#include <Common/Common.hpp>
#include <FW1FontWrapper/FW1FontWrapper.h>

#include "CFont.hpp"

class CFontManager final
{
public:
	void FirstInitFonts();

public:
	CFont m_VerdanaFont;

private:
	IFW1Factory* m_pFW1Factory = nullptr;

private:
	bool m_bInit = false;
};

CFontManager* GetFontManager();