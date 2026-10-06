#include "player_bot_follow.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main(){for(int speed=8;speed<=256;speed+=8){float world=speed*.58f;float delta=BotFollow::PacketVelocity(speed);assert(std::abs(delta*20-world)<.0001f);
// Native wire velocities have 1/16-unit granularity. Check worst correction over 200 ms.
float encoded=std::round(delta*16)/16;assert(std::abs((encoded*20-world)*.2f)<=.126f);
}std::cout<<"32 server/client velocity agreement and wire-quantization cases passed\n";}
