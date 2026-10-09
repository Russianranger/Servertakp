# Validation and review notes

Server baseline: `d8d589a0ce486ee47ec990d3c3bbbf604b2572a1` (October 6, 2026).
Bot source: QuestOpenRpg/Server `68dcdee2b60c9948524288af90d71b52beae686c`.
The source feature was 30 upstream commits behind this fork. This integration
ports the feature onto current server code instead of replacing the server with
the older bot branch.

The source author reports Debian 12 x86_64 deployment and gameplay-driven fixes
for movement, sitting, armor appearance, taunt, Harmony and casting controls.
That history does not qualify this merged build. Live client/server gameplay,
individual spells, AAs, skills, races and zones still need confirmation.

## Integration review

- Retained current RDP disconnect/zoning lifecycle and trade packet/session checks.
- Kept the upstream removal of the incorrect classic NPC Enchanter resist cap.
- Save/detach bots before logout, linkdead, zoning and zone shutdown destroy owner
  group state or zone data. Repeated cleanup excludes already-depopped bots.
- Require all eleven schema versions and all required transactional tables before
  commands, restoration and inventory transfer. Fresh upstream databases skip bot
  restoration without querying missing bot tables.
- Claim runtime generations under one transaction and reject stale equipment or
  supply writes. Bot SQL does not replay individual statements after a reconnect.
- On an uncertain equipment COMMIT, discard only stale in-memory trade offers,
  return offered currency once and disconnect for durable inventory recovery.

## Tests

Run `python3 tests/player-bots/run.py` with a C++20 compiler available. Set `CXX`
if necessary. Tests do not launch a game server or modify a database.

All sixteen standalone programs passed during integration:
formation layout, formation modes, resting, forced-sit eligibility, movement
prediction, continuity, spacing, velocity, fine pacing, DPS spell policy,
AE targeting and beneficial permissions, crowd-control limits, backstab and
hate selection, schema readiness and uncertain trade recovery. Movement, readiness
and trade recovery tests exercise production headers. The targeting,
spell and skill fixtures contain extracted production snippets with controlled
stub entities, not complete zone integration tests; keep these extracts aligned
with future production changes. Spell-policy fixtures use actual server records.

Run `python3 tests/player-bots/database.py` against a disposable MariaDB instance
whose account may create databases. It creates randomly named test databases and
drops only those databases; it does not select an existing gameplay database.
The nine integration cases cover installation/reapplication against the bundled
database dump, retained player/bot inventory, case-insensitive names, failed rename
rollback, existing collisions, nontransactional inventory rejection, incomplete
schema detection, concurrent name creation, equipment rollback, generation claim
serialization and upgrading an older name registry.
All nine passed locally on MariaDB 10.11.14 against the September 8 bundled dump;
the isolated databases were dropped after the tests.

The GitHub Actions workflow builds all server components and client import/export
tools with Lua 5.1 on native x86_64 and ARM64, runs the standalone regressions on
both architectures and runs the MariaDB suite separately. It archives binaries,
logs, dependency checks and source/architecture metadata. Check the PR's workflow
results for the exact commit being qualified; adding a workflow does not prove a
successful build on an architecture.

## Packaging and deployment

Eleven additive migrations and a preflight/apply/verify helper are included. They
retain existing player data. Stop world/zone processes before installing them;
MariaDB DDL can partially apply before a later script fails. Backups and explicit
operator installation are required. No running server or database was deployed
as part of this source integration.

The prototype GM companion command is retained at GM-only access because it is
referenced by command registration; the player feature is `#bot` at player access.
Normal CMake output and opt-in bounded movement diagnostics are retained. There
is no precompiled client, launcher or old bot installer in this change.

## Gameplay checks before a binary release

- Log out/in with equipped armor; verify appearance and item persistence.
- Use taunt and compare aggro, then toggle it off and check the report.
- Use `#bot dps cast off` and confirm DoTs/heals/buffs remain available.
- Check Harmony and other area spells before enemies have hate.
- Check mez/charm/stun caps against appropriate and over-level targets.
- Check group heals, Mass Group Buff on bots, and protected targets.
- Check DoT-only kills, backstab weapons/position, shield bash and monk specials.
- Check Harm Touch/Lay on Hands reuse across dismissal and login.
- Zone, log out, lose the connection and shut down a zone with active bots; check
  groups, pets, health, equipment and recasts after reconnecting.
- Accept/cancel equipment trades, including replacements and offered coins;
  verify inventory recovery after an interrupted database connection.

Future upstream updates require an explicit merge/rebase, conflict review, build
and regression checks. Do not automatically rebase or deploy a running server.
Preserve upstream history and license notices.
