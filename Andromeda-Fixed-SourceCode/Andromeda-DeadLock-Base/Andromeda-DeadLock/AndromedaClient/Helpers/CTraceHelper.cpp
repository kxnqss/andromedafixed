#include "CTraceHelper.hpp"

#include <Windows.h>
#include <atomic>
#include <cmath>
#include <cstring>

#include <Common/Common.hpp>
#include <Common/MemoryEngine.hpp>

#include "DeadLock/SDK/SDK.hpp"

#include <GameClient/CEntityCache/CEntityCache.hpp>

// ── Internal types ────────────────────────────────────────────────────────────

struct Ray_t
{
	char data[64] = {};
};

struct TraceFilter_t
{
	uintptr_t vtable_ptr = 0; // 0x00
	int64_t m_uTraceMask = 0; // 0x08
	int64_t m_v1[2] = {}; // 0x10
	int32_t m_arrSkipHandles[4] = {}; // 0x20
	int16_t m_arrCollisions[2] = {}; // 0x30
	int16_t m_v2 = 0; // 0x34
	uint8_t m_v3 = 0; // 0x36
	uint8_t m_v4 = 0; // 0x37
	uint8_t m_v5 = 0; // 0x38
	uint8_t m_v6 = 0; // 0x39
	uint8_t pad[0x0E] = {}; // 0x3A→0x48

	void init_default(uint64_t mask)
	{
		memset(this, 0, sizeof(*this));
		m_uTraceMask = static_cast<int64_t>(mask);
		m_arrSkipHandles[0] = 0xFFFFFFFF;
		m_arrSkipHandles[1] = 0xFFFFFFFF;
		m_arrSkipHandles[2] = 0xFFFFFFFF;
		m_arrSkipHandles[3] = 0xFFFFFFFF;
		m_v2 = (int16_t)0xFFFF;
		m_v4 = 0x0F;
		m_v5 = 0x03;
		m_v6 = 0x69;
		// filter[0x40] != 0 → entity path → ShouldHitEntity IS called
		// filter[0x40] == 0 → BSP-only path → ShouldHitEntity never called
		*(reinterpret_cast<uint8_t*>(this) + 0x40) = 1;
	}

	void set_vtable(uintptr_t addr)
	{
		vtable_ptr = addr;
	}
};

using TraceShapeFn = bool(__fastcall *)(
	void* pManager, // RCX
	Ray_t* pRay, // RDX
	const TraceVec3* pStart, // R8
	const TraceVec3* pEnd, // R9
	TraceFilter_t* pFilter, // [RSP+0x20]
	GameTrace_t* pResult // [RSP+0x28]
	);

// ── State ─────────────────────────────────────────────────────────────────────

static uintptr_t g_manager_global_addr = 0;
static TraceShapeFn g_wrapper_fn = nullptr;
static uintptr_t g_wrapper_addr = 0;
static uintptr_t g_filter_vtable = 0;
static bool g_ready = false;
static std::atomic<int> g_crash_count{0};

static constexpr int VTABLE_COPY_SIZE = 16;
static uintptr_t g_vtable_world_only[VTABLE_COPY_SIZE] = {};
static uintptr_t g_vtable_with_entities[VTABLE_COPY_SIZE] = {};

static uintptr_t g_skip_entity = 0;

// For Trace_EnsureReady lazy init
static bool s_TraceInited = false;

// ── Custom vtable callbacks ───────────────────────────────────────────────────

static int __fastcall custom_get_trace_type_world(void*)
{
	return 1;
}

static int __fastcall custom_get_trace_type_all(void*)
{
	return 0;
}

