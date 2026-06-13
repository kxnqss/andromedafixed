#pragma once

#include <cstdint>

// ── Types exposed to callers ──────────────────────────────────────────────────
// Internal structs (Ray_t, TraceFilter_t, TraceShapeFn) stay in the .cpp.

struct TraceVec3
{
	float x, y, z;
};

struct GameTrace_t
{
	void* m_pSurface = nullptr; // 0x00
	void* m_pHitEntity = nullptr; // 0x08
	void* m_pHitboxData = nullptr; // 0x10
	char pad2[0x38] = {}; // 0x18
	uint32_t m_uContents = 0; // 0x50
	char pad3[0x24] = {}; // 0x54
	TraceVec3 m_vStartPos = {}; // 0x78
	TraceVec3 m_vEndPos = {}; // 0x84
	TraceVec3 m_vNormal = {}; // 0x90
	TraceVec3 m_vPosition = {}; // 0x9C
	char pad4[0x04] = {}; // 0xA8
	float m_flFraction = 1.f; // 0xAC
	char pad5[0x06] = {}; // 0xB0
	bool m_bAllSolid = false; // 0xB6
	char pad6[0x5C] = {}; // 0xB7→0x113
};

namespace TraceMask
{
static constexpr uint64_t SHOT = 0x1C3003;
static constexpr uint64_t VISIBILITY = 0x1C3003;
}

// ── Public API ────────────────────────────────────────────────────────────────

// Initialize the trace system (pattern-scan for TraceShape wrapper + manager).
// Returns true on success.
bool Trace_Init();

// Reinitialize after a map reload (when the manager pointer goes stale).
void Trace_Reinit();

// True if the trace system has been successfully initialized and is usable.
bool Trace_IsReady();

// Lazy init + stale-manager check. Call once per feature tick before tracing.
void Trace_EnsureReady();

// Straight-line visibility check from 'from' to 'to'.
// skip_entity is excluded from the trace (typically: your own pawn's address).
// Returns true when the path is clear (fraction > 0.97).
// Returns true (optimistic) when the trace system is not yet ready.
bool Trace_IsVisible(const TraceVec3* from, const TraceVec3* to,
                     uintptr_t skip_entity);

// Like Trace_IsVisible, but additionally checks every cached FissureWall entity.
// Returns false if the segment intersects any wall's world-space AABB.
bool Trace_IsVisibleEx(const TraceVec3* from, const TraceVec3* to,
                       uintptr_t skip_entity);

// Raw trace — full result written to *result.
bool Trace_Do(const TraceVec3* from, const TraceVec3* to,
              uint64_t mask, uintptr_t skip_entity,
              bool world_only, GameTrace_t* result);