#pragma once

#include "Actor.h"
#include <atomic>
#include <memory>
#include <type_traits>

class ActorFactory
{
    public:
        ActorFactory();
        ~ActorFactory() = default;

        template <typename T>
        static std::shared_ptr<T> Create(int32 templateId = 0)
        {
            static_assert(std::is_base_of_v<Actor, T>);
            std::shared_ptr<T> actor = std::make_shared<T>();
            actor->Init(GenerateId(actor->GetActorType(), templateId));

            return actor;
        }

        static int64 GenerateId(Protocol::EActorType type, int32 templateId)
        {
            return ((int32)type << 28) | (templateId << 20) |
                   (_counter.fetch_add(1));
        }

    private:
        inline static std::atomic<int64> _counter = 0;
};
