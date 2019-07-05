#include <memory>
#include "RBControl.hpp"
#include "RBControl_arm.hpp"
#include "RBControl_wifi.hpp"
#include "rbprotocol.h"
#include "rbwebserver.h"

#include "roboruka.hpp"
#include "motors.hpp"

#include "config.hpp"

using namespace rb;

void roborukaSetup() {
    // Initialize the robot manager
    auto& man = Manager::get();
    man.install();

    // Set the battery measurement coeficient
    auto& batt = man.battery();
    batt.setCoef(BATTERY_COEFFICIENT);

    // Set motor power limits
    man.setMotors()
        .pwmMaxPercent(MOTOR_LEFT, 100)
        .pwmMaxPercent(MOTOR_RIGHT, 100)
        .set();

    // Set-up servos
    auto& servos = man.initSmartServoBus(3);
    if(!servos.posOffline(2).isNaN())
        servos.setAutoStop(2);
    servos.limit(0,  0_deg, 220_deg);
    servos.limit(1, 85_deg, 210_deg);
    servos.limit(2, 75_deg, 160_deg);

    #ifndef WIFI_DEFAULT_AP
    const auto wifi_checkval = 0;
#else
    const auto wifi_checkval = 1;
#endif
    if(man.expander().digitalRead(SW1) != wifi_checkval) {
        man.leds().yellow();
        WiFi::connect(WIFI_NAME, WIFI_PASSWORD);
    } else {
        man.leds().green();
        WiFi::startAp(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL);
    }

    // Start web server with control page (see data/index.html)
    rb_web_start(80);
}

std::unique_ptr<Arm> roborukaBuildArm() {
    ArmBuilder builder;
    builder.body(60, 110).armOffset(0, 20);

    auto b0 = builder.bone(0, 110);
    b0.relStops(-95_deg, 0_deg);
    b0.calcServoAng([](Angle absAngle, Angle) -> Angle {
        return Angle::Pi + absAngle + 30_deg;
    });
    b0.calcAbsAng([](Angle servoAng) -> Angle {
        return servoAng - Angle::Pi - 30_deg;
    });

    auto b1 = builder.bone(1, 130);
    b1.relStops(30_deg, 170_deg)
        .absStops(-20_deg, Angle::Pi)
        .baseRelStops(40_deg, 160_deg);
    b1.calcServoAng([](Angle absAngle, Angle) -> Angle {
        absAngle = Arm::clamp(absAngle + Angle::Pi*1.5);
        return Angle::Pi + absAngle + 25_deg;
    });
    b1.calcAbsAng([](Angle servoAng) -> Angle {
        auto a = servoAng - Angle::Pi - 25_deg;
        return Arm::clamp(a - Angle::Pi*1.5);
    });

    return builder.build();
}

void roborukaSendArmInfo(Protocol& prot, const Arm::Definition& def) {
    auto info = std::make_unique<rbjson::Object>();
    info->set("height", def.body_height);
    info->set("radius", def.body_radius);
    info->set("off_x", def.arm_offset_x);
    info->set("off_y", def.arm_offset_y);

    auto *bones = new rbjson::Array();
    info->set("bones", bones);

    auto& servo = Manager::get().servoBus();
    for(const auto& b : def.bones) {
        auto pos = servo.pos(b.servo_id);
        if(pos.isNaN()) {
            while(bones->size() != 0)
                bones->remove(bones->size()-1);
            break;
        }

        pos -= Angle::deg(BONE_TRIMS[b.servo_id]);

        auto *info_b = new rbjson::Object();
        info_b->set("len", b.length);
        info_b->set("angle", b.calcAbsAng(pos).rad());
        info_b->set("rmin", b.rel_min.rad());
        info_b->set("rmax", b.rel_max.rad());
        info_b->set("amin", b.abs_min.rad());
        info_b->set("amax", b.abs_max.rad());
        info_b->set("bmin", b.base_rel_min.rad());
        info_b->set("bmax", b.base_rel_max.rad());
        bones->push_back(info_b);
    }
    prot.send_mustarrive("arminfo", info.release());
}
