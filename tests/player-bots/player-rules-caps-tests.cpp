#include <cassert>
#include <iostream>
namespace SpecialAbility{enum{StunImmunity};}
struct Mob{bool client=false,bot=false;bool IsClient(){return client;}bool IsNPC(){return !client;}};
bool IsPlayerBot(Mob*m){return m&&m->bot;}
struct Spell{int max[1]={40};};Spell spells[1];
struct Target{int level=41;bool client=false,immune=false;int GetLevel(){return level;}bool IsClient(){return client;}bool IsNPC(){return !client;}bool GetSpecialAbility(int){return immune;}
bool mez(Mob*caster){int spell_id=0,effect_index=0;return GetLevel() > spells[spell_id].max[effect_index] && (caster->IsClient() || IsPlayerBot(caster)) && IsNPC();}
bool stunExempt(Mob*caster){return IsClient() || (caster && caster->IsNPC() && !IsPlayerBot(caster));}
bool spin(Mob*caster,int max_level=40){return GetSpecialAbility(SpecialAbility::StunImmunity) ||
					(GetLevel() > max_level && caster && (caster->IsClient() || IsPlayerBot(caster)) && IsNPC());}
};
int main(){Mob human,npc,bot;human.client=true;bot.bot=true;Target t;
assert(t.mez(&human));assert(t.mez(&bot));assert(!t.mez(&npc));
t.level=40;assert(!t.mez(&bot));assert(!t.mez(&human));
assert(!t.stunExempt(&bot));assert(t.stunExempt(&npc));assert(!t.stunExempt(&human));
t.level=41;assert(t.spin(&bot));assert(t.spin(&human));assert(!t.spin(&npc));
t.level=40;assert(!t.spin(&bot));t.immune=true;assert(t.spin(&npc));
std::cout<<"13 extracted crowd-control boundary checks passed\n";}
