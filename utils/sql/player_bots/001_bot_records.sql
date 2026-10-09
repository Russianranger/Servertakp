-- Additive installation: run against the destination server's existing peq.
-- Does not replace player records, change account access, or import a snapshot.
CREATE TABLE IF NOT EXISTS takp_bot_data (
 id INT UNSIGNED NOT NULL AUTO_INCREMENT,
 owner_character_id INT UNSIGNED NOT NULL,
 name VARCHAR(15) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL,
 class TINYINT UNSIGNED NOT NULL,
 race SMALLINT UNSIGNED NOT NULL,
 gender TINYINT UNSIGNED NOT NULL,
 created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
 PRIMARY KEY (id), UNIQUE KEY bot_name (name), KEY bot_owner (owner_character_id)
) ENGINE=InnoDB;
CREATE TABLE IF NOT EXISTS takp_bot_schema (
 version INT UNSIGNED NOT NULL PRIMARY KEY,
 installed_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;
INSERT IGNORE INTO takp_bot_schema(version) VALUES (1);
