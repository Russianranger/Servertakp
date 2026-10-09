#include "player_bot_rest.h"
#include "player_bot_follow.h"
#include <cassert>
#include <iostream>
int main(){
 for(unsigned distance:{100u,400u,2500u}) {
  float range=std::sqrt(float(distance));
  assert(PlayerBotRestLocation(true,(range+.5f)*(range+.5f),distance,false));
  assert(!PlayerBotRestLocation(true,(range+3)*(range+3),distance,false));
 }
 assert(PlayerBotRestLocation(false,100000,100,false));
 assert(PlayerBotRestLocation(true,100000,100,true));
 assert(PlayerBotBaseManaRegen(false,false,200)==1);
 assert(PlayerBotBaseManaRegen(true,false,0)==2);
 assert(PlayerBotBaseManaRegen(true,false,1)==3);
 assert(PlayerBotBaseManaRegen(true,false,15)==5);
 assert(PlayerBotBaseManaRegen(true,false,252)==20);
 assert(PlayerBotBaseManaRegen(true,true,252)==2);
 // A last velocity report is not permanent proof of movement.
 BotFollow::Motion motion;motion.Update(1000,0,0,1.5,0);
 PlayerBotRestGate gate;PlayerBotRestGate::Position p{0,0,0},b{-20,0,0};
 bool resting=false;
 for(unsigned t=1000;t<=5000;t+=50){
  motion.Update(t,0,0,1.5,0);
  resting=gate.Update(t,p,b,motion.Moving(t),PlayerBotRestLocation(true,400,400,false));
  if(t<3000)assert(!resting);
 }
 assert(resting);
 p[0]=1;assert(!gate.Update(5050,p,b,false,true));
 std::cout<<"Rest-distance, stale motion, movement cancellation and meditation checks passed\n";
}
