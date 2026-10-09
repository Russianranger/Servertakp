CREATE TABLE IF NOT EXISTS takp_bot_ability_recasts (
 bot_id INT UNSIGNED NOT NULL, kind VARCHAR(8) NOT NULL, ability_id INT UNSIGNED NOT NULL,
 ready_at DATETIME NOT NULL, PRIMARY KEY(bot_id,kind,ability_id)
) ENGINE=InnoDB;
INSERT IGNORE INTO takp_bot_schema(version) VALUES(6);
