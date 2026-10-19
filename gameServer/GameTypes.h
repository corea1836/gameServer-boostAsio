#pragma once

#include "MathUtils.h"
#include "Types.h"
#include "protobuf/Enum.pb.h"
#include "protobuf/Struct.pb.h"
#include <functional>
#include <memory>

using CallbackFunc = std::function<void()>;

#define USING_SHARED_PTR(name) using name##Ref = std::shared_ptr<class name>
USING_SHARED_PTR(Actor);
USING_SHARED_PTR(Channel);
USING_SHARED_PTR(GameSession);
USING_SHARED_PTR(JobSerializer);
USING_SHARED_PTR(Player);
USING_SHARED_PTR(World);

#define USING_WEAK_PTR(name) using name##WeakRef = std::weak_ptr<class name>
USING_WEAK_PTR(Channel);
USING_WEAK_PTR(GameSession);
USING_WEAK_PTR(World);

#define USING_UNIQUE_PTR(name)                                                 \
    using name##UniqueRef = std::unique_ptr<class name>
USING_UNIQUE_PTR(GameLoop);

struct MoveInfo
{
        Vector3 direction;
        float yaw = 0;
        Vector3 currentPosition;
        Protocol::EMoveState move_state =
            Protocol::EMoveState::EMOVE_STATE_STOP;
        int64 lastUpdateTick = 0;
        bool lastMulticastMoving = false;

        Protocol::MoveData ToProto() const
        {
            Protocol::MoveData data;
            data.set_direction_x(direction.x);
            data.set_direction_y(direction.y);
            data.set_direction_z(direction.z);
            data.set_yaw(yaw);
            data.set_current_position_x(currentPosition.x);
            data.set_current_position_y(currentPosition.y);
            data.set_move_state(move_state);

            return data;
        }

        bool GetNeedMove() const
        {
            return move_state == Protocol::EMOVE_STATE_MOVE;
        }
};
