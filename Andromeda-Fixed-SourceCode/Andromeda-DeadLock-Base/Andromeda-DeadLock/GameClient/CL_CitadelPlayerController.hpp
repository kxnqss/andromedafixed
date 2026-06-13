#pragma once

class CCitadelPlayerController;

class CL_CitadelPlayerController final
{
public:
	CCitadelPlayerController* GetLocal();
};

CL_CitadelPlayerController* GetCL_CitadelPlayerController();