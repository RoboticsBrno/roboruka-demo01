#include "RBControl_manager.hpp"
#include "rbprotocol.h"

#include "motors.hpp"

#include "config.hpp"

static int scale_range(float x, float omin, float omax, float nmin, float nmax) {
    return ((float(x) - omin) / (omax - omin)) * (nmax - nmin) - nmax;
}

static int scale_motors(float val) {
    return scale_range(val, RBPROTOCOL_AXIS_MIN, RBPROTOCOL_AXIS_MAX, -100.f, 100.f);
}

void motorsHandleJoysticks(rbjson::Object *pkt) {
    const rbjson::Array *data = pkt->getArray("data");

    if(data->size() < 1) {
        return;
    }

    auto builder = rb::Manager::get().setMotors();

    // Drive
    {
        const rbjson::Object *joy = data->getObject(0);
        int x = joy->getInt("x");
        int y = joy->getInt("y");

        if(x != 0)
            x = scale_motors(x);
        if(y != 0)
            y = scale_motors(y);

        int r = ((y - (x/1.5f)));
        int l = ((y + (x/1.5f)));
        if(r < 0 && l < 0) {
            std::swap(r, l);
        }

        l *= MOTOR_POLARITY_SWITCH_LEFT;
        r *= MOTOR_POLARITY_SWITCH_RIGHT;

        builder.power(MOTOR_LEFT, l).power(MOTOR_RIGHT, r);
    }

    builder.set();
}
