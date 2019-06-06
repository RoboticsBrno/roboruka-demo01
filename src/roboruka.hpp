#pragma once

#include <memory>
#include "rbprotocol.h"

std::unique_ptr<rb::Arm> roborukaBuildArm();
void roborukaSendArmInfo(rb::Protocol& prot, const rb::Arm::Definition& def);


void roborukaSetup();