static bool __fastcall custom_should_hit_entity(void*, void* entity_handle, int)
{
	if (!entity_handle)
		return false;
	uintptr_t addr = reinterpret_cast<uintptr_t>(entity_handle);
	if (g_skip_entity && addr == g_skip_entity)
		return false;
	__try
	{
		uint8_t team = *reinterpret_cast<uint8_t*>(addr + 0x3F3);
		if (team == 0)
			return false;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
	return true;
}

// ── SEH-safe helpers ──────────────────────────────────────────────────────────

static uintptr_t seh_deref(uintptr_t addr)
{
	__try
	{
		return *reinterpret_cast<uintptr_t*>(addr);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return 0;
	}
}

static bool seh_call_wrapper(TraceShapeFn fn, void* manager, Ray_t* ray,
                             const TraceVec3* start, const TraceVec3* end,
                             TraceFilter_t* filter, GameTrace_t* result)
{
	__try
	{
		fn(manager, ray, start, end, filter, result);
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
}

// ── Pattern / RTTI scanning ───────────────────────────────────────────────────

static uintptr_t find_trace_wrapper()
{
	auto pattern =
		"48 89 5C 24 20 48 89 4C 24 08 55 57 41 54 41 55 41 56 48 8D";
	uintptr_t match =
		reinterpret_cast<uintptr_t>(FindPattern(CLIENT_DLL, pattern));

	if (!match)
	{
		pattern = "48 89 5C 24 ?? 48 89 4C 24 ?? 55 57";
		match = reinterpret_cast<uintptr_t>(FindPattern(CLIENT_DLL, pattern));
	}

	return match;
}

static uintptr_t find_manager_pattern()
{
	uintptr_t match = reinterpret_cast<uintptr_t>(FindPattern(
		CLIENT_DLL,
		XorStr("48 8B 0D ?? ?? ?? ?? 4C 8D 44 24 ?? E8 ?? ?? ?? ?? E8")));
	if (!match)
		return 0;

	int32_t disp = 0;
	__try
	{
		disp = *reinterpret_cast<int32_t*>(match + 3);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return 0;
	}

	uintptr_t global_addr = match + 7 + disp;

	const uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandleA(CLIENT_DLL));
	auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(
		base + reinterpret_cast<PIMAGE_DOS_HEADER>(base)->e_lfanew);
	size_t client_size = nt->OptionalHeader.SizeOfImage;

	if (global_addr < base || global_addr >= base + client_size)
		return 0;

	uintptr_t ptr_val = seh_deref(global_addr);
	if (!ptr_val || ptr_val < 0x10000)
		return 0;

	return global_addr;
}

static uintptr_t find_manager_via_callers(uintptr_t wrapper_addr)
{
	const uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandleA(CLIENT_DLL));
	auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(
		base + reinterpret_cast<PIMAGE_DOS_HEADER>(base)->e_lfanew);
	auto* sections = IMAGE_FIRST_SECTION(nt);

	uintptr_t text_start = 0;
	size_t text_size = 0;
	for (int i = 0; i < nt->FileHeader.NumberOfSections; ++i)
	{
		if (memcmp(sections[i].Name, ".text", 5) == 0)
		{
			text_start = base + sections[i].VirtualAddress;
			text_size = sections[i].Misc.VirtualSize;
			break;
		}
	}
	if (!text_start)
	{
		text_start = base + 0x1000;
		text_size = nt->OptionalHeader.SizeOfImage - 0x1000;
	}

	auto code = reinterpret_cast<const uint8_t*>(text_start);

	for (size_t i = 0; i + 5 <= text_size; ++i)
	{
		if (code[i] != 0xE8)
			continue;

		int32_t rel = *reinterpret_cast<const int32_t*>(code + i + 1);
		uintptr_t target = text_start + i + 5 + rel;
		if (target != wrapper_addr)
			continue;

		uintptr_t call_addr = text_start + i;

		for (size_t back = 7; back <= 64 && call_addr >= text_start + back; ++back)
		{
			uintptr_t probe = call_addr - back;
			const auto* b = reinterpret_cast<const uint8_t*>(probe);
			if (b[0] == 0x48 && b[1] == 0x8B && b[2] == 0x0D)
			{
				int32_t disp2 = *reinterpret_cast<const int32_t*>(b + 3);
				uintptr_t global_addr = probe + 7 + disp2;

				if (global_addr < base || global_addr >= base + nt->OptionalHeader.SizeOfImage)
					continue;

				uintptr_t ptr_val = seh_deref(global_addr);
				if (!ptr_val || ptr_val < 0x10000)
					continue;

				return global_addr;
			}
		}
	}

	return 0;
}

