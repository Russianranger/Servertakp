// Command helpers share the owner-scoped runtime in player_bot.cpp.
bool Supply(Client *c,PlayerBot *bot,int slot,int item_id,bool give)
{
 if(!PlayerBotDatabaseReady(c))return false;
 if(slot<22||slot>29||c->GetAggroCount()||bot->IsEngaged())return false;
 const auto *held=c->GetInv().GetItem(slot);
 if(give&&(!held||!held->IsClassCommon()||!held->GetCustomDataString().empty()||(!held->IsStackable()&&held->GetCharges()>0)))return false;
 if(!give&&(held||item_id<=0))return false;
 if(!BotQuery("START TRANSACTION").Success())return false;
 auto abort=[](){BotQuery("ROLLBACK");return false;};
 auto owner=BotQuery(fmt::format("SELECT id FROM takp_bot_data WHERE id={} AND owner_character_id={} FOR UPDATE",bot->ID(),c->CharacterID()));
 if(!owner.Success()||owner.RowCount()!=1)return abort();
 auto lease=BotQuery(fmt::format("SELECT generation FROM takp_bot_runtime WHERE bot_id={} FOR UPDATE",bot->ID()));
 if(!lease.Success()||lease.RowCount()!=1||strtoul(lease.begin()[0],nullptr,10)!=bot->Generation())return abort();
 auto stored=BotQuery(fmt::format("SELECT itemid,charges,custom_data FROM character_inventory WHERE id={} AND slotid={} FOR UPDATE",c->CharacterID(),slot));
 if(!stored.Success())return abort();
 std::unique_ptr<EQ::ItemInstance> returned;
 if(give) {
  if(stored.RowCount()!=1)return abort();auto r=stored.begin();
  if(strtoul(r[0],nullptr,10)!=held->GetID()||atoi(r[1])!=held->GetCharges()||(r[2]&&*r[2]))return abort();
  const int quantity=held->IsStackable()?held->GetCharges():1;if(quantity<1||quantity>20)return abort();
  if(!BotQuery(fmt::format("INSERT INTO takp_bot_supplies VALUES({},{},{}) ON DUPLICATE KEY UPDATE quantity=quantity+VALUES(quantity)",bot->ID(),held->GetID(),quantity)).Success())return abort();
  if(!BotQuery(fmt::format("DELETE FROM character_inventory WHERE id={} AND slotid={}",c->CharacterID(),slot)).Success())return abort();
 } else {
  if(stored.RowCount())return abort();const auto *item=database.GetItem(item_id);if(!item||c->CheckLoreConflict(item))return abort();
  auto supply=BotQuery(fmt::format("SELECT quantity FROM takp_bot_supplies WHERE bot_id={} AND item_id={} FOR UPDATE",bot->ID(),item_id));
  if(!supply.Success()||supply.RowCount()!=1||atoi(supply.begin()[0])<1)return abort();
  returned.reset(database.CreateItem(item_id,1));if(!returned)return abort();
  int quantity=returned->IsStackable()?std::min(20,atoi(supply.begin()[0])):1;returned->SetCharges(returned->IsStackable()?quantity:0);
  if(!BotQuery(fmt::format("INSERT INTO character_inventory(id,slotid,itemid,charges,custom_data) VALUES({},{},{},{},'')",c->CharacterID(),slot,item_id,returned->GetCharges())).Success())return abort();
  if(!BotQuery(fmt::format("UPDATE takp_bot_supplies SET quantity=quantity-{} WHERE bot_id={} AND item_id={}",quantity,bot->ID(),item_id)).Success())return abort();
 }
 if(!BotQuery("COMMIT").Success()){BotQuery("ROLLBACK");c->Kick("Supply transaction uncertain; reconnect to reload inventory.");return false;}
 if(give)c->DeleteItemInInventory(slot,0,true,false);else{c->GetInv().PutItem(slot,*returned);c->SendItemPacket(slot,returned.get(),ItemPacketTrade);}
 c->Message(Chat::White,"Bot supplies saved.");return true;
}

