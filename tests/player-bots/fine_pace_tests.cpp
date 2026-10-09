#include "player_bot_follow.h"
#include <cassert>
#include <iostream>
int main() {
 using namespace BotFollow;
 for(float speed:{18.8f,29.2f,30.f,45.f}) {
  float owner=0,bot=-20;PaceFilter filter;int lo=1000,hi=0;
  for(unsigned t=0;t<30000;t+=20) {
   int pace=filter.Update(t,Pace(56,speed,owner-bot-20,true,true));
   if(t>5000){lo=std::min(lo,pace);hi=std::max(hi,pace);}
   owner+=speed*.02f;bot+=pace*.58f*.02f;
   assert(std::abs(owner-bot-20)<.6f);
  }
  assert(hi-lo<=1);
  assert(std::abs(Pace(100,speed,0,true,false)*.58f-speed)<=.291f);
  std::cout<<speed<<" units/sec: settled speed range "<<lo*.58f<<" to "<<hi*.58f<<"\n";
 }
 for(float speed:{0.f,18.8f,29.2f,100.f}) for(float error:{-30.f,0.f,30.f}) {
  assert(Pace(48,speed,error,true,false)<=48);
  assert(Pace(0,speed,error,true,true)==0);
 }
 std::cout<<"Fine pace steady-follow, spacing and disabled-boost checks passed\n";
}
