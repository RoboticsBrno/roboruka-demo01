#include <esp_log.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>

#include <Arduino.h>

#include "rbprotocol.h"
#include "rbwebserver.h"

#include "RBControl_manager.hpp"
#include "RBControl_battery.hpp"
#include "RBControl_wifi.hpp"
#include "RBControl_arm.hpp"

#include "motors.hpp"
#include "roboruka.hpp"

#include "config.hpp"


static_assert(sizeof(OWNER) > 1, "The OWNER config variable is not set, change it in config.hpp!");
static_assert(sizeof(NAME) > 1, "The NAME config variable is not set, change it in config.hpp!");

using namespace rb;

std::unique_ptr<Arm> gArm;
std::unique_ptr<Protocol> gProtocol;

static void handleCommand(const std::string& command, rbjson::Object* pkt);
static void main_program();

void loop() { }
void setup() {
    auto& man = Manager::get();

    // Build the arm definition
    gArm = roborukaBuildArm();

    // Initialize misc robot stuff
    roborukaSetup();

    // =============================  !!!!README!!!  =============================
    // Set the battery measuring coefficient.
    // Measure voltage at battery connector and
    // coef = voltageMeasureAtBatteriesInMilliVolts / raw
    auto& batt = man.battery();
    batt.setCoef(9.185f);

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

    // Initialize the communication protocol
    gProtocol.reset(new Protocol(OWNER, NAME, "Compiled at " __DATE__ " " __TIME__, handleCommand));
    gProtocol->start();

    main_program();
}

void handleCommand(const std::string& command, rbjson::Object* pkt) {
    if(command == "joy") {
        motorsHandleJoysticks(pkt);
    } else if(command == "arm") {
        const double x = pkt->getDouble("x");
        const double y = pkt->getDouble("y");
        gArm->solve(x, y);

        auto& servos = Manager::get().servoBus();
        for(const auto& b : gArm->bones()) {
            servos.set(b.def.servo_id, b.servoAng() + Angle::deg(BONE_TRIMS[b.def.servo_id]), 200);
        }
    } else if(command == "grab") {
        auto &servos = Manager::get().servoBus();
        const bool isGrabbing = servos.posOffline(2).deg() < 140;
        const auto angle = !isGrabbing ? 75_deg : 160_deg;
        servos.set(2, angle + Angle::deg(BONE_TRIMS[2]), 200.f, 1.f);
    } else if(command == "arminfo") {
        roborukaSendArmInfo(*gProtocol, gArm->definition());
    }
}

void main_program() {
    auto& batt = Manager::get().battery();

    printf("%s's roboruka '%s' started!\n", OWNER, NAME);
    printf("\n\nBATTERY CALIBRATION INFO: %d (raw) * %.2f (coef) = %dmv\n\n\n", batt.raw(), batt.coef(), batt.voltageMv());

    int iter = 0;
    while(true) {
        if(gProtocol->is_possessed()) {
            // Send text to the android application
            gProtocol->send_log("Tick #%d, battery at %d%%, %dmv\n", iter++, batt.pct(), batt.voltageMv());
        }
        sleep(1);
    }
}
