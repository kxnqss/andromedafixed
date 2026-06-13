#pragma once

#include <Common/Common.hpp>

#define MATERIALSYSTEM2_DLL        "materialsystem2.dll"
#define CMATERIALSYSTEM2_INTERFACE "VMaterialSystem2_001"

class KeyValues3;

// ── CMaterial2 ────────────────────────────────────────────────────────────────
// vtable[0] = GetName() — verified in IDA (sub_18000B640 in materialsystem2.dll).

class CMaterial2
{
public:
	// vtable[0] = GetName (confirmed in IDA: sub_18000B640, ??_7CMaterial2@@6B@ at 0x18012A160)
	// vtable[1] = GetShareName
	inline const char* GetName()
	{
		using Fn = const char*( __fastcall* )( CMaterial2* );
		auto* vtable = *reinterpret_cast<void***>( this );
		if ( !vtable || !vtable[ 0 ] ) return nullptr;
		return reinterpret_cast<Fn>( vtable[ 0 ] )( this );
	}
};

// ── CMaterialSystem2 ──────────────────────────────────────────────────────────
// Fetched via CreateInterface("VMaterialSystem2_001") from materialsystem2.dll.
// CreateMaterial is exposed as a free function in FunctionListSDK.hpp.

class CMaterialSystem2
{
};
