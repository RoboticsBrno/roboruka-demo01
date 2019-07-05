#pragma once

#include "RBControl_pinout.hpp"

namespace rbjson {
    class Object;
};

#define MOTOR_LEFT rb::MotorId::M2
#define MOTOR_RIGHT rb::MotorId::M1


void motorsHandleJoysticks(rbjson::Object *pkt);