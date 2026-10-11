#!/usr/bin/env python3
"""Compile the production bot spell policy against the shipped TAKP spell rows.

The generated native fixture uses the actual LoadClassSpells, HealTarget,
CastCombatBuff and CastClassSpells bodies, not copies of the policy. Engine
primitives model target, stacking, cast-start and timer responses deterministically;
this is not a client or whole-zone integration test.
"""
from pathlib import Path
import argparse
import csv
import json
import os
import re
import shlex
import subprocess
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[2]
YAULP = {43, 44, 210, 1534, 2326, 3186}


def spells_from_seed():
    with zipfile.ZipFile(ROOT / 'utils/sql/database_full/alkabor_latest.zip') as archive:
        entry = max((i for i in archive.infolist() if i.filename.endswith('.sql')),
                    key=lambda i: i.file_size)
        sql = archive.read(entry).decode('utf-8')
    start = sql.index('CREATE TABLE `spells_new`')
    columns = re.findall(r'^\s*`([^`]+)`', sql[start:sql.index('ENGINE=', start)], re.M)
    result = {}
    for batch in re.findall(r'INSERT INTO `spells_new` VALUES (.*);', sql):
        for raw in re.findall(r'\((.*?)(?:\),|\)$)', batch):
            row = next(csv.reader([raw], quotechar="'", escapechar='\\', skipinitialspace=True))
            if len(row) != len(columns):
                raise AssertionError('Unexpected native spell row width')
            value = dict(zip(columns, row))
            result[int(value['id'])] = value
    return result


def production_function(source, signature, fallback=None):
    if signature not in source:
        if fallback is not None:
            return fallback
        raise AssertionError('Production function missing: ' + signature)
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 1
    cursor = opening + 1
    while depth:
        depth += (source[cursor] == '{') - (source[cursor] == '}')
        cursor += 1
    return source[start:cursor]


