#!/usr/bin/env python3
"""Preflight/apply the bot migrations to an existing, stopped TAKP world.

Uses the MariaDB command-line client, including its DELIMITER support. Credentials
belong in a private defaults file, not in process arguments or source control.
"""

import argparse
import pathlib
import re
import shutil
import subprocess
import sys


MIGRATION_DIR = pathlib.Path(__file__).resolve().parent
VERSIONS = set(range(1, 12))
NAME_ROWS = """
SELECT CONVERT(name USING utf8mb4) COLLATE utf8mb4_general_ci AS shared_name,
       'player' AS kind,id FROM character_data WHERE name<>''
"""


class MigrationError(RuntimeError):
    pass


class Database:
    def __init__(self, name, client=None, defaults_extra_file=None, client_args=()):
        if not re.fullmatch(r"[A-Za-z0-9_]+", name):
            raise MigrationError("Use a database name containing only letters, digits and underscores.")
        executable = client or shutil.which("mariadb") or shutil.which("mysql")
        if not executable:
            raise MigrationError("Install the MariaDB command-line client first.")
        self.command = [str(executable)]
        if defaults_extra_file:
            self.command.append("--defaults-extra-file=" + str(defaults_extra_file))
        for arg in client_args:
            if re.match(r"(?:-p|-f$|-e|-D|--(?:password|force|execute|database)(?:=|$))", arg):
                raise MigrationError("Use --defaults-extra-file for credentials; do not override execution options.")
        self.command += list(client_args)
        self.command += ["--batch", "--skip-column-names", "--database=" + name]

    def run(self, sql):
        result = subprocess.run(self.command, input=sql, text=True, capture_output=True)
        if result.returncode:
            raise MigrationError(result.stderr.strip() or "MariaDB client failed.")
        return [line.split("\t") for line in result.stdout.splitlines() if line]


def migrations():
    files = sorted(MIGRATION_DIR.glob("[0-9][0-9][0-9]_*.sql"))
    found = {int(path.name[:3]) for path in files}
    if found != VERSIONS or len(files) != len(VERSIONS):
        raise MigrationError("Expected exactly bot migrations 001 through 011.")
    return files


def required_columns():
    """Read field names from our additive migrations, not from operator data."""
    columns = {"character_data": {"id", "name"},
               "character_inventory": {"id", "slotid", "itemid", "charges", "custom_data"}}
    for path in migrations():
        sql = path.read_text()
        for table, fields in re.findall(r"CREATE TABLE IF NOT EXISTS (\w+)\s*\((.*?)\)\s*ENGINE", sql, re.S | re.I):
            columns[table] = set(re.findall(
                r"\b(\w+)\s+(?:INT|TINYINT|SMALLINT|VARCHAR|TIMESTAMP|DATETIME)\b", fields, re.I))
        for table, field in re.findall(r"ALTER TABLE (\w+) ADD COLUMN IF NOT EXISTS (\w+)", sql, re.I):
            columns.setdefault(table, set()).add(field)
    return columns


def preflight(db):
    version = db.run("SELECT VERSION();")[0][0]
    numeric = re.match(r"(\d+)\.(\d+)", version)
    if "MariaDB" not in version or not numeric or tuple(map(int, numeric.groups())) < (10, 3):
        raise MigrationError("The bot migrations require MariaDB 10.3 or later.")
    engines = dict(db.run("""SELECT TABLE_NAME,ENGINE FROM information_schema.TABLES
        WHERE TABLE_SCHEMA=DATABASE() AND
        (TABLE_NAME IN ('character_data','character_inventory') OR TABLE_NAME REGEXP '^takp_bot_');"""))
    for table in ("character_data", "character_inventory"):
        if table not in engines:
            raise MigrationError("Missing TAKP player table: " + table)
    invalid = sorted(table for table, engine in engines.items() if engine != "InnoDB")
    if invalid:
        raise MigrationError("Transactional bot storage requires InnoDB: " + ", ".join(invalid)
                             + ". Back up and convert these separately; installation does not convert player tables.")
    available = {}
    for table, column in db.run("""SELECT TABLE_NAME,COLUMN_NAME FROM information_schema.COLUMNS
         WHERE TABLE_SCHEMA=DATABASE();"""):
        available.setdefault(table, set()).add(column.lower())
    for table, fields in required_columns().items():
        # A partial install may legitimately lack columns added by later migrations.
        fields = {field.lower() for field in fields}
        later = {"formation_gather", "formation_line", "keep_pace"} if table == "takp_bot_owner_settings" else set()
        if table in engines and fields - later - available.get(table, set()):
            raise MigrationError("Incompatible columns in " + table + ": "
                                 + ", ".join(sorted(fields - later - available.get(table, set()))))
    names = NAME_ROWS
    if "takp_bot_data" in engines:
        names += " UNION ALL SELECT CONVERT(name USING utf8mb4) COLLATE utf8mb4_general_ci,'bot',id FROM takp_bot_data"
    conflicts = db.run("SELECT COUNT(*) FROM (SELECT shared_name FROM (" + names
                       + ") AS names GROUP BY shared_name HAVING COUNT(*)>1) AS collisions;")[0][0]
    if int(conflicts):
        raise MigrationError("Character/bot name collisions must be resolved before installation ("
                             + conflicts + " conflicting names). No names or records were changed.")
    if "takp_bot_name_registry" in engines:
        stale = db.run("SELECT COUNT(*) FROM takp_bot_name_registry r LEFT JOIN (" + names
                       + ") AS names ON r.kind=names.kind AND r.entity_id=names.id WHERE names.id IS NULL"
                       + " OR CONVERT(r.name USING utf8mb4) COLLATE utf8mb4_general_ci <> names.shared_name;")[0][0]
        if int(stale):
            raise MigrationError("The bot name registry contains inconsistent reservations. Repair it separately before installation.")
    return version