static uintptr_t s_rtti_vtable = 0;

static uintptr_t find_instance_by_rtti(const char* /*dll*/, const char* name)
{
	const uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandleA(CLIENT_DLL));
	if (!base)
		return 0;
	auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(
		base + reinterpret_cast<PIMAGE_DOS_HEADER>(base)->e_lfanew);
	const size_t imageSize = nt->OptionalHeader.SizeOfImage;

	const size_t nameLen = strlen(name);
	uintptr_t nameAddr = 0;
	for (uintptr_t i = base; i < base + imageSize - nameLen - 1; ++i)
	{
		__try
		{
			if (memcmp(reinterpret_cast<void*>(i), name, nameLen) == 0 &&
			    *reinterpret_cast<char*>(i + nameLen) == '\0')
			{
				nameAddr = i;
				break;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			continue;
		}
	}
	if (!nameAddr)
		return 0;

	const uintptr_t typeDesc = nameAddr - 16;
	const uint32_t typeDescRVA = (uint32_t)(typeDesc - base);

	for (uintptr_t i = base; i < base + imageSize - 4; i += 4)
	{
		__try
		{
			if (*reinterpret_cast<uint32_t*>(i) != typeDescRVA)
				continue;
			const uintptr_t col = i - 12;
			if (*reinterpret_cast<uint32_t*>(col) != 1)
				continue;

			for (uintptr_t j = base + 8; j < base + imageSize - 8; j += 8)
			{
				__try
				{
					if (*reinterpret_cast<uintptr_t*>(j) == col)
					{
						s_rtti_vtable = j + 8;
						return reinterpret_cast<uintptr_t>(&s_rtti_vtable);
					}
				}
				__except (EXCEPTION_EXECUTE_HANDLER) {}
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			continue;
		}
	}

	return 0;
}

// ── Core trace logic ──────────────────────────────────────────────────────────

static bool verify_wrapper()
{
	if (!g_wrapper_fn || !g_manager_global_addr)
		return false;

	uintptr_t manager_ptr = seh_deref(g_manager_global_addr);
	if (!manager_ptr || manager_ptr < 0x10000)
		return false;

	TraceVec3 sky_start{0.f, 0.f, 50000.f};
	TraceVec3 sky_end{0.f, 0.f, 50001.f};

	Ray_t ray;
	TraceFilter_t filter;
	filter.init_default(TraceMask::SHOT);

	GameTrace_t result;
	memset(&result, 0, sizeof(result));
	result.m_flFraction = -1.f;

	if (!seh_call_wrapper(g_wrapper_fn,
	                      reinterpret_cast<void*>(manager_ptr),
	                      &ray, &sky_start, &sky_end, &filter, &result))
		return false;

	float fraction = result.m_flFraction;
	return (fraction >= 0.f && fraction <= 1.01f);
}

static bool do_trace_single(const TraceVec3* from, const TraceVec3* to,
                            uint64_t mask, bool /*world_only*/,
                            GameTrace_t* result)
{
	uintptr_t manager_ptr = seh_deref(g_manager_global_addr);
	if (!manager_ptr)
		return false;

	memset(result, 0, sizeof(GameTrace_t));
	result->m_flFraction = 1.f;

	Ray_t ray;
	TraceFilter_t filter;
	filter.init_default(mask);

	if (g_vtable_with_entities[0])
		filter.set_vtable(reinterpret_cast<uintptr_t>(&g_vtable_with_entities));
	else if (g_filter_vtable)
		filter.set_vtable(g_filter_vtable);

	if (!seh_call_wrapper(g_wrapper_fn,
	                      reinterpret_cast<void*>(manager_ptr),
	                      &ray, from, to, &filter, result))
	{
		if (g_crash_count.fetch_add(1) + 1 >= 3)
			g_ready = false;
		return false;
	}

	return true;
}

static bool do_trace_internal(const TraceVec3* from, const TraceVec3* to,
                              uint64_t mask, uintptr_t skip_entity,
                              bool world_only, GameTrace_t* result)
{
	if (!g_ready || g_crash_count.load() >= 3 || !g_wrapper_fn)
		return false;

	g_skip_entity = skip_entity;

	if (!do_trace_single(from, to, mask, world_only, result))
		return false;

	if (!world_only && skip_entity)
	{
		TraceVec3 cur_start = *from;
		float accum_fraction = 0.f;
		float remaining = 1.f;

		for (int retry = 0; retry < 3; ++retry)
		{
			auto* hit = result->m_pHitEntity;
			if (!hit || reinterpret_cast<uintptr_t>(hit) != skip_entity)
				break;

			float frac = result->m_flFraction;
			accum_fraction += frac * remaining;
			remaining = 1.f - accum_fraction;

			if (remaining < 0.001f)
			{
				result->m_flFraction = 1.f;
				result->m_pHitEntity = nullptr;
				break;
			}

			float dx = to->x - from->x, dy = to->y - from->y, dz = to->z - from->z;
			float len = sqrtf(dx * dx + dy * dy + dz * dz);
			if (len > 0.001f)
			{
				float nudge = 1.f / len;
				cur_start.x = result->m_vEndPos.x + dx * nudge;
				cur_start.y = result->m_vEndPos.y + dy * nudge;
				cur_start.z = result->m_vEndPos.z + dz * nudge;
			}
			else
			{
				break;
			}

			if (!do_trace_single(&cur_start, to, mask, world_only, result))
				return false;

			result->m_flFraction = accum_fraction + result->m_flFraction * remaining;
		}
	}

	return true;
}

// ── Public API ────────────────────────────────────────────────────────────────

bool Trace_Init()
{
	const uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandleA(CLIENT_DLL));
	if (!base)
		return false;

	if (!g_wrapper_fn)
	{
		uintptr_t fn = find_trace_wrapper();
		if (!fn)
			return false;
		g_wrapper_fn = reinterpret_cast<TraceShapeFn>(fn);
		g_wrapper_addr = fn;
	}

	if (!g_manager_global_addr)
	{
		// Caller-scan is more reliable than a pattern match against globals.
		g_manager_global_addr = find_manager_via_callers(g_wrapper_addr);
		if (!g_manager_global_addr)
			g_manager_global_addr = find_manager_pattern();
		if (!g_manager_global_addr)
			return false;
	}

	if (!g_filter_vtable)
	{
		// Prefer CTraceFilterLOS; fall back to base CTraceFilter, then hard RVA.
		uintptr_t instance = find_instance_by_rtti(CLIENT_DLL, ".?AVCTraceFilterLOS@@");
		if (!instance)
			instance = find_instance_by_rtti(CLIENT_DLL, ".?AVCTraceFilter@@");
		if (instance)
			g_filter_vtable = seh_deref(instance);

		auto nt = reinterpret_cast<PIMAGE_NT_HEADERS>(
			base + reinterpret_cast<PIMAGE_DOS_HEADER>(base)->e_lfanew);
		(void)nt; // suppress unused warning

		if (!g_filter_vtable || g_filter_vtable < base)
			g_filter_vtable = base + 0x28FDC60u;

		if (g_filter_vtable)
		{
			// CTraceFilter vtable layout (verified via dezlock dump):
			//   [0] ShouldHitEntity(C_BaseEntity*, int) → bool
			//   [1] GetTraceType() → int
			g_vtable_world_only[0] = seh_deref(g_filter_vtable);
			g_vtable_world_only[1] = reinterpret_cast<uintptr_t>(&custom_get_trace_type_world);
			g_vtable_with_entities[0] = reinterpret_cast<uintptr_t>(&custom_should_hit_entity);
			g_vtable_with_entities[1] = reinterpret_cast<uintptr_t>(&custom_get_trace_type_all);
		}
	}

	uintptr_t manager_ptr = seh_deref(g_manager_global_addr);
	if (!manager_ptr || manager_ptr < 0x10000)
		return false;

	if (!verify_wrapper())
	{
		g_wrapper_fn = nullptr;
		g_wrapper_addr = 0;
		g_manager_global_addr = 0;
		return false;
	}

	g_ready = true;
	g_crash_count.store(0);
	return true;
}