PRELUDE = r'''
#include <cassert>
#include <algorithm>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <iostream>
using uint16=uint16_t; using uint32=uint32_t;
const int SPDAT_RECORDS=8000,ST_Target=5,ST_Self=6,ST_Group=41,ST_Corpse=3,ST_Pet=14;
const int SE_CurrentHP=0,SE_ArmorClass=1,SE_ATK=2,SE_MovementSpeed=3,SE_AttackSpeed=11;
const int SE_CurrentMana=15,SE_Revive=81,SE_Lull=18,SE_Invisibility=12;
const int SE_InvisVsUndead=28,SE_Levitate=57,SE_WaterBreathing=14,SE_Root=99,SE_TotalHP=69;
const int SpellType_Nuke=1,SpellType_Heal=2,SpellType_Root=4,SpellType_Buff=8;
const int SpellType_Lifetap=64,SpellType_Snare=128,SpellType_DOT=256,SpellType_InCombatBuff=1024,SpellType_Slow=8192;
namespace Class { const int Cleric=2,Bard=8; }
struct Spell { bool valid=false,short_combat=false; int classes[16]{},not_player_spell=0,targettype=0;
 int mana=0,buffduration=0,effectid[12]{},base[12]{}; bool beneficial=true; float range=100; };
Spell spells[SPDAT_RECORDS];
bool IsValidSpell(int id){return id>0&&id<SPDAT_RECORDS&&spells[id].valid;}
bool IsBeneficialSpell(int id){return spells[id].beneficial;}
bool IsDetrimentalSpell(int id){return !IsBeneficialSpell(id);}
bool IsEffectInSpell(int id,int effect){return std::find(spells[id].effectid,spells[id].effectid+12,effect)!=spells[id].effectid+12;}
bool IsHealingSpell(int id){return IsBeneficialSpell(id)&&IsEffectInSpell(id,SE_CurrentHP);}
bool IsDamageSpell(int id){for(int n=0;n<12;++n)if(spells[id].effectid[n]==SE_CurrentHP&&spells[id].base[n]<0)return true;return false;}
bool IsDOTSpell(int id){return spells[id].buffduration>0&&IsDamageSpell(id);}
bool IsPureNukeSpell(int id){return !spells[id].buffduration&&IsDamageSpell(id);}
bool IsHasteSpell(int id){return IsEffectInSpell(id,SE_AttackSpeed);}
bool IsCureSpell(int){return false;} bool IsSummonPetSpell(int){return false;}
bool IsMezSpell(int){return false;} bool IsCharmSpell(int){return false;}
bool IsFearSpell(int){return false;} bool IsEvacSpell(int){return false;}
bool IsSummonSpell(int){return false;} bool IsSlowSpell(int){return false;}
bool IsLifetapSpell(int){return false;} bool IsInvulnerabilitySpell(int){return false;}
float DistanceSquared(float a,float b){return (a-b)*(a-b);}
struct Timer { bool ready=true; bool Check(){return ready;} static uint32 now; static uint32 GetCurrentTime(){return now;} }; uint32 Timer::now=100000;
struct Mob { virtual ~Mob()=default; float hp=100,pos=0; bool dead=false;
 float GetHPRatio(){return hp;} bool HasDied(){return dead;} float GetPosition(){return pos;}
 int CanBuffStack(int,int,bool){return 0;} };
struct Entry { uint16 spellid,type; uint32 time_cancast=0; };
struct PlayerBot:Mob {
 int level=65,role=Class::Cleric,mana=1000,stance_=1; bool melee=true,permitted=true,immune=false;
 bool control=false,cure=false,rotation=false,rotation_configured=false,pet_enabled_=false,active_family=false,block_regular=true;
 Timer spell_timer_; Mob owner,enemy; bool owner_present=true; std::map<std::string,int> options_;
 std::vector<Entry> AIspells; std::vector<int> songs_,utility_spells_; size_t next_song_=0; std::set<int> denied;
 std::vector<int> casts; std::vector<int> attempts; int cured=0; Mob *healed=nullptr;
 int GetClass(){return role;} int GetLevel(){return level;} int GetMana(){return mana;}
 int GetActSpellCost(int,int cost){return cost;} Mob *Owner(){return owner_present?&owner:nullptr;}
 std::vector<Mob*> Friends(){return {};} Mob *GetTarget(){return &enemy;}
 bool Allowed(Mob*){return permitted;} bool CombatRange(Mob*){return melee;}
 bool IsCasting(){return !casts.empty();} bool ControlAdds(){return control;}
 bool RotationHeal(bool &configured){configured=rotation_configured;return rotation;}
 bool Cure(Mob*){if(cure){++cured;return true;}return false;}
 bool Utility(const std::string&,Mob*){return false;} bool GetPet(){return false;}
 bool TrySpell(int,Mob*){return false;} bool IsImmuneToSpell(int,Mob*){return immune;}
 int CanBuffStack(int,int,bool){return active_family?-1:0;}
 void AddSpellToNPCList(int,int id,int kind,int,int,int){AIspells.push_back({uint16(id),uint16(kind),0});}
 bool AIDoSpellCast(int index,Mob *target,int cost){int id=AIspells[index].spellid;attempts.push_back(id);
  if(denied.count(id))return false;
  casts.push_back(id);mana-=cost;if(AIspells[index].type==SpellType_Heal)healed=target;return true;}
 bool AICastSpell(Mob *target,int,int kinds){
  for(int n=int(AIspells.size())-1;n>=0;--n){const auto &e=AIspells[n];
   if(!(e.type&kinds)||e.time_cancast>Timer::now)continue;
   if(spells[e.spellid].targettype==ST_Self&&target!=this)continue;
   if(e.type==SpellType_Buff&&block_regular&&!spells[e.spellid].short_combat)continue;
   if(e.type==SpellType_Buff&&spells[e.spellid].short_combat&&active_family)continue;
   if(AIDoSpellCast(n,target,spells[e.spellid].mana))return true;
  }return false;
 }
 void LoadClassSpells(); bool HealTarget(Mob*); bool CastCombatBuff(); bool CastClassSpells(bool);
};
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--source', type=Path, default=ROOT/'zone/player_bot.cpp')
    args = parser.parse_args()
    rows = spells_from_seed()
    candidates = set()
    for id_, row in rows.items():
        effects = {int(row['effectid'+str(n)]) for n in range(1, 13)}
        if (0 < int(row['classes2']) <= 65 and row['not_player_spell'] == '0'
                and row['targettype'] == '6' and row['goodEffect'] != '0'
                and 0 < int(row['buffduration']) <= 4 and effects & {1, 2, 11, 69}):
            candidates.add(id_)
    assert candidates == YAULP, ('The native short cleric self-buff family changed', candidates)
    support = {id_ for id_, row in rows.items() if row['name'] in {'Courage', 'Minor Healing'}
               and 0 < int(row['classes2']) <= 65 and row['not_player_spell'] == '0'}
    assert support
    fixture = ['void Seed(){']
    for id_ in sorted(YAULP | support):
        row = rows[id_]
        fixture.append('{auto &s=spells['+str(id_)+'];s.valid=true;')
        fixture.append('s.short_combat='+('true' if id_ in candidates else 'false')+';')
        for key in ('targettype', 'mana', 'buffduration', 'not_player_spell'):
            fixture.append('s.'+key+'='+row[key]+';')
        fixture.append('s.beneficial='+('true' if row['goodEffect'] != '0' else 'false')+';')
        for n in range(15):
            fixture.append('s.classes['+str(n)+']='+row['classes'+str(n+1)]+';')
        for n in range(12):
            fixture.append('s.effectid['+str(n)+']='+row['effectid'+str(n+1)]+';')
            fixture.append('s.base['+str(n)+']='+row['effect_base_value'+str(n+1)]+';')
        fixture.append('}')
    fixture.append('}')
    source = args.source.read_text()
    functions = [production_function(source, signature, fallback) for signature, fallback in (
        ('void PlayerBot::LoadClassSpells()', None),
        ('bool PlayerBot::HealTarget(Mob *target)', None),
        ('bool PlayerBot::CastCombatBuff()', 'bool PlayerBot::CastCombatBuff(){return false;}'),
        ('bool PlayerBot::CastClassSpells(bool combat)', None))]
    # Fixtures only need the resist adjustment field populated by the real loader.
    native = PRELUDE.replace('int mana=0,buffduration=0,', 'int ResistDiff=0,mana=0,buffduration=0,')
    native += '\n'.join(fixture + functions)
    native += r'''
