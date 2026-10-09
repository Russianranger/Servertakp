#pragma once

#include <cstdlib>
#include <string>

// Probe metadata before reading takp_bot_schema, so an uninstalled or partial
// database disables bots without issuing queries against missing bot tables.
// Keep the engine list in sync with utils/sql/player_bots/001..011.
namespace PlayerBotSchema {
inline constexpr const char *RequiredTables =
 "'character_data','character_inventory','takp_bot_schema','takp_bot_data',"
 "'takp_bot_runtime','takp_bot_inventory','takp_bot_buffs','takp_bot_recasts',"
 "'takp_bot_settings','takp_bot_supplies','takp_bot_pets','takp_bot_pet_buffs',"
 "'takp_bot_pet_items','takp_bot_songs','takp_bot_options','takp_bot_spell_settings',"
 "'takp_bot_blocked_buffs','takp_bot_heal_rotations','takp_bot_heal_rotation_members',"
 "'takp_bot_heal_rotation_targets','takp_bot_ability_recasts','takp_bot_name_registry',"
 "'takp_bot_saved_groups','takp_bot_owner_settings'";
inline constexpr unsigned RequiredTableCount = 24;
inline constexpr unsigned MigrationCount = 11;

template <typename Query>
bool Ready(Query query)
{
 auto tables = query(
  "SELECT COUNT(*) FROM information_schema.TABLES WHERE TABLE_SCHEMA=DATABASE() "
  "AND ENGINE='InnoDB' AND TABLE_NAME IN (" + std::string(RequiredTables) + ")");
 if (!tables.Success() || tables.RowCount() != 1 || !tables.begin()[0] ||
     std::strtoul(tables.begin()[0], nullptr, 10) != RequiredTableCount) return false;
 auto versions = query("SELECT COUNT(*) FROM takp_bot_schema WHERE version BETWEEN 1 AND 11");
 return versions.Success() && versions.RowCount() == 1 && versions.begin()[0] &&
        std::strtoul(versions.begin()[0], nullptr, 10) == MigrationCount;
}
}