void RotationCommand(Client *c,uint32 leader,const std::string &action,const char *argument)
{
 if(!leader){c->Message(Chat::White,"Specify your rotation leader's bot name.");return;}
 const auto owner=c->CharacterID();
 if(action=="healrotationcreate") {
  auto eligible=BotQuery(fmt::format("SELECT id FROM takp_bot_data WHERE id={} AND owner_character_id={} AND class IN(2,3,4,6,10,15)",leader,owner));
  if(!eligible.Success()||eligible.RowCount()!=1){c->Message(Chat::White,"The rotation leader must be one of your healing-class bots.");return;}
  if(!BotQuery("START TRANSACTION").Success())return;
  auto a=BotQuery(fmt::format("INSERT INTO takp_bot_heal_rotations(leader_bot_id,owner_character_id) VALUES({},{})",leader,owner));
  auto b=BotQuery(fmt::format("INSERT INTO takp_bot_heal_rotation_members VALUES({},{})",leader,leader));
  if(!a.Success()||!b.Success()){BotQuery("ROLLBACK");c->Message(Chat::White,"Rotation already exists or leader belongs to another rotation.");return;}
  if(!BotQuery("COMMIT").Success()){BotQuery("ROLLBACK");return;}
  c->Message(Chat::White,"Healing rotation created, stopped. Add healers/targets, then start it.");return;
 }
 auto exists=BotQuery(fmt::format("SELECT enabled,interval_ms FROM takp_bot_heal_rotations WHERE leader_bot_id={} AND owner_character_id={}",leader,owner));
 if(!exists.Success()||exists.RowCount()!=1){c->Message(Chat::White,"Create this rotation first.");return;}
 if(action=="healrotationstart"||action=="healrotationstop") {
  BotQuery(fmt::format("UPDATE takp_bot_heal_rotations SET enabled={},next_member=0,next_cast_at=CURRENT_TIMESTAMP(3) WHERE leader_bot_id={}",action=="healrotationstart",leader));
 } else if(action=="healrotationchangeinterval") {
  int interval;if(!SlotNumber(argument,interval)||interval<500||interval>60000){c->Message(Chat::White,"Interval must be 500-60000 milliseconds.");return;}
  BotQuery(fmt::format("UPDATE takp_bot_heal_rotations SET interval_ms={} WHERE leader_bot_id={}",interval,leader));
 } else if(action=="healrotationaddmember"||action=="healrotationremovemember") {
  auto member=BotQuery(fmt::format("SELECT id FROM takp_bot_data WHERE owner_character_id={} AND name='{}' AND class IN(2,3,4,6,10,15)",owner,Strings::Escape(argument)));
  if(!member.Success()||member.RowCount()!=1){c->Message(Chat::White,"Specify one of your healing-class bots.");return;}
  const auto id=strtoul(member.begin()[0],nullptr,10);
  auto q=action=="healrotationaddmember"?fmt::format("INSERT INTO takp_bot_heal_rotation_members VALUES({},{})",leader,id):fmt::format("DELETE FROM takp_bot_heal_rotation_members WHERE leader_bot_id={} AND bot_id={}",leader,id);
  if(!BotQuery(q).Success()){c->Message(Chat::White,"Member already belongs to a rotation.");return;}
 } else if(action=="healrotationaddtarget"||action=="healrotationremovetarget") {
  auto *target=entity_list.GetMob(argument);bool friendly=false;for(auto *bot:Owned(c))if(bot->ID()==leader&&bot->Friendly(target))friendly=true;
  if(action=="healrotationaddtarget"&&!friendly){c->Message(Chat::White,"Spawn the leader and specify a member of its group.");return;}
  auto name=Strings::Escape(argument);
  auto q=action=="healrotationaddtarget"?fmt::format("INSERT IGNORE INTO takp_bot_heal_rotation_targets VALUES({},'{}')",leader,name):fmt::format("DELETE FROM takp_bot_heal_rotation_targets WHERE leader_bot_id={} AND name='{}'",leader,name);BotQuery(q);
 } else if(action=="healrotationdelete") {
  if(!BotQuery("START TRANSACTION").Success())return;
  for(auto table:{"takp_bot_heal_rotation_targets","takp_bot_heal_rotation_members","takp_bot_heal_rotations"})if(!BotQuery(fmt::format("DELETE FROM {} WHERE leader_bot_id={}",table,leader)).Success()){BotQuery("ROLLBACK");return;}
  BotQuery("COMMIT");
 } else if(action=="healrotationlist") {
  c->Message(Chat::White,"Rotation enabled %s, interval %s ms",exists.begin()[0],exists.begin()[1]);
  auto members=BotQuery(fmt::format("SELECT d.name FROM takp_bot_heal_rotation_members m JOIN takp_bot_data d ON d.id=m.bot_id WHERE m.leader_bot_id={} ORDER BY d.id",leader));
  if(members.Success())for(auto r=members.begin();r!=members.end();++r)c->Message(Chat::White,"Healer: %s",r[0]);
  auto targets=BotQuery(fmt::format("SELECT name FROM takp_bot_heal_rotation_targets WHERE leader_bot_id={}",leader));
  if(targets.Success())for(auto r=targets.begin();r!=targets.end();++r)c->Message(Chat::White,"Target: %s",r[0]);return;
 } else {c->Message(Chat::White,"Unknown heal rotation action. See #bot help.");return;}
 c->Message(Chat::White,"Healing rotation updated.");
}