int main(){Seed();
 const int ranks[]={210,43,44,1534,2326,3186};
 for(int rank:ranks){PlayerBot b;b.level=spells[rank].classes[Class::Cleric-1];b.LoadClassSpells();
  assert(!b.CastClassSpells(false));assert(b.casts.empty());
  const auto e=std::find_if(b.AIspells.begin(),b.AIspells.end(),[&](const auto&s){return s.spellid==rank;});
  assert(e!=b.AIspells.end()&&e->type==SpellType_InCombatBuff);
  assert(b.CastClassSpells(true));assert(b.casts.back()==rank);
 }
 auto fresh=[](){PlayerBot b;b.LoadClassSpells();return b;};
 {auto b=fresh();b.owner.hp=40;assert(b.CastClassSpells(true));assert(b.healed==&b.owner);assert(b.attempts.size()==1);}
 {auto b=fresh();b.cure=true;assert(b.CastClassSpells(true));assert(b.cured==1&&b.casts.empty());}
 {auto b=fresh();b.control=true;assert(b.CastClassSpells(true));assert(b.casts.empty());}
 {auto b=fresh();b.rotation=true;assert(b.CastClassSpells(true));assert(b.casts.empty());}
 {auto b=fresh();b.spell_timer_.ready=false;assert(!b.CastClassSpells(true));assert(b.attempts.empty());}
 {auto b=fresh();b.owner_present=false;assert(!b.CastClassSpells(true));assert(b.attempts.empty());}
 {auto b=fresh();b.options_["suspend"]=1;assert(!b.CastClassSpells(true));assert(b.attempts.empty());}
 {auto b=fresh();b.stance_=0;assert(!b.CastClassSpells(true));assert(b.attempts.empty());}
 {auto b=fresh();b.permitted=false;assert(!b.CastClassSpells(true));assert(b.attempts.empty());}
 {auto b=fresh();b.melee=false;assert(!b.CastClassSpells(true));assert(b.attempts.empty());}
 {auto b=fresh();b.mana=0;assert(!b.CastClassSpells(true));assert(b.attempts.empty());}
 {auto b=fresh();for(auto&e:b.AIspells)e.time_cancast=Timer::now+1;assert(!b.CastClassSpells(true));assert(b.attempts.empty());}
 {auto b=fresh();b.immune=true;assert(!b.CastClassSpells(true));assert(b.attempts.empty());}
 {auto b=fresh();b.active_family=true;assert(!b.CastClassSpells(true));assert(b.attempts.empty());
  b.active_family=false;assert(b.CastClassSpells(true));assert(b.casts.back()==3186);}
 {auto b=fresh();for(int rank:ranks)b.denied.insert(rank);assert(!b.CastClassSpells(true));assert(b.casts.empty());assert(b.attempts.size()==6);}
 {auto b=fresh();b.denied.insert(3186);assert(b.CastClassSpells(true));assert(b.attempts.size()==2&&b.casts.back()==2326);}
 {auto b=fresh();b.block_regular=false;assert(b.CastClassSpells(false));assert(b.casts.back()!=3186&&b.casts.back()!=2326);}
 {auto b=fresh();b.casts.push_back(200);assert(!b.CastClassSpells(true));assert(b.attempts.empty());}
 // Yaulp V/VI's native mana regeneration remains available during permitted melee.
 assert(IsEffectInSpell(2326,SE_CurrentMana)&&IsEffectInSpell(3186,SE_CurrentMana));
 std::cout<<"Six seed-native Yaulp ranks and 18 scheduling/gate/priority checks passed\n";
}
'''
    with tempfile.TemporaryDirectory(prefix='takp-yaulp-tests-') as tmp:
        cpp = Path(tmp)/'production-spell-ai.cpp'
        binary = Path(tmp)/('production-spell-ai.exe' if os.name == 'nt' else 'production-spell-ai')
        cpp.write_text(native)
        subprocess.run(shlex.split(os.environ.get('CXX', 'c++')) +
                       ['-std=c++20', '-Wall', '-Wextra', '-Werror', str(cpp), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
    print(json.dumps({'native_short_self_buff_ids': sorted(candidates),
                      'production_functions': 4, 'result': 'passed'}))


if __name__ == '__main__':
    main()
