#include "player_bot_schema.h"
#include <cassert>
#include <string>
#include <vector>

struct Result {
 bool success;
 unsigned rows;
 std::string value;
 bool Success() const { return success; }
 unsigned RowCount() const { return rows; }
 struct Row {
  const char *value;
  const char *operator[](int) const { return value; }
 };
 Row begin() { return {value.c_str()}; }
};

int main()
{
 auto check = [](Result tables, Result versions, bool ready, unsigned calls) {
  std::vector<std::string> queries;
  const bool result = PlayerBotSchema::Ready([&](const std::string &sql) {
   queries.push_back(sql);
   return queries.size() == 1 ? tables : versions;
  });
  assert(result == ready);
  assert(queries.size() == calls);
  assert(queries[0].find("information_schema.TABLES") != std::string::npos);
  assert(queries[0].find("ENGINE='InnoDB'") != std::string::npos);
 };
 check({true,1,"0"},{true,1,"0"},false,1); // Fresh upstream database.
 check({true,1,"23"},{true,1,"11"},false,1); // Missing table or MyISAM inventory.
 check({false,0,""},{true,1,"11"},false,1); // Metadata query fails safely.
 check({true,1,"24"},{true,1,"10"},false,2); // Partial migrations.
 check({true,1,"24"},{false,0,""},false,2);
 check({true,1,"24"},{true,1,"11"},true,2);
 // Every required table is present once in the production engine whitelist.
 unsigned count = 0;
 for (const char *ch = PlayerBotSchema::RequiredTables; *ch; ++ch) if (*ch == ',') ++count;
 assert(count + 1 == PlayerBotSchema::RequiredTableCount);
}
