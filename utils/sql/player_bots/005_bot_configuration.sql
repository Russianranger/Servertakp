CREATE TABLE IF NOT EXISTS takp_bot_options (
 bot_id INT UNSIGNED NOT NULL, setting VARCHAR(32) NOT NULL,
 value INT NOT NULL, PRIMARY KEY(bot_id,setting)
) ENGINE=InnoDB;
CREATE TABLE IF NOT EXISTS takp_bot_spell_settings (
 bot_id INT UNSIGNED NOT NULL, spell_id SMALLINT UNSIGNED NOT NULL,
 enabled TINYINT UNSIGNED NOT NULL DEFAULT 1,
 min_hp TINYINT UNSIGNED NOT NULL DEFAULT 0,
 max_hp TINYINT UNSIGNED NOT NULL DEFAULT 100,
 priority SMALLINT NOT NULL DEFAULT 0,
 PRIMARY KEY(bot_id,spell_id)
) ENGINE=InnoDB;
CREATE TABLE IF NOT EXISTS takp_bot_blocked_buffs (
 bot_id INT UNSIGNED NOT NULL, spell_id SMALLINT UNSIGNED NOT NULL,
 PRIMARY KEY(bot_id,spell_id)
) ENGINE=InnoDB;
CREATE TABLE IF NOT EXISTS takp_bot_heal_rotations (
 leader_bot_id INT UNSIGNED PRIMARY KEY,
 owner_character_id INT UNSIGNED NOT NULL,
 enabled TINYINT UNSIGNED NOT NULL DEFAULT 0,
 interval_ms INT UNSIGNED NOT NULL DEFAULT 3000,
 next_member INT UNSIGNED NOT NULL DEFAULT 0,
 next_cast_at DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3)
) ENGINE=InnoDB;
CREATE TABLE IF NOT EXISTS takp_bot_heal_rotation_members (
 leader_bot_id INT UNSIGNED NOT NULL, bot_id INT UNSIGNED NOT NULL,
 PRIMARY KEY(leader_bot_id,bot_id), UNIQUE KEY one_rotation_per_bot(bot_id)
) ENGINE=InnoDB;
CREATE TABLE IF NOT EXISTS takp_bot_heal_rotation_targets (
 leader_bot_id INT UNSIGNED NOT NULL, name VARCHAR(63) NOT NULL,
 PRIMARY KEY(leader_bot_id,name)
) ENGINE=InnoDB;
INSERT IGNORE INTO takp_bot_schema(version) VALUES(5);
