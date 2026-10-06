#include "../client.h"
#include "../player_bot.h"
#include <cctype>
#include <cstdlib>
#include <string>

namespace {
bool BotName(const std::string &name)
{
 if (name.size() < 4 || name.size() > 15) return false;
 for (unsigned char ch : name) if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))) return false;
 return true;
}
bool Number(const char *text, unsigned &value)
{
 if (!text || !*text) return false;
 value = 0;
 for (; *text; ++text) {
  if (*text < '0' || *text > '9' || value > 1000) return false;
  value = value * 10 + (*text - '0');
 }
 return true;
}
}

// All records are character-owned. No caller supplies an owner ID and no
// command can read another character's roster, including administrator callers.
void command_bot(Client *c, const Seperator *sep)
{
 if (HandlePlayerBotCommand(c,sep)) return;
 const auto action = Strings::ToLower(sep->arg[1]);
 const auto owner = std::to_string(c->CharacterID());
 if (action != "create" && action != "list") {
  c->Message(Chat::White, "#bot create Name Class Race Gender | #bot list | #bot help.");
  return;
 }
 auto version = database.QueryDatabase("SELECT version FROM takp_bot_schema WHERE version=1");
 if (!version.Success() || version.RowCount() != 1) {
  c->Message(Chat::White, "Bot database migration 001 must be installed by the server operator.");
  return;
 }
 if (action == "list") {
  auto result = database.QueryDatabase("SELECT name,class,race,gender FROM takp_bot_data WHERE owner_character_id=" + owner + " ORDER BY name");
  if (!result.Success()) { c->Message(Chat::White, "Unable to read bot roster."); return; }
  for (auto row = result.begin(); row != result.end(); ++row)
   c->Message(Chat::White, "%s: class %s, race %s, gender %s", row[0], row[1], row[2], row[3]);
  if (!result.RowCount()) c->Message(Chat::White, "You have no saved bots.");
  return;
 }
 std::string name = sep->arg[2];
 unsigned class_id, race, gender;
 if (!BotName(name) || !Number(sep->arg[3], class_id) || !Number(sep->arg[4], race) || !Number(sep->arg[5], gender) ||
     class_id < 1 || class_id > 15 || gender > 1 || !((race >= 1 && race <= 12) || race == 128 || race == 130)) {
  c->Message(Chat::White, "Usage: #bot create Name Class Race Gender. Name: 4-15 letters; class: 1-15; race: 1-12,128,130; gender: 0 male,1 female.");
  return;
 }
 for (auto &ch : name) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
 name[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(name[0])));
 if (!database.CheckNameFilter(name)) {c->Message(Chat::White,"That name is not allowed.");return;}
 // Name is strictly ASCII letters above; never interpolate arbitrary command text.
 auto character = database.QueryDatabase("SELECT id FROM character_data WHERE name='" + name + "' LIMIT 1");
 if (!character.Success()) { c->Message(Chat::White, "Unable to validate bot name."); return; }
 if (character.RowCount()) { c->Message(Chat::White, "That name belongs to a character."); return; }
 auto result = database.QueryDatabase("INSERT INTO takp_bot_data(owner_character_id,name,class,race,gender) VALUES(" +
  owner + ",'" + name + "'," + std::to_string(class_id) + "," + std::to_string(race) + "," + std::to_string(gender) + ")");
 if (!result.Success()) { c->Message(Chat::White, "Bot could not be saved; its name may already be taken."); return; }
 c->Message(Chat::White, "%s saved. Use #bot spawn %s.", name.c_str(), name.c_str());
}
