#pragma once

#include <DeadLock/SDK/Update/CCitadelCamera.hpp>
#include <DeadLock/SDK/Update/CCitadelInput.hpp>
#include <DeadLock/SDK/Update/CUserCmd.hpp>

#include <AndromedaClient/Features/IFeature.hpp>

class CAimbot : public IFeature
{
public:
	// IFeature
	const char* GetName() const override;
	const char8_t* GetIcon() const override;
	void Load(const rapidjson::Value& doc) override;
	void Save(JsonWriter& w) override;
	void RenderMenu() override;

	// IFeature (engine callbacks)
	void OnCreateMove(CCitadelInput* pInput, CUserCmd* pUserCmd) override;
	void OnPostMove(CCitadelInput* pInput) override;
	void OnRender() override;

	void OnDamage(int attacker_entidx, int victim_entidx, int hitgroup_id);

	// ── Settings ────────────────────────────────────────────────────────────
	bool Active = false;
	int AimKey = 0x01; // VK_LBUTTON
	float SensitivityX = 10.f;
	float SensitivityY = 10.f;
	float InertiaScale = 1.0f; // multiplier for inertia strength (0.1=minimal, 2.0=heavy)
	float DriftScale = 1.0f; // multiplier for drift amplitude (0=off, 2.0=strong)
	float Fov = 60.f;
	bool ShowFov = true;
	float FovColor[3] = {1.f, 1.f, 1.f};
	// Bitmask: bit0=Head, bit1=Neck, bit2=Torso, bit3=Arms, bit4=Legs  (see CAimbotBones.hpp)
	int BonesMask = 0b00111; // default: Head | Neck | Torso
	bool AntiFrog = true;
	float HsThreshold = 45.f; // percent

	// ── Cached live VData bullet params (updated each frame, not saved) ──
	float m_flCachedBulletSpeed = 0.f;
	float m_flCachedBulletGravity = 0.f;
	float m_flFovScale = 1.f;

private:
	struct PendingWrite
	{
		bool valid = false;
		CCitadel_Camera* pActiveCam = nullptr;
		QAngle qFinal;
		uintptr_t pawnAddr = 0;
	} m_pending;
};

CAimbot* GetAimbot();