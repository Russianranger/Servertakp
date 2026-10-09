CREATE TABLE IF NOT EXISTS takp_bot_saved_groups (
 owner_character_id INT UNSIGNED NOT NULL,
 name VARCHAR(32) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL,
 bot_id INT UNSIGNED NOT NULL,
 PRIMARY KEY(owner_character_id,name,bot_id), KEY bot_member(bot_id)
) ENGINE=InnoDB;
INSERT IGNORE INTO takp_bot_schema(version) VALUES(8);
