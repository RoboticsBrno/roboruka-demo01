#pragma once

#include "RBControl_arm.hpp"

#include "rbprotocol.h"

rb::Arm *roborukaBuildArm();
void roborukaSendArmInfo(rb::Protocol& prot, const rb::Arm::Definition& def);


void roborukaSetup();
