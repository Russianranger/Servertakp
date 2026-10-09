// NPC trade-window equipment exchange. All offered slots commit together.
bool TradePlayerBot(Client *c,PlayerBot *bot)
{
 auto reject=[&](){c->Message(Chat::White,"Bot trade returned: use compatible equipment, no bags/custom data, no combat, and resolve lore/weapon conflicts first.");c->FinishTrade(c);return true;};
 if(!PlayerBotDatabaseReady(c))return reject();
 if(!CanTradePlayerBot(c,bot))return reject();
 struct Swap {int from,to;uint32 item;int charges;std::unique_ptr<EQ::ItemInstance> returned;};std::vector<Swap> plan;
 if(!BotQuery("START TRANSACTION").Success())return reject();
 auto abort=[&](){BotQuery("ROLLBACK");return reject();};
 auto owner=BotQuery(fmt::format("SELECT id FROM takp_bot_data WHERE id={} AND owner_character_id={} FOR UPDATE",bot->ID(),c->CharacterID()));
 auto lease=BotQuery(fmt::format("SELECT generation FROM takp_bot_runtime WHERE bot_id={} FOR UPDATE",bot->ID()));
 if(!lease.Success()||lease.RowCount()!=1||strtoul(lease.begin()[0],nullptr,10)!=bot->Generation())return abort();
 auto worn=BotQuery(fmt::format("SELECT slot,item_id,charges FROM takp_bot_inventory WHERE bot_id={} FOR UPDATE",bot->ID()));
 auto offered=BotQuery(fmt::format("SELECT slotid,itemid,charges,custom_data FROM character_inventory WHERE id={} AND slotid BETWEEN {} AND {} FOR UPDATE",c->CharacterID(),EQ::invslot::TRADE_BEGIN,EQ::invslot::TRADE_END));
 if(!owner.Success()||owner.RowCount()!=1||!worn.Success()||!offered.Success())return abort();
 std::map<int,std::pair<uint32,int>> final_items;std::set<int> used;
 for(auto r=worn.begin();r!=worn.end();++r)final_items[atoi(r[0])]={strtoul(r[1],nullptr,10),atoi(r[2])};
 size_t count=0;
 for(int slot=EQ::invslot::TRADE_BEGIN;slot<=EQ::invslot::TRADE_END;++slot){
  const auto *item=c->GetInv().GetItem(slot);if(!item)continue;++count;
  if(!item->IsClassCommon()||!item->GetCustomDataString().empty()||!item->IsEquipable(bot->GetRace(),bot->GetClass())||item->GetItem()->ReqLevel>bot->GetLevel())return abort();
  bool matched=false;for(auto r=offered.begin();r!=offered.end();++r)if(atoi(r[0])==slot&&strtoul(r[1],nullptr,10)==item->GetID()&&atoi(r[2])==(item->GetCharges()<0?32767:item->GetCharges())&&(!r[3]||!*r[3]))matched=true;
  if(!matched)return abort();
  int chosen=-1;
  for(int replace=0;replace<2&&chosen<0;++replace)for(int dest=EQ::invslot::EQUIPMENT_BEGIN;dest<=EQ::invslot::EQUIPMENT_END;++dest){
   if(used.count(dest)||!item->IsEquipable(dest)||(item->IsStackable()&&dest!=EQ::invslot::slotAmmo)||(replace==0&&final_items.count(dest)))continue;
   if(dest==EQ::invslot::slotSecondary&&item->IsWeapon()&&!bot->GetSkill(EQ::skills::SkillDualWield))continue;
   chosen=dest;break;
  }
  if(chosen<0)return abort();
  Swap swap{slot,chosen,item->GetID(),item->GetCharges()<0?32767:item->GetCharges(),nullptr};
  if(final_items.count(chosen)){auto old=final_items[chosen];swap.returned.reset(database.CreateItem(old.first,old.second==32767?-1:old.second));if(!swap.returned||c->CheckLoreConflict(swap.returned->GetItem()))return abort();}
  final_items[chosen]={swap.item,swap.charges};used.insert(chosen);plan.push_back(std::move(swap));
 }
 if(count!=offered.RowCount())return abort();
 std::set<uint32> lore;
 for(auto &slot:final_items){const auto *item=database.GetItem(slot.second.first);if(!item)return abort();if((item->Lore[0]=='*'||item->Lore[0]=='#')&&!lore.insert(item->ID).second)return abort();if(slot.first==EQ::invslot::slotPrimary&&item->IsType2HWeapon()&&final_items.count(EQ::invslot::slotSecondary))return abort();}
 for(auto &s:plan){
  if(!BotQuery(fmt::format("INSERT INTO takp_bot_inventory VALUES({},{},{},{}) ON DUPLICATE KEY UPDATE item_id=VALUES(item_id),charges=VALUES(charges)",bot->ID(),s.to,s.item,s.charges)).Success())return abort();
  auto query=s.returned?fmt::format("UPDATE character_inventory SET itemid={},charges={},custom_data='' WHERE id={} AND slotid={}",s.returned->GetID(),s.returned->GetCharges()<0?32767:s.returned->GetCharges(),c->CharacterID(),s.from):fmt::format("DELETE FROM character_inventory WHERE id={} AND slotid={}",c->CharacterID(),s.from);
  if(!BotQuery(query).Success())return abort();
 }
 if(!BotQuery("COMMIT").Success()){
  BotQuery("ROLLBACK");
  PlayerBotTradeRecovery::DiscardStaleOffers(c,EQ::invslot::TRADE_BEGIN,EQ::invslot::TRADE_END);
  bot->trade->Reset();
  c->Kick("Bot trade result uncertain; reconnect to reload saved inventory.");
  return false;
 }
 for(auto &s:plan){c->DeleteItemInInventory(s.from,0,false,false);if(s.returned)c->GetInv().PutItem(s.from,*s.returned);}
 bot->LoadEquipment();bot->Save(true);c->FinishTrade(c);c->Message(Chat::White,"Bot equipment saved. Replaced equipment and all coins were returned.");return true;
}
