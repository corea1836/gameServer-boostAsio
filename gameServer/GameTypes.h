#pragma once

#include "protobuf/Enum.pb.h"
#include "protobuf/Struct.pb.h"
#include <functional>
#include <memory>

class Vector3;

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

struct MoveData
{
        float direction_x = 0, direction_y = 0, direction_z = 0;
        float yaw = 0;
        float position_x = 0, position_y = 0;
        Protocol::EMoveState move_state =
            Protocol::EMoveState::EMOVE_STATE_STOP;

        Protocol::MoveData ToProto() const
        {
            Protocol::MoveData data;
            data.set_direction_x(direction_x);
            data.set_direction_y(direction_y);
            data.set_direction_z(direction_z);
            data.set_yaw(yaw);
            data.set_position_x(position_x);
            data.set_position_y(position_y);
            data.set_move_state(move_state);

            return data;
        }
};
