#pragma once

#include <Common/MemoryEngine.hpp>
#include <DeadLock/SDK/Math/Vector3.hpp>
#include <DeadLock/SDK/Math/QAngle.hpp>

// ── CCitadel_Camera ───────────────────────────────────────────────────────────
// Source: sdk/client/_rtti/CCitadel_Camera.hpp  (RTTI-only, no schema registration)
// Update: check field_0x0038, field_0x0044, field_0x00C8, field_0x00D4 in the _rtti file after a game patch.
class CCitadel_Camera
{
	// [RTTI] field_0x0038 — Vector3, world-space camera position (x,y,z, 12 bytes)
	static constexpr uintptr_t k_vecPosition    = 0x38;
	// [RTTI] field_0x0044 — float[3] { pitch, yaw, roll }, view angles (standard path)
	static constexpr uintptr_t k_angView        = 0x44;
	// [RTTI] field_0x0050 — float
	static constexpr uintptr_t k_flFov = 0x50;
	// [RTTI CCitadel_ThirdPersonCamera] field_0x00C8 — Vector3, third-person camera position
	static constexpr uintptr_t k_vecPosition_3P = 0xC8;
	// [RTTI CCitadel_ThirdPersonCamera] field_0x00D4 — float[3] { pitch, yaw, roll }, view angles (Apollo/Fencer path, vtable[15])
	static constexpr uintptr_t k_angView_3P     = 0xD4;

public:
	CUSTOM_OFFSET_FIELD( Vector3, m_vecPosition, k_vecPosition );

	inline auto m_angView() -> float*
	{
		return reinterpret_cast<float*>( reinterpret_cast<uintptr_t>( this ) + k_angView );
	}

	inline auto m_flFov() -> float
	{
		return *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(this) + k_flFov);
	}

	// Writes to both angle fields so every hero's render pipeline picks up the change.
	// Standard chars read 0x44; Apollo/Fencer read 0xD4 via vtable[15].
	// Writing to 0xD4 for standard chars is safe — field exists (always CCitadel_ThirdPersonCamera),
	// but that path is never called for them, so it has no effect.
	inline void SetAngles( float pitch, float yaw, float roll = 0.f )
	{
		float* ang0 = reinterpret_cast<float*>( reinterpret_cast<uintptr_t>( this ) + k_angView );
		float* ang1 = reinterpret_cast<float*>( reinterpret_cast<uintptr_t>( this ) + k_angView_3P );
		ang0[0] = ang1[0] = pitch;
		ang0[1] = ang1[1] = yaw;
		ang0[2] = ang1[2] = roll;
	}
};

// ── CCitadelCameraManager ─────────────────────────────────────────────────────
// Source: sdk/client/_rtti/CCitadelCameraManager.hpp  (RTTI-only, no schema registration)
// Update: check field_0x0028 in the _rtti file after a game patch.
class CCitadelCameraManager
{
	// [RTTI] field_0x0028 — CCitadel_Camera*, pointer to the currently active camera
	static constexpr uintptr_t k_pActiveCamera = 0x28;

public:
	auto GetActiveCamera() -> CCitadel_Camera*
	{
		return *reinterpret_cast<CCitadel_Camera**>(
			reinterpret_cast<uintptr_t>( this ) + k_pActiveCamera
		);
	}
};
