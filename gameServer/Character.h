#pragma once

#include "Actor.h"

struct CharacterData
{
        string name;
        int32 level;
};

class Character : public Actor
{
        using Super = Actor;

    public:
        Character();
        ~Character() override;

        Protocol::CharacterData ToProto() const
        {
            Protocol::CharacterData data;
            data.set_name(_characterData.name);
            data.set_level(_characterData.level);
            *data.mutable_actor_data() = Super::ToProto();

            return data;
        }

        void BindStats(string name, int32 level);

    private:
        CharacterData _characterData;
};
