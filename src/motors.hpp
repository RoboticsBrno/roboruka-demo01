#pragma once

#include "RBControl_pinout.hpp"

namespace rbjson {
    class Object;
};

#define MOTOR_LEFT rb::MotorId::M1
#define MOTOR_RIGHT rb::MotorId::M6


void motorsHandleJoysticks(rbjson::Object *pkt);