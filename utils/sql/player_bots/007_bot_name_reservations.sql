-- Additive shared name reservations. Existing collisions abort rather than rename/delete characters.
CREATE TABLE IF NOT EXISTS takp_bot_name_registry (
 name VARCHAR(64) NOT NULL PRIMARY KEY,
 kind VARCHAR(8) NOT NULL,
 entity_id INT UNSIGNED NOT NULL,
 UNIQUE KEY entity_name(kind,entity_id)
) ENGINE=InnoDB;
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
