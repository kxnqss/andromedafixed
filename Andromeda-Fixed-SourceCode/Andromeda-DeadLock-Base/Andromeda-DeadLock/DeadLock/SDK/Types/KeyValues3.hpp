#pragma once

#include <Common/Common.hpp>

// ── KV3 format descriptor ─────────────────────────────────────────────────────

struct KV3ID_t
{
	const char* name = nullptr;
	uint64_t    unk0 = 0;
	uint64_t    unk1 = 0;
};

// Opaque KV3 node – actual internal layout is engine-private.
// Allocated on the stack via a fixed-size buffer; 0x20 bytes covers the header.
class alignas(8) KeyValues3
{
	uint8_t m_Pad[0x20];
};
