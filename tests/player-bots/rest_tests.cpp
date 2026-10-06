#include "player_bot_rest.h"
#include <iostream>
#include <stdexcept>
int main() {
 int checks=0;
 auto check=[&](bool good){++checks;if(!good)throw std::runtime_error("Rest regression");};
 PlayerBotRestGate g;PlayerBotRestGate::Position owner{0,0,0},bot{10,0,0};
 check(!g.Update(0,owner,bot,false,true));
 check(!g.Update(1999,owner,bot,false,true));
 check(g.Update(2000,owner,bot,false,true));
 // Player walks while the bot pauses between route updates.
 for(uint64_t t=2050;t<8050;t+=50){owner[0]+=1;check(!g.Update(t,owner,bot,false,true));}
 check(!g.Update(8500,owner,bot,false,true));
 check(g.Update(10000,owner,bot,false,true));
 // Player stops, but bot is still catching up.
 bot[0]+=1;check(!g.Update(10050,owner,bot,false,true));
 check(!g.Update(11999,owner,bot,false,true));
 check(g.Update(12050,owner,bot,false,true));
 // Movement signal arrives before the next position update.
 check(!g.Update(12100,owner,bot,true,true));
 check(!g.Update(14099,owner,bot,false,true));
 check(g.Update(14100,owner,bot,false,true));
 // Vertical movement, including jumps/falling, also prevents sitting.
 owner[2]+=1;check(!g.Update(14150,owner,bot,false,true));
 bot[2]+=1;check(!g.Update(14200,owner,bot,false,true));
 check(!g.Update(16200,owner,bot,false,false)); // combat/casting/not at rest location
 check(!g.Update(18199,owner,bot,false,true));
 check(g.Update(18200,owner,bot,false,true));
 g.Reset(18250);check(!g.Update(19000,owner,bot,false,true)); // movement starts during NPC::Process
 check(g.Update(20250,owner,bot,false,true));
 check(!g.Update(1,owner,bot,false,true)); // clock reset
 PlayerBotRestGate slow;owner={0,0,0};check(!slow.Update(0,owner,bot,false,true));
 for(uint64_t t=100;t<=3000;t+=100){owner[0]+=.02f;check(!slow.Update(t,owner,bot,false,true));}
 std::cout<<checks<<" rest/movement assertions passed\n";
}
