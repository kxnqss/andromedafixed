#pragma once

#include <Common/Common.hpp>
#include <DeadLock/SDK/Interface/CMaterialSystem2.hpp>
#include <DeadLock/SDK/Types/CEntityData.hpp>

// ── CMeshData (pointed to by CMeshDrawEntry::pMeshData) ───────────────────────

struct CMeshData
{
	void*                    vtable;                  // +0x00
	char                     pad_0x08[ 0x18 ];        // +0x08
	void*                    pSceneAnimatableObject;  // +0x20 (unused / null in current build)
	CMaterial2*              pMaterial;               // +0x28
	CMaterial2*              pMaterialCopy;           // +0x30
};

// ── CMeshDrawEntry (stride 0x68 in DrawModel's arrMeshDraw flat array) ────────

struct CMeshDrawEntry
{
	CMeshData*               pMeshData;     // +0x00
	__int64                  pad_0x08;      // +0x08
	__int64                  pad_0x10;      // +0x10
	CSceneAnimatableObject*  pSceneObject;  // +0x18 — typed scene object
	CMaterial2*              pMaterial;     // +0x20 — override target
	CMaterial2*              pMaterialCopy; // +0x28
	char                     pad_0x30[ 0x38 ];

	static CMeshDrawEntry* Get( __int64* arrMeshDraw , int index )
	{
		return reinterpret_cast<CMeshDrawEntry*>(
			reinterpret_cast<uintptr_t>( arrMeshDraw ) + index * sizeof( CMeshDrawEntry ) );
	}
};

// ── Hook declaration ──────────────────────────────────────────────────────────

auto __fastcall Hook_DrawModel(
	__int64  pAnimatableSceneObjectDesc ,
	__int64  pDx11                      ,
	__int64* arrMeshDraw                ,
	int      numMeshes                  ,
	__int64  pSceneView                 ,
	__int64  pSceneLayer                ,
	__int64  a7 ) -> void**;

using DrawModel_t = decltype( &Hook_DrawModel );
inline DrawModel_t DrawModel_o = nullptr;
