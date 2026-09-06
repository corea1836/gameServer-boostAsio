#pragma once

#include "GameTypes.h"
#include "protobuf/Enum.pb.h"
#include "protobuf/Struct.pb.h"
#include <Types.h>

struct ActorData
{
        int64 actorId = 0;
        Protocol::EActorType actorType = Protocol::EActorType::EACTOR_TYPE_NONE;
};

struct PositionData
{
        float x = 0;
        float y = 0;
        float z = 0;
        float yaw = 0.f;

        Protocol::PositionData ToProto() const
        {
            Protocol::PositionData data;
            data.set_x(x);
            data.set_y(y);
            data.set_z(z);
            data.set_yaw(yaw);
            return data;
        }
};

class Actor : public std::enable_shared_from_this<Actor>
{
    public:
        Actor();
        virtual ~Actor() = default;

        Actor(const Actor &) = delete;
        Actor &operator=(const Actor &) = delete;

        void Init(int64 id);

        Protocol::ActorData ToProto() const
        {
            Protocol::ActorData data;
            data.set_actor_id(_actorData.actorId);
            data.set_actor_type(_actorData.actorType);
            *data.mutable_position_data() = _positionData.ToProto();
            *data.mutable_move_data() = _moveData.ToProto();
            return data;
        }

        void EnterWorld(WorldRef newWorld) { _world = newWorld; }
        void LeaveWorld() { _world.reset(); }

        const ActorData &GetActorData() const { return _actorData; }
        ActorData &GetActorData() { return _actorData; }
        const virtual Protocol::EActorType &GetActorType() const
        {
            return _actorData.actorType;
        }
        const PositionData &GetPositionData() const { return _positionData; }
        PositionData &GetPositionData() { return _positionData; }
        const MoveData &GetMoveData() const { return _moveData; }
        MoveData &GetMoveData() { return _moveData; }
        WorldRef GetWorld() const { return _world.lock(); }

    private:
        WorldWeakRef _world;
        ActorData _actorData;
        PositionData _positionData;
        MoveData _moveData;
};
