#pragma once

#include "GameTypes.h"
#include "MathUtils.h"
#include "protobuf/Enum.pb.h"
#include "protobuf/Struct.pb.h"
#include <Types.h>

struct PositionInfo
{
        Vector3 position;
        float yaw = 0.f;

        Protocol::PositionData ToProto() const
        {
            Protocol::PositionData data;
            data.set_x(position.x);
            data.set_y(position.y);
            data.set_z(position.z);
            data.set_yaw(yaw);
            return data;
        }
};

struct ActorInfo
{
        int64 actorId = 0;
        Protocol::EActorType actorType = Protocol::EActorType::EACTOR_TYPE_NONE;
        Protocol::EActorState actorState =
            Protocol::EActorState::EACTOR_STATE_NONE;
        int32 speed = 5;
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
            data.set_actor_id(_actorInfo.actorId);
            data.set_actor_type(_actorInfo.actorType);
            data.set_actor_state(_actorInfo.actorState);
            *data.mutable_position_data() = _currentPositionInfo.ToProto();
            *data.mutable_move_data() = _moveInfo.ToProto();
            return data;
        }

        void EnterWorld(WorldRef newWorld) { _world = newWorld; }
        void LeaveWorld() { _world.reset(); }

        const ActorInfo &GetActorInfo() const { return _actorInfo; }
        ActorInfo &GetActorInfo() { return _actorInfo; }
        const virtual Protocol::EActorType &GetActorType() const
        {
            return _actorInfo.actorType;
        }
        const virtual Protocol::EActorState &GetActorState() const
        {
            return _actorInfo.actorState;
        }
        const PositionInfo &GetCurrentPositionInfo() const
        {
            return _currentPositionInfo;
        }
        PositionInfo &GetCurrentPositionInfo() { return _currentPositionInfo; }
        const MoveInfo &GetMoveInfo() const { return _moveInfo; }
        MoveInfo &GetMoveInfo() { return _moveInfo; }
        WorldRef GetWorld() const { return _world.lock(); }

    private:
        WorldWeakRef _world;
        ActorInfo _actorInfo;
        PositionInfo _currentPositionInfo;
        MoveInfo _moveInfo;
};
