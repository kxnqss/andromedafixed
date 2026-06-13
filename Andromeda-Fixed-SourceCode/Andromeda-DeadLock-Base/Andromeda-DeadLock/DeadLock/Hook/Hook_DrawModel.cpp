#include "Hook_DrawModel.hpp"

#include <DeadLock/SDK/FunctionListSDK.hpp>
#include <AndromedaClient/Features/CVisual/CVisual.hpp>
#include <GameClient/CL_CitadelPlayerPawn.hpp>
#include <Common/MemoryEngine.hpp>

// ── Chams material state ──────────────────────────────────────────────────────

static CMaterial2* g_pChamsEnemy = nullptr;
static CMaterial2* g_pChamsTeam = nullptr;
static bool g_bCreated = false;
static bool g_bInitDone = false;

// ── Vmat template ─────────────────────────────────────────────────────────────

static constexpr char kVmatFmt[] =
	R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    shader = "fuckyounothinghasworked.vfx"
    F_RENDER_BACKFACES = 1
    g_vColorTint = [1.000, 1.000, 1.000]
    g_flMetalness = 0.0
    TextureColor = resource:"materials/default/default_color.tga"
    TextureNormal = resource:"materials/default/default_normal.tga"
}
)";

// ── Material helpers ──────────────────────────────────────────────────────────

static std::string GenerateRandomName(const char* suffix)
{
	static constexpr char kCharset[] = "abcdefghijklmnopqrstuvwxyz0123456789";
	std::string name = "mat_";
	for (int i = 0; i < 8; i++)
		name += kCharset[rand() % (sizeof(kCharset) - 1)];
	return name + "_" + suffix;
}

static CMaterial2* CreateChamsMaterial(const char* pszVmat, const char* szName)
{
	unsigned char buffer[0x1000] = {};
	KeyValues3* kv3 = reinterpret_cast<KeyValues3*>(buffer + 0x100);

	KV3ID_t kv3_id;
	kv3_id.name = "generic";
	kv3_id.unk0 = 0x41B818518343427Eull;
	kv3_id.unk1 = 0xB5F447C23C0CDF8Cull;

	if (!KeyValues3_LoadKV3(kv3, nullptr, pszVmat, &kv3_id, XorStr(""), 0))
	{
		DEV_LOG("KeyValues3_LoadKV3 failed for %s\n", szName);
		return nullptr;
	}

	CMaterial2** pHandle = nullptr;
	CMaterialSystem2_CreateMaterial(reinterpret_cast<CMaterial2**>(&pHandle), szName, kv3);

	if (!pHandle || !*pHandle)
	{
		DEV_LOG("CreateMaterial failed for %s\n", szName);
		return nullptr;
	}

	return *pHandle;
}

static CMaterial2* CreateChamsMaterialNamed(const char* suffix)
{
	std::string name = GenerateRandomName(suffix);
	return CreateChamsMaterial(kVmatFmt, name.c_str());
}

static void InitMaterials()
{
	if (g_bInitDone)
		return;
	g_bInitDone = true;

	srand(static_cast<unsigned>(GetTickCount64() ^ reinterpret_cast<uintptr_t>(&g_bCreated)));

	g_pChamsEnemy = CreateChamsMaterialNamed("enemy");
	g_pChamsTeam  = CreateChamsMaterialNamed("team");

	g_bCreated = true;
}

// ── Owner resolver ────────────────────────────────────────────────────────────
//
// CSceneAnimatableObject::hOwner (+0xC0) holds the entity CHandle.
// Get<C_BaseEntity>() resolves it through the entity system.
//

static C_CitadelPlayerPawn* GetPawnFromSceneObject(CSceneAnimatableObject* pSceneObj)
{
	if (!pSceneObj)
		return nullptr;

	CHandle& hOwner = pSceneObj->hOwner();
	if (!hOwner.IsValid())
		return nullptr;

	auto* pEntity = hOwner.Get<C_BaseEntity>();
	if (!pEntity || !pEntity->IsCitadelPlayerPawn())
		return nullptr;

	return reinterpret_cast<C_CitadelPlayerPawn*>(pEntity);
}

// ── Hook ──────────────────────────────────────────────────────────────────────

auto __fastcall Hook_DrawModel(
	__int64 pAnimatableSceneObjectDesc,
	__int64 pDx11,
	__int64* arrMeshDraw,
	int numMeshes,
	__int64 pSceneView,
	__int64 pSceneLayer,
	__int64 a7) -> void**
{
	auto* pChams = &GetVisual()->m_Chams;

	const bool bEnemy = pChams->Enemy.Enabled;
	const bool bTeam = pChams->Team.Enabled;

	if (!bEnemy && !bTeam)
		return DrawModel_o(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, numMeshes, pSceneView, pSceneLayer, a7);

	if (!g_bCreated)
		InitMaterials();

	if (!arrMeshDraw || numMeshes <= 0)
		return DrawModel_o(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, numMeshes, pSceneView, pSceneLayer, a7);

	auto* pLocal = GetCL_CitadelPlayerPawn()->GetLocal();

	static constexpr int kMaxMeshes = 256;
	const int meshCount = numMeshes < kMaxMeshes ? numMeshes : kMaxMeshes;

	struct SavedMat_t
	{
		CMaterial2** ppSlot;
		CMaterial2* pOrig;
	};

	int savedCount = 0;
	SavedMat_t saved[kMaxMeshes];

	for (int i = 0; i < meshCount; ++i)
	{
		auto* pEntry = CMeshDrawEntry::Get(arrMeshDraw, i);
		if (!pEntry->pMeshData)
			continue;

		CMaterial2* pMat = pEntry->pMaterial;
		CMaterial2** ppSlot = &pEntry->pMaterial;

		if (!pMat)
			continue;

		// Resolve owner: CSceneAnimatableObject → hOwner (+0xC0) → C_CitadelPlayerPawn
		auto* pPawn = GetPawnFromSceneObject(pEntry->pSceneObject);
		if (!pPawn || pPawn == pLocal)
			continue;

		// Pick material by team membership
		CMaterial2* pOverride = nullptr;
		const bool bSameTeam = pLocal && (pPawn->m_iTeamNum() == pLocal->m_iTeamNum());

		if (bSameTeam && bTeam && g_pChamsTeam)
			pOverride = g_pChamsTeam;

		if (!bSameTeam && bEnemy && g_pChamsEnemy)
			pOverride = g_pChamsEnemy;

		if (!pOverride)
			continue;

		saved[savedCount++] = {ppSlot, pMat};
		*ppSlot = pOverride;
	}

	auto* result = DrawModel_o(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, numMeshes, pSceneView, pSceneLayer, a7);
	for (int i = 0; i < savedCount; ++i)
		*saved[i].ppSlot = saved[i].pOrig;

	return result;
}
