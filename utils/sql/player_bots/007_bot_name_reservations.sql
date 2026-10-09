-- Additive shared name reservations. Existing collisions abort rather than rename/delete characters.
-- MariaDB 10.3+. Stop all world/zone processes before installing or rerunning.
CREATE TABLE IF NOT EXISTS takp_bot_name_registry (
 name VARCHAR(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL PRIMARY KEY,
 kind VARCHAR(8) NOT NULL,
 entity_id INT UNSIGNED NOT NULL,
 UNIQUE KEY entity_name(kind,entity_id)
) ENGINE=InnoDB;
DELIMITER //
BEGIN NOT ATOMIC
 IF (SELECT COUNT(*) FROM information_schema.TABLES WHERE TABLE_SCHEMA=DATABASE()
     AND TABLE_NAME IN ('character_data','character_inventory','takp_bot_data','takp_bot_name_registry')
     AND ENGINE='InnoDB') <> 4 THEN
  SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Bot name reservations require character_data, character_inventory and bot tables to use InnoDB';
 END IF;
 IF EXISTS (
  SELECT shared_name FROM (
   SELECT CONVERT(name USING utf8mb4) COLLATE utf8mb4_general_ci AS shared_name FROM character_data WHERE name<>''
   UNION ALL
   SELECT CONVERT(name USING utf8mb4) COLLATE utf8mb4_general_ci FROM takp_bot_data
  ) AS names GROUP BY shared_name HAVING COUNT(*) > 1
 ) THEN
  SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Character/bot name collision: resolve conflicting names before migration 007';
 END IF;
 IF EXISTS (
  SELECT 1 FROM takp_bot_name_registry AS r LEFT JOIN (
   SELECT CONVERT(name USING utf8mb4) COLLATE utf8mb4_general_ci AS shared_name,'player' AS kind,id FROM character_data WHERE name<>''
   UNION ALL
   SELECT CONVERT(name USING utf8mb4) COLLATE utf8mb4_general_ci,'bot',id FROM takp_bot_data
  ) AS names ON r.kind=names.kind AND r.entity_id=names.id
  WHERE names.id IS NULL OR CONVERT(r.name USING utf8mb4) COLLATE utf8mb4_general_ci <> names.shared_name
 ) THEN
  SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='Stale bot name reservation: repair inconsistent registry entries before migration 007';
 END IF;
END//
DELIMITER ;
-- Ensure an existing installation also reserves names without regard to case,
-- even when its database default uses a binary/case-sensitive collation.
ALTER TABLE takp_bot_name_registry MODIFY name VARCHAR(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL;
INSERT INTO takp_bot_name_registry(name,kind,entity_id)
 SELECT name,'player',id FROM character_data WHERE name<>''
 ON DUPLICATE KEY UPDATE name=IF(kind=VALUES(kind) AND entity_id=VALUES(entity_id),VALUES(name),NULL);
INSERT INTO takp_bot_name_registry(name,kind,entity_id)
 SELECT name,'bot',id FROM takp_bot_data
 ON DUPLICATE KEY UPDATE name=IF(kind=VALUES(kind) AND entity_id=VALUES(entity_id),VALUES(name),NULL);
DELIMITER //
CREATE OR REPLACE TRIGGER takp_bot_player_name_insert AFTER INSERT ON character_data FOR EACH ROW
BEGIN
 IF NEW.name<>'' THEN INSERT INTO takp_bot_name_registry VALUES(NEW.name,'player',NEW.id); END IF;
END//
CREATE OR REPLACE TRIGGER takp_bot_player_name_update AFTER UPDATE ON character_data FOR EACH ROW
BEGIN
 IF NOT (NEW.name<=>OLD.name) THEN
  DELETE FROM takp_bot_name_registry WHERE kind='player' AND entity_id=OLD.id;
  IF NEW.name<>'' THEN INSERT INTO takp_bot_name_registry VALUES(NEW.name,'player',NEW.id); END IF;
 END IF;
END//
CREATE OR REPLACE TRIGGER takp_bot_player_name_delete AFTER DELETE ON character_data FOR EACH ROW
BEGIN DELETE FROM takp_bot_name_registry WHERE kind='player' AND entity_id=OLD.id; END//
CREATE OR REPLACE TRIGGER takp_bot_name_insert AFTER INSERT ON takp_bot_data FOR EACH ROW
BEGIN INSERT INTO takp_bot_name_registry VALUES(NEW.name,'bot',NEW.id); END//
CREATE OR REPLACE TRIGGER takp_bot_name_update AFTER UPDATE ON takp_bot_data FOR EACH ROW
BEGIN
 IF NOT (NEW.name<=>OLD.name) THEN
  DELETE FROM takp_bot_name_registry WHERE kind='bot' AND entity_id=OLD.id;
  INSERT INTO takp_bot_name_registry VALUES(NEW.name,'bot',NEW.id);
 END IF;
END//
CREATE OR REPLACE TRIGGER takp_bot_name_delete AFTER DELETE ON takp_bot_data FOR EACH ROW
BEGIN DELETE FROM takp_bot_name_registry WHERE kind='bot' AND entity_id=OLD.id; END//
DELIMITER ;
INSERT IGNORE INTO takp_bot_schema(version) VALUES(7);
