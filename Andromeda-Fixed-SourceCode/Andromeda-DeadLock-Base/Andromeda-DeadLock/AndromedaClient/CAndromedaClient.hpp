#pragma once

#include <DeadLock/SDK/Types/CEntityData.hpp>

class IGameEvent;
class CCitadelInput;
class CUserCmd;

class IAndromedaClient
{
public:
	virtual ~IAndromedaClient() = default;
	virtual void OnFireEventClientSide(IGameEvent* pGameEvent) = 0;
	virtual void OnAddEntity(CEntityInstance* pInst, CHandle handle) = 0;
	virtual void OnRemoveEntity(CEntityInstance* pInst, CHandle handle) = 0;
	virtual void OnStartSound(const Vector3& Pos, const int SourceEntityIndex, const char* szSoundName) = 0;
	virtual void OnCreateMove(CCitadelInput* pCitadelInput, CUserCmd* pUserCmd) = 0;
	virtual void OnPostMove(CCitadelInput* pCitadelInput) = 0;
	virtual void OnClientOutput() = 0;
	virtual void OnRender() = 0;
};

class CAndromedaClient final : public IAndromedaClient
{
public:
	static void OnInit();

	void OnFireEventClientSide(IGameEvent* pGameEvent) override;
	void OnAddEntity(CEntityInstance* pInst, CHandle handle) override;
	void OnRemoveEntity(CEntityInstance* pInst, CHandle handle) override;
	void OnStartSound(const Vector3& Pos, const int SourceEntityIndex, const char* szSoundName) override;
	void OnCreateMove(CCitadelInput* pCitadelInput, CUserCmd* pUserCmd) override;
	void OnPostMove(CCitadelInput* pCitadelInput) override;
	void OnClientOutput() override;
	void OnRender() override;
};

CAndromedaClient* GetAndromedaClient();