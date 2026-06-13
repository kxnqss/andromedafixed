#pragma once

#include <cstdio>
#include <GameClient/CL_Bones.hpp>

// ── Aimbot bone mask constants ────────────────────────────────────────────────
// Each bit corresponds to a HitboxSlot:  mask = 1 << static_cast<int>(slot).
// SetBit(mask, i) in the menu uses the loop index i which equals the slot int,
// so the bit position and slot value always stay in sync.

inline constexpr int kBoneMaskHead  = 1 << static_cast<int>(HitboxSlot::Head);   // 0x01
inline constexpr int kBoneMaskNeck  = 1 << static_cast<int>(HitboxSlot::Neck);   // 0x02
inline constexpr int kBoneMaskTorso = 1 << static_cast<int>(HitboxSlot::Torso);  // 0x04
inline constexpr int kBoneMaskArms  = 1 << static_cast<int>(HitboxSlot::Arms);   // 0x08
inline constexpr int kBoneMaskLegs  = 1 << static_cast<int>(HitboxSlot::Legs);   // 0x10
inline constexpr int kBoneMaskAll   = kBoneMaskHead | kBoneMaskNeck | kBoneMaskTorso | kBoneMaskArms | kBoneMaskLegs;
inline constexpr int kBoneCount     = static_cast<int>(HitboxSlot::Count);        // 5

// ── Bone entry ────────────────────────────────────────────────────────────────

struct AimBoneEntry
{
	int        mask;   // bit in BonesMask
	HitboxSlot slot;   // hitbox slot to query via GetCL_Bones()->GetHitboxBones()
	const char* label; // UI display label
};

// Ordered head → neck → torso → arms → legs.
// ScanTargets iterates this in order: the first slot with a trace-visible bone
// wins as the aim point, which is why head is listed first.
inline constexpr AimBoneEntry k_AimBones[kBoneCount] =
{
	{ kBoneMaskHead,  HitboxSlot::Head,  "Head"  },
	{ kBoneMaskNeck,  HitboxSlot::Neck,  "Neck"  },
	{ kBoneMaskTorso, HitboxSlot::Torso, "Torso" },
	{ kBoneMaskArms,  HitboxSlot::Arms,  "Arms"  },
	{ kBoneMaskLegs,  HitboxSlot::Legs,  "Legs"  },
};

// ── Helpers ───────────────────────────────────────────────────────────────────

// Returns true when mask represents "Head only" — a state disallowed while
// Anti-Frog is active (it would be immediately redirected to Neck, which
// makes the setting misleading to the user).
inline bool IsBonesMaskOnlyHead(int mask)
{
	return mask == kBoneMaskHead;
}

// Writes a human-readable preview of the active bones into buf
// (e.g. "Head, Neck").  buf must be at least 64 bytes.
inline void FormatBonesMaskPreview(int mask, char* buf, int bufSz)
{
	int  pos   = 0;
	bool first = true;

	for (int i = 0; i < kBoneCount; ++i)
	{
		if (!(mask & k_AimBones[i].mask))
			continue;

		if (!first)
			pos += snprintf(buf + pos, bufSz - pos, ", ");

		pos += snprintf(buf + pos, bufSz - pos, "%s", k_AimBones[i].label);
		first = false;
	}

	if (first)
		snprintf(buf, bufSz, "None");
}