void Trace_Reinit()
{
	auto saved_fn = g_wrapper_fn;
	auto saved_addr = g_wrapper_addr;

	g_ready = false;
	g_manager_global_addr = 0;
	g_filter_vtable = 0;
	memset(g_vtable_world_only, 0, sizeof(g_vtable_world_only));
	memset(g_vtable_with_entities, 0, sizeof(g_vtable_with_entities));
	g_crash_count.store(0);

	g_wrapper_fn = saved_fn;
	g_wrapper_addr = saved_addr;

	Trace_Init();
}

bool Trace_IsReady()
{
	return g_ready;
}

void Trace_EnsureReady()
{
	if (!s_TraceInited)
	{
		s_TraceInited = true;
		Trace_Init();
		return;
	}
	// Reinit if manager pointer went stale (map reload).
	if (g_ready && seh_deref(g_manager_global_addr) == 0)
		Trace_Reinit();
}

bool Trace_IsVisible(const TraceVec3* from, const TraceVec3* to,
                     uintptr_t skip_entity)
{
	if (!g_ready)
		return true; // optimistic: don't skip targets when uninitialised

	GameTrace_t result;
	if (!do_trace_internal(from, to, TraceMask::VISIBILITY, skip_entity, true, &result))
		return true;

	return result.m_flFraction > 0.97f;
}

