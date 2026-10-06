#include "player_bot_follow.h"
#include <cassert>
#include <iostream>
int main(){using namespace BotFollow;int checks=0;
for(unsigned interval:{200u,500u,1000u}) {
 Motion m;float bot=-10;bool active=false;
 for(unsigned t=0;t<30000;t+=50){float report=30.f*(t/interval)*interval/1000.f;m.Update(t+1000,report,0,1.5f,0);
  assert(std::abs(m.speed-30)<.01);assert(m.Moving(t+1000));assert(std::abs(m.X(t+1000)-30.f*t/1000.f)<.01);checks+=3;
  float gap=m.X(t+1000)-bot;active=ShouldMove(gap,10,true,active);if(active)bot+=Pace(48,m.speed,gap-10,true,true)*.58f*.05f;
  assert(gap<14 && gap>3);++checks;
 }
 m.Update(31000,900,0,0,0);assert(!m.Moving(31000)&&m.speed==0&&m.X(32000)==900);++checks;
}
Motion stale;stale.Update(1000,0,0,1.5,0);assert(stale.X(2100)<=33.01);assert(!stale.Moving(2600));assert(stale.X(2600)==0);checks+=3;
Motion turn;turn.Update(1000,0,0,1.5,0);turn.Update(2000,30,0,0,1.5);assert(turn.X(2500)==30&&std::abs(turn.Y(2500)-15)<.01);++checks;
Motion teleport;teleport.Update(1000,0,0,1.5,0);teleport.Update(2000,1000,0,1.5,0);assert(teleport.speed<=30.01);++checks;
assert(Pace(48,60,20,true,false)<=48);assert(Pace(48,60,20,true,true)>48);assert(Pace(0,60,20,true,true)==0);checks+=3;
std::cout<<checks<<" sparse-report prediction, pace, turns, stop and stale-input checks passed\n";
}
