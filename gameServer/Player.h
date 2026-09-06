#pragma once

#include "Character.h"
#include "GameTypes.h"
#include "protobuf/Enum.pb.h"

class Player : public Character
{
        using Super = Character;

    public:
        Player()
        {
            Super::GetActorData().actorType =
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

    protected:
        GameSessionWeakRef _session;
};
