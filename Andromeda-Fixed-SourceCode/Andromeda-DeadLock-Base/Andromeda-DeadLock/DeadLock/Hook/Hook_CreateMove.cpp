#include "Hook_CreateMove.hpp"

#include <DeadLock/SDK/Update/CCitadelInput.hpp>

#include <GameClient/CL_CitadelPlayerController.hpp>

#include <AndromedaClient/CAndromedaClient.hpp>

auto Hook_CreateMove( CCitadelInput* pCitadelInput , uint32_t split_screen_index , char a3 ) -> void
{
	// ── PreMove ──────────────────────────────────────────────────────
	// Aimbot writes camera angles BEFORE the original processes input.
	// This way the game engine uses our aimed angles as the base for
	// this tick, eliminating lag and jitter during movement.
	if ( auto* pLocalCtrl = GetCL_CitadelPlayerController()->GetLocal(); pLocalCtrl )
	{
		// GetUserCmd may already have data; pass it if available
		auto* pUserCmd = pCitadelInput->GetUserCmd( pLocalCtrl );
		GetAndromedaClient()->OnCreateMove( pCitadelInput , pUserCmd );
	}

	// ── Original ─────────────────────────────────────────────────────
	CreateMove_o( pCitadelInput , split_screen_index , a3 );

	// ── PostMove ─────────────────────────────────────────────────────
	// Apply camera angles AFTER the original so our write wins over any
	// engine angle processing done during the tick (fixes movement jitter).
	GetAndromedaClient()->OnPostMove( pCitadelInput );
}
