#!/usr/bin/env python3
"""MariaDB integration tests. Creates/drops only randomly named test databases.

Connect to a disposable MariaDB instance with permission to create databases.
No existing gameplay database is selected or modified.
"""

import argparse
import concurrent.futures
import importlib.util
import pathlib
import sys
import unittest
import uuid
import zipfile


ROOT = pathlib.Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location("bot_migrations", ROOT / "utils/sql/player_bots/install.py")
INSTALL = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(INSTALL)
OPTIONS = {}


class BotDatabaseTests(unittest.TestCase):
    def setUp(self):
        self.name = "takp_bot_test_" + uuid.uuid4().hex
        self.admin = INSTALL.Database("mysql", **OPTIONS)
        self.admin.run("CREATE DATABASE `" + self.name + "` CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;")
        self.addCleanup(self.admin.run, "DROP DATABASE IF EXISTS `" + self.name + "`;")
        self.db = INSTALL.Database(self.name, **OPTIONS)
        with zipfile.ZipFile(ROOT / "utils/sql/database_full/alkabor_latest.zip") as archive:
            self.db.run(archive.read(next(name for name in archive.namelist()
                                         if pathlib.PurePosixPath(name).name.startswith("player_tables_"))).decode())
        self.db.run("INSERT INTO character_data(id,account_id,name,level,exp,cur_hp) VALUES(1000001,9,'Thorhero',20,12345,120);")
        self.db.run("INSERT INTO character_inventory(id,slotid,itemid,charges,custom_data) VALUES(1000001,22,1001,5,'');")

    def install(self):
        INSTALL.apply(self.db, progress=lambda message: None)

    def bot(self, name="Mercy", identity=1000001):
        self.db.run("INSERT INTO takp_bot_data(id,owner_character_id,name,class,race,gender) VALUES("
                    + str(identity) + ",1000001,'" + name + "',2,1,1);")

    def test_current_bundled_dump_install_and_reapply_preserve_records(self):
        # Follow the current bundled dump's new-install instructions, including
        # full content/login schemas rather than a synthetic character fixture.
        with zipfile.ZipFile(ROOT / "utils/sql/database_full/alkabor_latest.zip") as archive:
            for prefix in ("alkabor_", "login_tables_"):
                name = next(name for name in archive.namelist()
                            if pathlib.PurePosixPath(name).name.startswith(prefix) and name.endswith(".sql"))
                self.db.run(archive.read(name).decode())
        player_before = self.db.run("SELECT * FROM character_data WHERE id=1000001;")
        inventory_before = self.db.run("SELECT * FROM character_inventory WHERE id=1000001;")
        self.install()
        self.bot()
        self.db.run("INSERT INTO takp_bot_runtime(bot_id,active,hp,mana,generation) VALUES(1000001,1,77,88,9);")
        self.db.run("INSERT INTO takp_bot_inventory VALUES(1000001,13,1002,32767);")
        self.db.run("INSERT INTO takp_bot_owner_settings VALUES(1000001,1,1,1,1);")
        bot_before = self.db.run("SELECT * FROM takp_bot_data;")
        runtime_before = self.db.run("SELECT * FROM takp_bot_runtime;")
        schema_before = self.db.run("SELECT * FROM takp_bot_schema ORDER BY version;")
        self.install()
        self.assertEqual(INSTALL.VERSIONS, INSTALL.installed_versions(self.db))
        self.assertEqual(schema_before, self.db.run("SELECT * FROM takp_bot_schema ORDER BY version;"))
        self.assertEqual(bot_before, self.db.run("SELECT * FROM takp_bot_data;"))
        self.assertEqual(runtime_before, self.db.run("SELECT * FROM takp_bot_runtime;"))
        self.assertEqual(player_before, self.db.run("SELECT * FROM character_data WHERE id=1000001;"))
        self.assertEqual(inventory_before, self.db.run("SELECT * FROM character_inventory WHERE id=1000001;"))
        self.assertEqual([["1000001", "13", "1002", "32767"]], self.db.run("SELECT * FROM takp_bot_inventory;"))
        self.assertEqual([["1000001", "1", "1", "1", "1"]], self.db.run("SELECT * FROM takp_bot_owner_settings;"))

    def test_case_insensitive_shared_names_and_failed_rename_are_atomic(self):
        self.install()
        self.bot()
        self.assertEqual([["utf8mb4_general_ci"]], self.db.run("""SELECT COLLATION_NAME FROM information_schema.COLUMNS
            WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='takp_bot_name_registry' AND COLUMN_NAME='name';"""))
        # Conflict must reject the whole character INSERT, even without strict mode.
        with self.assertRaises(INSTALL.MigrationError):
            self.db.run("SET SESSION sql_mode=''; INSERT INTO character_data(id,name) VALUES(1000002,'mErCy');")
        self.assertEqual([["0"]], self.db.run("SELECT COUNT(*) FROM character_data WHERE id=1000002;"))
        with self.assertRaises(INSTALL.MigrationError):
            self.db.run("UPDATE character_data SET name='Mercy' WHERE id=1000001;")
        self.assertEqual([["Thorhero"]], self.db.run("SELECT name FROM character_data WHERE id=1000001;"))
        self.assertEqual([["Thorhero"]], self.db.run("SELECT name FROM takp_bot_name_registry WHERE kind='player' AND entity_id=1000001;"))
        with self.assertRaises(INSTALL.MigrationError):
            self.db.run("UPDATE takp_bot_data SET name='Thorhero' WHERE id=1000001;")
        self.assertEqual([["Mercy"]], self.db.run("SELECT name FROM takp_bot_name_registry WHERE kind='bot' AND entity_id=1000001;"))
        self.db.run("UPDATE character_data SET name='Newhero' WHERE id=1000001; UPDATE takp_bot_data SET name='Kindly' WHERE id=1000001;")
        INSTALL.verify(self.db)
        self.db.run("START TRANSACTION; DELETE FROM takp_bot_data WHERE id=1000001; DELETE FROM character_data WHERE id=1000001; ROLLBACK;")
        self.assertEqual([["2"]], self.db.run("SELECT COUNT(*) FROM takp_bot_name_registry;"))
        self.db.run("DELETE FROM takp_bot_data WHERE id=1000001; DELETE FROM character_data WHERE id=1000001;")
        self.assertEqual([["0"]], self.db.run("SELECT COUNT(*) FROM takp_bot_name_registry;"))

    def test_existing_collision_aborts_without_modifying_player_or_bot(self):
        for path in INSTALL.migrations()[:6]:
            self.db.run(path.read_text())
        self.bot("tHoRhErO")
        before = self.db.run("SELECT * FROM character_data WHERE id=1000001;")
        with self.assertRaisesRegex(INSTALL.MigrationError, "name collisions"):
            self.install()
        with self.assertRaisesRegex(INSTALL.MigrationError, "name collision"):
            self.db.run("SET SESSION sql_mode='';\n" + INSTALL.migrations()[6].read_text())
        self.assertEqual(before, self.db.run("SELECT * FROM character_data WHERE id=1000001;"))
        self.assertEqual([["tHoRhErO"]], self.db.run("SELECT name FROM takp_bot_data;"))
        self.assertEqual(set(range(1, 7)), INSTALL.installed_versions(self.db))

    def test_nontransactional_player_table_is_rejected_before_install(self):
        self.db.run("ALTER TABLE character_inventory ENGINE=MyISAM;")
        with self.assertRaisesRegex(INSTALL.MigrationError, "InnoDB.*character_inventory"):
            self.install()
        self.assertEqual(set(), INSTALL.installed_versions(self.db))
        self.assertEqual([["1001", "5"]], self.db.run("SELECT itemid,charges FROM character_inventory WHERE id=1000001;"))

    def test_missing_trigger_or_final_column_cannot_pass_verification(self):
        self.install()
        self.db.run("DROP TRIGGER takp_bot_name_insert;")
        with self.assertRaisesRegex(INSTALL.MigrationError, "Missing bot name reservation triggers"):
            INSTALL.verify(self.db)
        self.install()
        self.db.run("ALTER TABLE takp_bot_owner_settings DROP COLUMN keep_pace;")
        with self.assertRaisesRegex(INSTALL.MigrationError, "Missing bot schema columns"):
            INSTALL.verify(self.db)
        self.install()
        INSTALL.verify(self.db)

    def test_concurrent_character_and_bot_creation_cannot_share_name(self):
        self.install()
        statements = ["INSERT INTO character_data(id,name) VALUES(1000002,'Racehero');",
                      "INSERT INTO takp_bot_data(id,owner_character_id,name,class,race,gender) VALUES(1000002,1000001,'Racehero',2,1,1);"]
        def attempt(sql):
            try:
                self.db.run(sql)
                return True
            except INSTALL.MigrationError:
                return False
        with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
            self.assertEqual(1, sum(pool.map(attempt, statements)))
        self.assertEqual([["1"]], self.db.run("""SELECT
             (SELECT COUNT(*) FROM character_data WHERE name='Racehero') +
             (SELECT COUNT(*) FROM takp_bot_data WHERE name='Racehero');"""))
        INSTALL.verify(self.db)

    def test_transfer_rollback_keeps_inventory_on_both_sides(self):
        self.install()
        self.bot()
        self.db.run("""START TRANSACTION;
            INSERT INTO takp_bot_inventory(bot_id,slot,item_id,charges)
            SELECT 1000001,13,itemid,charges FROM character_inventory WHERE id=1000001 AND slotid=22;
            DELETE FROM character_inventory WHERE id=1000001 AND slotid=22;
            ROLLBACK;""")
        self.assertEqual([["1001", "5"]], self.db.run("SELECT itemid,charges FROM character_inventory WHERE id=1000001 AND slotid=22;"))
        self.assertEqual([["0"]], self.db.run("SELECT COUNT(*) FROM takp_bot_inventory;"))

    def test_runtime_lease_claims_serialize_and_receive_distinct_generations(self):
        self.install()
        self.bot()
        self.db.run("INSERT INTO takp_bot_runtime(bot_id,generation) VALUES(1000001,0);")
        claim = """START TRANSACTION;
            SELECT generation INTO @old_generation FROM takp_bot_runtime WHERE bot_id=1000001 FOR UPDATE;
            UPDATE takp_bot_runtime SET generation=generation+1 WHERE bot_id=1000001;
            SELECT generation FROM takp_bot_runtime WHERE bot_id=1000001;
            DO SLEEP(0.1);
            COMMIT;"""
        with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
            generations = [int(rows[0][0]) for rows in pool.map(self.db.run, [claim, claim])]
        self.assertEqual([1, 2], sorted(generations))
        self.assertEqual([["2"]], self.db.run("SELECT generation FROM takp_bot_runtime WHERE bot_id=1000001;"))

    def test_reapply_upgrades_binary_registry_without_changing_identity(self):
        self.install()
        self.bot()
        before = self.db.run("SELECT name,kind,entity_id FROM takp_bot_name_registry ORDER BY kind;")
        self.db.run("ALTER TABLE takp_bot_name_registry MODIFY name VARCHAR(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL;")
        with self.assertRaisesRegex(INSTALL.MigrationError, "case-insensitive"):
            INSTALL.verify(self.db)
        self.install()
        self.assertEqual(before, self.db.run("SELECT name,kind,entity_id FROM takp_bot_name_registry ORDER BY kind;"))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--client")
    parser.add_argument("--defaults-extra-file", type=pathlib.Path)
    parser.add_argument("--client-arg", action="append", default=[])
    args = parser.parse_args()
    OPTIONS.update(client=args.client, defaults_extra_file=args.defaults_extra_file, client_args=args.client_arg)
    result = unittest.TextTestRunner(verbosity=2).run(unittest.defaultTestLoader.loadTestsFromTestCase(BotDatabaseTests))
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())
