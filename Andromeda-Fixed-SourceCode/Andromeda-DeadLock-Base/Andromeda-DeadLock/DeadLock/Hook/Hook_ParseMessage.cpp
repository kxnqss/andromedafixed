#include "Hook_ParseMessage.hpp"

#include <DeadLock/SDK/SDK.hpp>
#include <DeadLock/SDK/Math/Vector3.hpp>

// [E] Stable — protobuf internal layout, no need to update after game patches.
// CDemoRecorder::ParseMessage stores the decoded protobuf at pNetMessage + 0x30.
static constexpr uintptr_t k_ParseMessage_pProtobuf = 0x30;
// CMsgSosStartSoundEvent::packed_params data — sound position float[3] at +0x12.
static constexpr uintptr_t k_SoundPos_in_PackedParams = 0x12;

#include <DeadLock/SDK/Network/CNetworkMessages.hpp>
#include <DeadLock/SDK/Interface/CSoundOpSystem.hpp>

#include <DeadLock/Protobuf/gameevents.pb.h>
#include <DeadLock/Protobuf/citadel_usermessages.pb.h>

#include <AndromedaClient/CAndromedaClient.hpp>
#include <AndromedaClient/Features/CAimbot/CAimbot.hpp>

auto Hook_ParseMessage(CDemoRecorder* pDemoRecorder, CNetworkSerializerPB* pSerializer, CNetMessagePB* pNetMessage) -> bool
{
	if (pSerializer->messageID == GE_SosStartSoundEvent)
	{
		CMsgSosStartSoundEvent* pMessage = reinterpret_cast<CMsgSosStartSoundEvent*>((PBYTE)pNetMessage + k_ParseMessage_pProtobuf);

		if (pMessage)
		{
			std::string SoundName = "null";
			Vector3 SoundPos;
			auto SourceEntityIndex = 0;

			if (pMessage->has_source_entity_index())
				SourceEntityIndex = pMessage->source_entity_index();

			if (pMessage->has_packed_params() && pMessage->packed_params().data())
			{
				SoundPos = *(Vector3*)(pMessage->packed_params().data() + k_SoundPos_in_PackedParams);

				const char* szSoundEventName = SDK::Interfaces::SoundOpSystem()->GetCSoundEventManager()->GetSoundEventName(
					pMessage->soundevent_hash());

				if (szSoundEventName)
					SoundName = szSoundEventName;

				GetAndromedaClient()->OnStartSound(SoundPos, SourceEntityIndex, SoundName.c_str());
			}
		}
	}
	else if (pSerializer->messageID == k_EUserMsg_Damage)
	{
		auto* pMsg = reinterpret_cast<CCitadelUserMessage_Damage*>((PBYTE)pNetMessage + k_ParseMessage_pProtobuf);

		if (pMsg && pMsg->has_entindex_attacker() && pMsg->has_entindex_victim() && pMsg->has_hitgroup_id())
			GetAimbot()->OnDamage(pMsg->entindex_attacker(), pMsg->entindex_victim(), pMsg->hitgroup_id());
	}

	return ParseMessage_o(pDemoRecorder, pSerializer, pNetMessage);
}
