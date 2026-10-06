#include "player_bot_follow.h"
#include <cassert>
#include <iostream>
int main(){using namespace BotFollow;
for(float speed:{15.f,30.f,45.f}){
float bot=-12,slot=-12,endpoint=-12;PaceFilter filter;int pauses=0;float worst=0;
for(unsigned t=0;t<20000;t+=20){float actual=speed*t/1000.f-12;
if(t%200==0){slot=actual;endpoint=slot+speed*SteeringSeconds;}
float error=slot-bot;int target=Pace(48,speed,error,true,true);int pace=filter.Update(t,target);
float step=pace*.58f*.02f;float remaining=endpoint-bot;
if(remaining<=.001f)++pauses;
bot+=std::min(std::max(0.f,remaining),step);
worst=std::max(worst,std::abs(bot-actual));
}
assert(pauses==0);assert(worst<6);std::cout<<"speed "<<speed<<": no endpoint pauses; max slot error "<<worst<<"\n";
}
PaceFilter f;assert(f.Update(1000,48)==48);assert(f.Update(1100,80)==48);assert(f.Update(1200,80)==56);assert(f.Update(1400,16)==48);assert(f.Update(1401,0)==0);
std::cout<<"Continuity and bounded pace-change tests passed\n";
}
