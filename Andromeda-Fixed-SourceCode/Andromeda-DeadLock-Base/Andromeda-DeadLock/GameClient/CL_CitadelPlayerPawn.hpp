#pragma once

class C_CitadelPlayerPawn;

class CL_CitadelPlayerPawn final
{
public:
	C_CitadelPlayerPawn* GetLocal();
};

CL_CitadelPlayerPawn* GetCL_CitadelPlayerPawn();