CREATE TABLE IF NOT EXISTS takp_bot_owner_settings (
 owner_character_id INT UNSIGNED NOT NULL PRIMARY KEY,
 formation_enabled TINYINT UNSIGNED NOT NULL DEFAULT 0
) ENGINE=InnoDB;
INSERT IGNORE INTO takp_bot_schema(version) VALUES(9);
