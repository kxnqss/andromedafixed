#pragma once

#include <mutex>
#include <unordered_map>

#include <Common/Common.hpp>
#include <DeadLock/SDK/Math/Vector3.hpp>

#include "IMiscComponent.hpp"

// Per-attacker heavy-melee charge state.
// Created in OnStartSound (network thread), validated in OnPostMove (game thread).
struct MeleeCharge
{
	ULONGLONG startMs = 0;
	ULONGLONG firstVisibleMs = 0;
	Vector3 initialPos = {};
	float initialDist = 0.f;
	bool posInitialized = false;
};

class CAutoParryComponent final : public IMiscComponent
{
public:
	void Load(const rapidjson::Value& v) override;
	void Save(JsonWriter& w) override;
	void RenderMenu() override;

	void OnStartSound(const Vector3&, int SourceEntityIndex, const char* szSoundName) override;
	void OnPostMove(CCitadelInput* pInput) override;

	bool Enabled = true;
	float Fov = 180.f;

private:
	void HandleSound(int entityIndex, const char* szSoundName);
	void Tick(CCitadelInput* pInput);

	std::mutex m_mutex;
	std::unordered_map<int, MeleeCharge> m_pendingCharges;
};