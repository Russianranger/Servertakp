#ifndef EQMACEMU_COMPANION_H
#define EQMACEMU_COMPANION_H

class Mob;
// Transient companions have no character/group_id database row.
bool IsTransientCompanion(Mob *mob);

#endif
