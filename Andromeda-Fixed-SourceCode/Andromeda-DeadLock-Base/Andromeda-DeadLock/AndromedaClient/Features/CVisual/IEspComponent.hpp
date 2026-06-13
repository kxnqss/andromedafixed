#pragma once

#include <Common/Common.hpp>
#include <DeadLock/SDK/Math/Rect_t.hpp>
#include <AndromedaClient/Settings/JsonHelpers.hpp>

class CCitadelPlayerController;
class C_CitadelPlayerPawn;
class C_NPC_Trooper;

// ── Position anchor ───────────────────────────────────────────────────────────

enum class EEspPosition : int
{
	TopLeft      = 0,
	TopMiddle    = 1,
	TopRight     = 2,
	BottomLeft   = 3,
	BottomMiddle = 4,
	BottomRight  = 5,
};

// ── Per-player render context ─────────────────────────────────────────────────
// Built once per entity in the main loop, then passed to every component.

struct EspPlayerContext
{
	CCitadelPlayerController* pCtrl    = nullptr;
	C_CitadelPlayerPawn*      pPawn    = nullptr;
	bool                      bTeam    = false;
	bool                      bAlive   = false;
	Rect_t                    box      = {};
	bool                      boxValid = false;
};

// ── IEspComponent ─────────────────────────────────────────────────────────────

class IEspComponent
{
public:
	virtual ~IEspComponent() = default;

	// Persistence
	virtual void Load(const rapidjson::Value& v) = 0;
	virtual void Save(JsonWriter& w)             = 0;

	// Menu – Enemy / Team table columns
	virtual void RenderEnemyMenu() = 0;
	virtual void RenderTeamMenu()  = 0;

	// Menu – "Other" section (outside the table); default: no-op
	virtual void RenderOtherMenu() {}

	// Rendering hooks – default no-op, override only what the component needs
	virtual void OnRenderPlayer (const EspPlayerContext& ctx)                {}
	virtual void OnRenderTrooper(C_NPC_Trooper* pTrooper, const Rect_t& box) {}

	// Per-frame logic not tied to a single entity (minimap unlock, etc.)
	virtual void OnFrame() {}

	// ImGui overlay rendering (called during the render pass)
	virtual void OnRender() {}
};