bool Trace_Do(const TraceVec3* from, const TraceVec3* to,
              uint64_t mask, uintptr_t skip_entity,
              bool world_only, GameTrace_t* result)
{
	return do_trace_internal(from, to, mask, skip_entity, world_only, result);
}

// ── FissureWall segment-AABB test ─────────────────────────────────────────────

static bool SegmentIntersectsAABB(const TraceVec3& s, const TraceVec3& e, const Vector3& mins, const Vector3& maxs)
{
	const float d[3] = {e.x - s.x, e.y - s.y, e.z - s.z};
	const float o[3] = {s.x, s.y, s.z};
	const float lo[3] = {mins.m_x, mins.m_y, mins.m_z};
	const float hi[3] = {maxs.m_x, maxs.m_y, maxs.m_z};

	float tmin = 0.f, tmax = 1.f;

	for (int i = 0; i < 3; ++i)
	{
		if (fabsf(d[i]) < 1e-6f)
		{
			if (o[i] < lo[i] || o[i] > hi[i])
				return false;
		}
		else
		{
			float t0 = (lo[i] - o[i]) / d[i];
			float t1 = (hi[i] - o[i]) / d[i];

			if (t0 > t1)
				std::swap(t0, t1);

			tmin = (std::max)(tmin, t0);
			tmax = (std::min)(tmax, t1);

			if (tmin > tmax)
				return false;
		}
	}

	return true;
}

bool Trace_IsVisibleEx(const TraceVec3* from, const TraceVec3* to,
                       uintptr_t skip_entity)
{
	if (!Trace_IsVisible(from, to, skip_entity))
		return false;

	auto* pCache = GetEntityCache();
	std::scoped_lock lock(pCache->GetLock());

	for (const auto& cached : pCache->GetEntitiesByType(CachedEntity_t::CITADEL_FISSURE_WALL))
	{
		auto* pEnt = cached.m_Handle.Get();
		if (!pEnt || pEnt->pEntityIdentity()->Handle() != cached.m_Handle)
			continue;

		auto* pModel = static_cast<C_BaseModelEntity*>(pEnt);
		const Vector3 origin = pEnt->GetOrigin();
		const Vector3 vMins = pModel->m_Collision().m_vecMins() + origin;
		const Vector3 vMaxs = pModel->m_Collision().m_vecMaxs() + origin;

		if (SegmentIntersectsAABB(*from, *to, vMins, vMaxs))
			return false;
	}

	return true;
}