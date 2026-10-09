# Build and install

This branch integrates persistent player bots into `Russianranger/Servertakp`.
Build the matching server executable and install the bot migrations before enabling
the feature. See [validation notes](VALIDATION.md) for what has been tested.

## Compatibility

Server baseline: `d8d589a0ce486ee47ec990d3c3bbbf604b2572a1` (October 6, 2026).
Imported bot feature: QuestOpenRpg/Server commit
`68dcdee2b60c9948524288af90d71b52beae686c`, originally based on March 25's
`bd838e594a29cd8d9b9bd43cd862c8692f01a10e`. The integration preserves the
current RDP stream lifecycle, validated trade packets and upstream spell fixes.

Use [queststakp](https://github.com/Russianranger/queststakp) and
[Mapstakp](https://github.com/Russianranger/Mapstakp) as the content repositories.
The bot feature does not require changes to either repository or the TAKP client.
Client rendering, zoning and gameplay still require a live smoke test.
An existing Docker image will continue running its own binaries until you build
and install this fork; changing a repository URL alone does not upgrade it.

## Build from a separate checkout

Use the upstream build dependencies. Inside an environment with those dependencies:

```sh
git clone --branch codex/takp-player-bots --recurse-submodules https://github.com/Russianranger/Servertakp.git takp-player-bots
cd takp-player-bots
bash scripts/validation/build-server.sh build/validation
```

The normal output is `build/validation/bin/zone`. The script builds all server
components with Lua 5.1, checks their shared-library dependencies and packages
the binaries with source/architecture metadata. Dependencies are listed in
`.github/workflows/player-bots.yml`, which qualifies native x86_64 and ARM64.
Do not build over an executable
currently in use. Movement diagnostics are disabled by default; developers can
configure `-DPLAYER_BOT_DIAGNOSTICS=ON` to enable bounded coordinate/packet traces
in `/tmp/takp-follow-trace.log` and `/tmp/takp-bot-packets.log`.

## Database and deployment

1. Back up the destination database and current zone executable. Keep backups private.
2. Have all players log out; stop world, zone launchers and zone processes while retaining
   database access. Follow your server's normal service management procedure.
3. Use the migration helper below with MariaDB 10.3 or later. Its default mode is
   a read-only preflight; `--apply` runs all eleven scripts in order and verifies
   the complete schema. Store database credentials in a private MariaDB defaults
   file. The scripts also support direct use through the MariaDB client, including
   delimiter directives and triggers. Stop on any SQL error.
4. Verify all eleven versions are recorded in `takp_bot_schema`. The scripts add bot
   tables and shared character/bot name reservations; they do not import player data.
   Existing name collisions must be resolved before installation can finish.
   Character and bot tables must use InnoDB for transactional name reservations.
5. Install the matching newly built zone binary using your server layout, then restart
   zone launchers. Keep the previous executable for a code-only rollback.
6. Log in and use `#bot help`. Ordinary players can use their own bots without an
   account privilege increase. Five bots plus the owner fill a six-member group.

```sh
python3 utils/sql/player_bots/install.py --database YOUR_DATABASE --defaults-extra-file /path/to/private.cnf
# Run only after the world and zone processes are stopped:
python3 utils/sql/player_bots/install.py --database YOUR_DATABASE --defaults-extra-file /path/to/private.cnf --apply
python3 utils/sql/player_bots/install.py --database YOUR_DATABASE --defaults-extra-file /path/to/private.cnf --verify
```

The helper retains existing characters and does not convert player tables. If
preflight reports MyISAM tables or duplicate names, resolve those separately before
applying migrations. An interrupted migration can be rerun after fixing the error;
DDL is not an atomic rollback. Bots stay unavailable while the installation is
missing any of the eleven versions or required InnoDB tables.

Never import someone else's database to install this feature. For a code-only
rollback, restore the previous zone executable while zones are stopped. Retain
newer gameplay and bot tables; restoring a database backup rewinds progress.

## First bot

```text
#bot create Mercy 2 1 1
#bot spawn Mercy
#bot assist all
#bot healpercent Mercy 80
```

This creates a female human cleric. Additional examples:

```text
#bot formation on
#bot formation gather on
#bot formation line on
#bot keeppace on
#bot sit Mercy on
#bot sit Mercy off
#bot dps cast off Mercy
#bot dps cast on Mercy
#bot report Mercy
```

DPS cast OFF blocks direct-damage casting and leaves DoTs, buffs and heals eligible.
It persists per bot; omitting the name changes currently spawned owned bots.
Forced sit is a runtime hold, cleared when a bot is dismissed/recreated.

## Keeping up with EQMacEmu

Keep this feature on a dedicated branch. Fetch upstream changes into a separate development
checkout, review conflicts, rebuild and run regression/gameplay checks before deploying.
Do not automatically rebase or update a running server. Preserve upstream history and
licenses. Fork publication does not imply upstream acceptance.
