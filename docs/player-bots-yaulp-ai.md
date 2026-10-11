# Cleric Yaulp automatic casting

The launcher device report says: "My cleric continuously casts Yaulp, and should
not do that." The supplied diagnostic bundle does not contain per-spell cast
timestamps, a cleric level, a spell rank or combat state. The confirmed source
defect below is therefore distinct from any claim about the exact observed
device interval.

## Cause in the deployed baseline

Production server commit `25bf70acb6bd24853cf09e447ddd62b96a4491a4` classifies
Yaulp as a normal automatic buff because its spell data includes armor class,
attack or haste effects. `CastClassSpells(false)` tries owner/group buffs then
self buffs. When Yaulp expires, that idle branch is eligible to cast it again.
This repeatedly stands the cleric up and interrupts its ordinary rest cycle.

Native `CanBuffStack` already rejects an active copy of the same spell and
conflicting/overwritten buffs. The AI spell entry also retains its native recast
gate. Missing active-buff or recast checks are not needed to explain idle
maintenance after expiry.

The shipped `alkabor_latest.zip` has these six player-usable ranks:

| Spell | ID | Cleric level | Mana | Duration cap | Recast |
| --- | --- | --- | --- | --- | --- |
| Yaulp | 210 | 1 | 1 | 4 ticks | 18 seconds |
| Yaulp II | 43 | 19 | 1 | 4 ticks | 18 seconds |
| Yaulp III | 44 | 44 | 1 | 4 ticks | 18 seconds |
| Yaulp IV | 1534 | 53 | 1 | 4 ticks | 18 seconds |
| Yaulp V | 2326 | 56 | 10 | 4 ticks | 18 seconds |
| Yaulp VI | 3186 | 65 | 10 | 4 ticks | 18 seconds |

All target only the caster. V and VI also grant mana regeneration and haste;
they remain usable instead of being removed from the bot spell list.

## Changed behavior

After normal beneficial-buff classification, short (at most four ticks),
self-targeted cleric buffs become `SpellType_InCombatBuff`. In the qualified
seed this descriptor matches exactly the six ranks above, including lower
ranks without an attack effect; production does not hard-code spell IDs.

Automatic casts require active permitted combat and a target within melee
range. Healing, configured heal rotations, curing and crowd control retain
their earlier scheduling priority. The existing casting, AI timer, owner,
suspension and stance gates remain. The self-buff selection checks effective
mana cost, the native AI entry's recast deadline, immunity and native buff
stacking. Only an actually accepted `AIDoSpellCast` consumes the opportunity;
a disabled or otherwise rejected rank cannot suppress the remaining choices.
Native cast completion still sets the recast timer.

The cleric does not maintain Yaulp while idle or fighting from outside melee
range. During permitted melee it can use the strongest allowed rank, then
refresh only after its buff and native recast gates permit it. Manual spell
commands, ordinary long-duration support buffs, other players/NPCs and saved
bot settings/data are unchanged. No migration or bot recreation is required.

## Verification and device check

`python3 tests/player-bots/yaulp_spell_tests.py` reads the shipped seed, verifies
all six ranks, extracts four actual production methods and compiles/runs those
bodies in a deterministic native fixture. Engine primitives control stacking,
timer, target and cast-start outcomes; this does not claim a full game session.
Tests cover idle exclusion, permitted melee, healing/cure/rotation/CC priority,
all existing gates, active-buff/expired-buff behavior, disabled ranks, failed
cast attempts, and ordinary buffs. The fixture fails on the deployed baseline
at the unwanted idle-cast assertion and passes after the fix. The standalone
bot runner includes this regression.

The updated native server source must be built and deployed before an existing
world uses this behavior. On Thor, preserve the database and bot roster, spawn
the existing cleric, remain idle across several former Yaulp refresh cycles,
then test healing, melee combat and combat from outside melee range. At levels
56/65 check that the mana/haste rank is still available during permitted melee.
Confirm the same roster after camp, restart and respawn. Save fresh logs and
the observed level/rank/combat state if unwanted casting remains.
