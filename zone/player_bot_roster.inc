// Saved groups contain only the invoking character's bots.
bool SavedBotGroup(Client *c,const std::string &action,const std::string &name)
{
 if(action.compare(0,8,"botgroup")!=0)return false;
 if(action=="botgrouplist"){
  auto rows=BotQuery(fmt::format("SELECT g.name,d.name FROM takp_bot_saved_groups g JOIN takp_bot_data d ON d.id=g.bot_id AND d.owner_character_id=g.owner_character_id WHERE g.owner_character_id={} ORDER BY g.name,d.name",c->CharacterID()));
  if(rows.Success())for(auto r=rows.begin();r!=rows.end();++r)c->Message(Chat::White,"%s: %s",r[0],r[1]);return true;
 }
 if(name.empty()||name.size()>32||name.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_")!=std::string::npos){c->Message(Chat::White,"Group name: 1-32 letters, numbers or underscores.");return true;}
 if(action=="botgroupcreate"){
  auto bots=Owned(c);if(bots.empty()){c->Message(Chat::White,"Spawn the bots to save first.");return true;}
  if(!BotQuery("START TRANSACTION").Success())return true;
  if(!BotQuery(fmt::format("DELETE FROM takp_bot_saved_groups WHERE owner_character_id={} AND name='{}'",c->CharacterID(),name)).Success()){BotQuery("ROLLBACK");return true;}
  for(auto *b:bots)if(!BotQuery(fmt::format("INSERT INTO takp_bot_saved_groups VALUES({},'{}',{})",c->CharacterID(),name,b->ID())).Success()){BotQuery("ROLLBACK");return true;}
  if(BotQuery("COMMIT").Success())c->Message(Chat::White,"Current bot group saved.");else BotQuery("ROLLBACK");
 }else if(action=="botgroupload"){
  auto rows=BotQuery(fmt::format("SELECT d.id FROM takp_bot_saved_groups g JOIN takp_bot_data d ON d.id=g.bot_id AND d.owner_character_id=g.owner_character_id WHERE g.owner_character_id={} AND g.name='{}' ORDER BY d.id",c->CharacterID(),name));
  if(rows.Success())for(auto r=rows.begin();r!=rows.end();++r)if(!Spawn(c,strtoul(r[0],nullptr,10)))c->Message(Chat::White,"A saved member could not spawn; check free group slots and fallen bots.");
 }else if(action=="botgroupdelete"){
  if(BotQuery(fmt::format("DELETE FROM takp_bot_saved_groups WHERE owner_character_id={} AND name='{}'",c->CharacterID(),name)).Success())c->Message(Chat::White,"Saved group removed; bots and equipment retained.");
 }else c->Message(Chat::White,"Use botgroupcreate/load/delete GroupName or botgrouplist.");
 return true;
}
