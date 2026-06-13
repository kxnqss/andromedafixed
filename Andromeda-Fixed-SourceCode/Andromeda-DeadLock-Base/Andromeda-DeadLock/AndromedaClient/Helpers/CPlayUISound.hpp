#pragma once

#include <Common/Common.hpp>

enum class UISound : uint32_t
{
	UI_Click,
	UI_Hover,
};

class CPlayUISound final
{
public:
	void PlayUISound(const UISound Sound);
};

CPlayUISound* GetPlayUISound();