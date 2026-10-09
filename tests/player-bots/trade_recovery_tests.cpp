#include "player_bot_trade_recovery.h"
#include <cassert>
#include <map>

struct Trade {
 unsigned cp=7,sp=3,gp=2,pp=1;
 bool reset=false;
 void Reset() { cp=sp=gp=pp=0; reset=true; }
};
struct Client {
 Trade session;
 Trade *trade=&session;
 std::map<int,int> memory{{3000,100},{3001,101}};
 std::map<int,int> durable{{3000,200}};
 unsigned money_calls=0,clears=0,cp=0,sp=0,gp=0,pp=0;
 void DeleteItemInInventory(int slot,int quantity,bool client_update,bool update_db) {
  assert(quantity==0 && !client_update && !update_db);
  memory.erase(slot);
  ++clears;
 }
 void AddMoneyToPP(unsigned copper,unsigned silver,unsigned gold,unsigned platinum,bool update) {
  assert(trade->reset && !update);
  cp+=copper;sp+=silver;gp+=gold;pp+=platinum;++money_calls;
 }
};
int main()
{
 Client client;
 PlayerBotTradeRecovery::DiscardStaleOffers(&client,3000,3007);
 assert(client.memory.empty());
 assert(client.durable.size()==1 && client.durable[3000]==200);
 assert(client.cp==7 && client.sp==3 && client.gp==2 && client.pp==1);
 assert(client.money_calls==1 && client.clears==8 && client.trade->reset);
 PlayerBotTradeRecovery::DiscardStaleOffers(&client,3000,3007);
 assert(client.money_calls==1); // Disconnect/fallback cannot refund twice.
}