void DeleteBot(Client *c,uint32 id,const char *confirmation)
{
 if(!id||std::string(confirmation)!="confirm"){c->Message(Chat::White,"Use #bot delete Name confirm after dismissing and removing all equipment and supplies.");return;}
 for(auto *b:Owned(c))if(b->ID()==id){c->Message(Chat::White,"Dismiss the bot first.");return;}
 if(!BotQuery("START TRANSACTION").Success())return;
 auto fail=[&](){BotQuery("ROLLBACK");c->Message(Chat::White,"Deletion refused; remove equipment/supplies and dismiss the bot first.");};
 auto owner=BotQuery(fmt::format("SELECT id FROM takp_bot_data WHERE id={} AND owner_character_id={} FOR UPDATE",id,c->CharacterID()));
 if(!owner.Success()||owner.RowCount()!=1){fail();return;}
 auto rotations=BotQuery(fmt::format("SELECT (SELECT COUNT(*) FROM takp_bot_heal_rotation_members WHERE bot_id={} OR leader_bot_id={})+(SELECT COUNT(*) FROM takp_bot_heal_rotations WHERE leader_bot_id={})",id,id,id));
 if(!rotations.Success()||atoi(rotations.begin()[0])){BotQuery("ROLLBACK");c->Message(Chat::White,"Remove the bot from healing rotations before deleting it.");return;}
 auto runtime=BotQuery(fmt::format("SELECT active FROM takp_bot_runtime WHERE bot_id={} FOR UPDATE",id));
 auto items=BotQuery(fmt::format("SELECT COUNT(*) FROM takp_bot_inventory WHERE bot_id={}",id));
 auto supplies=BotQuery(fmt::format("SELECT COUNT(*) FROM takp_bot_supplies WHERE bot_id={} AND quantity>0",id));
 if(!runtime.Success()||!items.Success()||!supplies.Success()||(runtime.RowCount()&&atoi(runtime.begin()[0]))||atoi(items.begin()[0])||atoi(supplies.begin()[0])){fail();return;}
 for(auto table:{"takp_bot_recasts","takp_bot_buffs","takp_bot_inventory","takp_bot_runtime","takp_bot_settings","takp_bot_supplies","takp_bot_pets","takp_bot_pet_buffs","takp_bot_pet_items","takp_bot_songs","takp_bot_options","takp_bot_spell_settings","takp_bot_blocked_buffs","takp_bot_ability_recasts","takp_bot_saved_groups","takp_bot_data"}) {
  if(!BotQuery(fmt::format("DELETE FROM {} WHERE {}={}",table,std::string(table)=="takp_bot_data"?"id":"bot_id",id)).Success()){fail();return;}
 }
 if(!BotQuery("COMMIT").Success()){fail();return;}
 c->Message(Chat::White,"Bot deleted.");
}
