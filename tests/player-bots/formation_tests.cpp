#include "player_bot_formation.h"
#include <iostream>
#include <stdexcept>
int checks = 0;
void check(bool value, const char* message) { ++checks; if (!value) throw std::runtime_error(message); }
int main() {
 using namespace BotFormation;
 for (unsigned n=1;n<=5;++n) {
  for (unsigned i=0;i<n;++i) {
   auto p=Slot(n,i); check(p.y<0,"Every slot must trail the player");
   for(unsigned j=0;j<i;++j) {auto q=Slot(n,j);check(std::hypot(p.x-q.x,p.y-q.y)>=10,"Bots need separation");}
   for(float angle : {0.f,Pi/2,Pi,3*Pi/2}) {
    auto r=Rotate(p,angle);check(std::abs(std::hypot(r.x,r.y)-std::hypot(p.x,p.y))<.001,"Rotation preserves spacing");
   }
  }
 }
 check(Slot(2,0).x<0&&Slot(2,1).x>0,"Two-bot left/right layout");
 check(Slot(3,2).x==0&&Slot(3,2).y<Slot(3,0).y,"Three-bot rear center");
 check(Slot(4,2).x<0&&Slot(4,3).x>0&&std::abs(Slot(4,2).x)<std::abs(Slot(4,0).x),"Four-bot narrow rear pair");
 check(Slot(5,2).x==0&&Slot(5,2).y<Slot(5,0).y&&Slot(5,2).y>Slot(5,3).y,"Five-bot staggered middle");
 check(Slot(5,3).x<0&&Slot(5,4).x>0&&Slot(5,3).y==Slot(5,4).y,"Five-bot rear pair");
 std::vector<uint32_t> slots;
 Reconcile(slots,{30,10,20});check(slots==std::vector<uint32_t>({10,20,30}),"Initial order deterministic");
 Reconcile(slots,{5,30,20,10});check(slots==std::vector<uint32_t>({10,20,30,5}),"Adding earlier ID does not shuffle existing slots");
 Reconcile(slots,{5,30,10});check(slots==std::vector<uint32_t>({10,30,5}),"Removing retains survivor order");
 Reconcile(slots,{5,30,10,5});check(slots.size()==3,"No duplicate slots");
 Reconcile(slots,{});check(slots.empty(),"No followers leaves no slots");
 Frame frame;frame.Update(0,0,0);frame.Update(0,0,128);check(frame.angle==0,"Turning in place preserves formation");
 frame.Update(1,0,64);check(frame.angle==0,"Packet jitter does not rotate formation");
 frame.Update(10,0,64);check(std::abs(frame.angle-Pi/6)<.001,"Sharp turn is smoothed");
 frame.Update(20,0,64);frame.Update(30,0,64);check(std::abs(frame.angle-Pi/2)<.001,"Moving formation catches up to direction");
 Frame west;west.Update(0,0,192);auto behind=Rotate(Slot(1,0),west.angle);check(behind.x>11.9,"TAKP heading uses 256 units");
 Clearance c;check(!c.Update(true)&&!c.Update(true)&&!c.Update(true)&&c.Update(true),"Wait for sustained clearance");
 check(!c.Update(false),"Obstacle falls back immediately");
 check(!c.Update(true)&&!c.Update(false)&&!c.Update(true)&&!c.Update(true)&&!c.Update(true)&&c.Update(true),"Intermittent clearance does not oscillate");
 check(CanPosition(true,false,false,false,false),"Idle follower can move");
 check(!CanPosition(false,false,false,false,false),"Stay respected");
 check(!CanPosition(true,true,false,false,false),"Combat respected");
 check(!CanPosition(true,false,true,false,false),"Casting respected");
 check(!CanPosition(true,false,false,true,false),"Suspension respected");
 check(!CanPosition(true,false,false,false,true),"Root respected");
 check(Settled(16,false)&&!Settled(16,true)&&Settled(9,true)&&!Settled(26,false),"Arrival deadband prevents jitter");
 std::cout << checks << " formation assertions passed\n";
}