def installed_versions(db):
    exists = db.run("""SELECT COUNT(*) FROM information_schema.TABLES WHERE TABLE_SCHEMA=DATABASE()
                        AND TABLE_NAME='takp_bot_schema';""")[0][0]
    return {int(row[0]) for row in db.run("SELECT version FROM takp_bot_schema ORDER BY version;")} if int(exists) else set()


def verify(db):
    preflight(db)
    if installed_versions(db) != VERSIONS:
        raise MigrationError("Bot schema versions 001 through 011 are not all installed.")
    actual = {}
    for table, column in db.run("SELECT TABLE_NAME,COLUMN_NAME FROM information_schema.COLUMNS WHERE TABLE_SCHEMA=DATABASE();"):
        actual.setdefault(table, set()).add(column.lower())
    for table, columns in required_columns().items():
        if {field.lower() for field in columns} - actual.get(table, set()):
            raise MigrationError("Missing bot schema columns in " + table)
    triggers = {row[0] for row in db.run("SELECT TRIGGER_NAME FROM information_schema.TRIGGERS WHERE TRIGGER_SCHEMA=DATABASE();")}
    expected = {"takp_bot_" + group + "name_" + action
                for group in ("", "player_") for action in ("insert", "update", "delete")}
    if expected - triggers:
        raise MigrationError("Missing bot name reservation triggers: " + ", ".join(sorted(expected - triggers)))
    collation = db.run("""SELECT COLLATION_NAME FROM information_schema.COLUMNS WHERE TABLE_SCHEMA=DATABASE()
                         AND TABLE_NAME='takp_bot_name_registry' AND COLUMN_NAME='name';""")[0][0]
    if collation != "utf8mb4_general_ci":
        raise MigrationError("Rerun migration 007 to install case-insensitive shared name reservations.")
    rows = NAME_ROWS + " UNION ALL SELECT CONVERT(name USING utf8mb4) COLLATE utf8mb4_general_ci,'bot',id FROM takp_bot_data"
    missing = db.run("SELECT COUNT(*) FROM (" + rows + ") AS names LEFT JOIN takp_bot_name_registry r"
                     + " ON r.kind=names.kind AND r.entity_id=names.id AND r.name=names.shared_name WHERE r.entity_id IS NULL;")[0][0]
    if int(missing):
        raise MigrationError("Missing shared name reservations; rerun migration 007 while the world is stopped.")


def apply(db, progress=print):
    preflight(db)
    for path in migrations():
        progress("Applying " + path.name)
        db.run(path.read_text())
    verify(db)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--database", required=True)
    parser.add_argument("--client")
    parser.add_argument("--defaults-extra-file", type=pathlib.Path)
    parser.add_argument("--client-arg", action="append", default=[], help="Connection option, e.g. --client-arg=--socket=/path")
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--apply", action="store_true", help="Apply/reapply all eleven migrations; stop world/zone processes first")
    mode.add_argument("--verify", action="store_true", help="Read-only check of a complete installation")
    args = parser.parse_args()
    try:
        db = Database(args.database, args.client, args.defaults_extra_file, args.client_arg)
        if args.apply:
            apply(db)
            print("Verified all eleven bot migrations, InnoDB tables and name reservations.")
        elif args.verify:
            verify(db)
            print("Verified all eleven bot migrations, InnoDB tables and name reservations.")
        else:
            preflight(db)
            print("Preflight passed; " + str(len(installed_versions(db)))
                  + " bot migrations installed. Stop world/zone processes, then use --apply.")
    except MigrationError as exc:
        print("Bot migration check failed: " + str(exc), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
