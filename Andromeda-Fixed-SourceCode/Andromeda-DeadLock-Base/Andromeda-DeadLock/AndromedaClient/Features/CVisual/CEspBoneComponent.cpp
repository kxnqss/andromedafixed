#include "CEspBoneComponent.hpp"

#include <DeadLock/SDK/Types/CEntityData.hpp>
#include <DeadLock/SDK/Math/Math.hpp>

#include <GameClient/CL_Bones.hpp>

#include <AndromedaClient/GUI/CMenuWidgets.hpp>
#include <AndromedaClient/Render/CRenderStackSystem.hpp>

// ── JSON helpers ──────────────────────────────────────────────────────────────

static void LoadBoneSettings(const rapidjson::Value& v, BoneEspSettings& s)
{
	if (!v.IsObject())
		return;
	GetBoolJson(v, XorStr("Enabled"), s.Enabled);
	GetColorJson(v, XorStr("Color"), s.Color);
}

static void SaveBoneSettings(JsonWriter& w, const BoneEspSettings& s)
{
	AddBoolJson(w, XorStr("Enabled"), s.Enabled);
	AddColorJson(w, XorStr("Color"), s.Color);
}

// ── Load / Save ───────────────────────────────────────────────────────────────

void CEspBoneComponent::Load(const rapidjson::Value& v)
{
	if (!v.HasMember(XorStr("Bones")))
		return;
	const auto& bones = v[XorStr("Bones")];

	if (bones.HasMember(XorStr("Enemy")))
		LoadBoneSettings(bones[XorStr("Enemy")], Enemy);
	if (bones.HasMember(XorStr("Team")))
		LoadBoneSettings(bones[XorStr("Team")], Team);
}

void CEspBoneComponent::Save(JsonWriter& w)
{
	w.String(XorStr("Bones"));
	w.StartObject();
	{
		w.String(XorStr("Enemy"));
		w.StartObject();
		SaveBoneSettings(w, Enemy);
		w.EndObject();
		w.String(XorStr("Team"));
		w.StartObject();
		SaveBoneSettings(w, Team);
		w.EndObject();
	}
	w.EndObject();
}

// ── Menu ──────────────────────────────────────────────────────────────────────

static void RenderBoneSide(BoneEspSettings& s, bool bEnemy)
{
	CMenuWidgets::RenderColorToggle({
		XorStr("Esp Bones"),
		s.Color,
		bEnemy ? 2 : 6,
		bEnemy ? XorStr("##Visual.EspBones") : XorStr("##Visual.EspTeamBones"),
		&s.Enabled
	});
}

void CEspBoneComponent::RenderEnemyMenu()
{
	RenderBoneSide(Enemy, true);
}

void CEspBoneComponent::RenderTeamMenu()
{
	RenderBoneSide(Team, false);
}

// ── Rendering ─────────────────────────────────────────────────────────────────

void CEspBoneComponent::OnRenderPlayer(const EspPlayerContext& ctx)
{
	const BoneEspSettings& s = ctx.bTeam ? Team : Enemy;
	if (!s.Enabled || !ctx.pPawn || !ctx.bAlive)
		return;

	const auto* pPairs = GetCL_Bones()->GetBonePairs(ctx.pPawn);
	if (!pPairs || pPairs->empty())
		return;

	auto* pNode = ctx.pPawn->m_pGameSceneNode();
	if (!pNode)
		return;

	auto* pSkel = pNode->GetSkeletonInstance();
	if (!pSkel)
		return;

	pSkel->CalcWorldSpaceBones(FLAG_ALL_BONE_FLAGS);
	const ImColor boneColor(s.Color[0], s.Color[1], s.Color[2], 1.f);

	for (const auto& pair : *pPairs)
	{
		Vector3 posA, posB;
		if (!pSkel->GetBonePosition(pair.idA, posA) || !pSkel->GetBonePosition(pair.idB, posB))
			continue;

		if (posA.IsZero() || posB.IsZero())
			continue;

		ImVec2 screenA, screenB;
		if (Math::WorldToScreen(posA, screenA) && Math::WorldToScreen(posB, screenB))
			GetRenderStackSystem()->DrawLine(screenA, screenB, boneColor, 2.f);
	}
}