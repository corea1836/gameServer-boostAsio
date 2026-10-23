#pragma once

#include "Character.h"
#include "GameTypes.h"
#include "protobuf/Enum.pb.h"
#include <unordered_set>

class Player : public Character
{
        using Super = Character;

    public:
        Player()
        {
            Super::GetActorInfo().actorType =
                Protocol::EActorType::EACTOR_TYPE_PLAYER;
        }
        ~Player() override;

        Protocol::PlayerData ToProto() const
        {
            Protocol::PlayerData data;
            *data.mutable_character_data() = Super::ToProto();

            return data;
        }

        void Bind(GameSessionRef session);

        GameSessionRef GetSession() { return _session.lock(); }

        // AOI: 지금 보고 있는 플레이어 id. World 큐에서만 읽고 쓴다(ADR-022)
        std::unordered_set<int64> &GetVisible() { return _visible; }

    protected:
        GameSessionWeakRef _session;
        std::unordered_set<int64> _visible;
};
