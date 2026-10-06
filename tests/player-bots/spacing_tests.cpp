#include "player_bot_follow.h"
#include <cassert>
#include <iostream>
int main() {
 using namespace BotFollow;
 unsigned checks=0;
 for(float desired:{3.f,10.f,20.f,50.f}) for(bool moving:{false,true}) {
  for(float angle=0;angle<6.28f;angle+=.1f) {
   float bx=100*std::cos(angle),by=100*std::sin(angle);
   auto target=FollowTarget(bx,by,0,0,desired,moving);
   float expected=std::max(3.f,desired-(moving?2.f:0.f));
   assert(std::abs(std::hypot(target.x,target.y)-expected)<.001f);
   // Simulate even a long AI interval: the finite route cannot reach the owner.
   assert(target.x*bx+target.y*by>0); checks+=2;
  }
  assert(!ShouldMove(2,desired,moving,true)); ++checks;
 }
 assert(!ShouldMove(15,20,true,true));
 assert(!ShouldMove(15,20,false,true));
 assert(ShouldMove(30,20,true,false)); checks+=3;
 Motion m; m.Update(1000,0,0,1.5f,0);
 assert(m.X(2000)>0);
 m.Update(2001,5,0,0,0);
 auto stopped=FollowTarget(-30,0,m.X(2001),m.Y(2001),10,false);
 assert(stopped.x==-5 && stopped.y==0); checks+=2;
 PaceFilter f; f.Update(1000,96); f.Reset();
 assert(f.Update(1001,32)==32);
 assert(Pace(48,60,40,true,false)<=48);checks+=2;
 // Raising the requested spacing cancels movement when already inside it.
 auto changed=FollowTarget(-15,0,0,0,20,true);
 assert(changed.x==-15); ++checks;
 for(float speed:{15.f,30.f,45.f}) {
  float bot=-20,endpoint=bot,owner=0;PaceFilter filter;int pauses=0;
  for(unsigned t=0;t<10000;t+=20) {
   owner=speed*t/1000.f;
   if(t%200==0) endpoint=FollowTarget(bot,0,owner,0,20,true,speed).x;
   float pace=filter.Update(t,Pace(48,speed,owner-bot-20,true,true))*.58f;
   if(endpoint<=bot) ++pauses;
   bot+=std::min(std::max(0.f,endpoint-bot),pace*.02f);
   assert(owner-bot>17 && owner-bot<23);++checks;
  }
  assert(pauses==0);++checks;
 }
 std::cout<<checks<<" spacing, stop, setting-change and speed-cap checks passed\n";
}
