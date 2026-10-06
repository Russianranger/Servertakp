
#include <map>
#include <vector>
#include <cassert>
#include <iostream>
#include <cstdint>
using uint16=uint16_t;using int16=int16_t;using int8=int8_t;
enum {ST_AECaster,ST_AETarget,ST_GroupTeleport,ST_Group,ST_UndeadAE,ST_SummonedAE,ST_AEBard};
namespace BodyType{enum{SummonedUndead,Undead,Vampire,Summoned,Summoned2,Summoned3};}
namespace Chat{enum{SpellFailure};} namespace StringID{enum{SPELL_NO_HOLD,NO_COMPONENT_LUCLIN};}
enum FACTION_VALUE{FACTION_AMIABLY=4,FACTION_INDIFFERENT=5,FACTION_THREATENINGLY=8,FACTION_SCOWLS=9};
const int SPELL_DIMENSIONAL_RETURN=100,INVALID_INDEX=-1;
#define LogSpellsDetail(...) ((void)0)
#define LogSpells(...) ((void)0)
struct Spell {int targettype=ST_AETarget,mana=10;float aoerange=50;bool npc_no_los=false,bad=true,direct=true,harmony=false,mez=false,blur=false,neutral=false,bard=false;};
Spell spells[4000];
bool IsDetrimentalSpell(int i){return spells[i].bad;}
bool IsHarmonySpell(int i){return spells[i].harmony;}
bool HasDirectDamageEffect(int i){return spells[i].direct;}
bool IsLuclinPortSpell(int){return false;}
bool IsNeutralSpell(int i){return spells[i].neutral;}
bool IsBardSong(int i){return spells[i].bard;}
bool IsTargetableAESpell(int i){return spells[i].targettype==ST_AETarget;}
bool IsMemBlurSpell(int i){return spells[i].blur;}
bool IsMezSpell(int i){return spells[i].mez;}
float DistanceSquared(float a,float b){return (a-b)*(a-b);}
struct Zone{bool SkipLoS(){return false;}} world;Zone*zone=&world;
struct Inv{int HasItem(int){return 1;}};
struct Mob{
 virtual ~Mob()=default;
 bool client=false,bot=false,beacon=false,pet=false,attackable=true,beneficial=false,los=true,aggro=false,untargetable=false;
 int body=99,hits=0;float pos=0;FACTION_VALUE faction=FACTION_INDIFFERENT;std::vector<Mob*> landed;
 bool IsClient(){return client;} bool IsNPC(){return !client && !beacon;}
 bool IsBeacon(){return beacon;}bool IsPet(){return pet;}bool IsUnTargetable(){return untargetable;}
 bool IsTrap(){return false;}bool IsHorse(){return false;}
 Mob*CastToClient(){return this;} Mob*CastToBeacon(){return this;}
 bool ClientFinishedLoading(){return true;}float GetAOERange(int i){return spells[i].aoerange;}
 int GetTargetsHit(){return hits;}void SetTargetsHit(int n){hits=n;}
 const char* GetName(){return "fixture";} const char* GetCleanName(){return "fixture";}
 int GetOrigBodyType(){return body;}float GetPosition(){return pos;}
 bool IsAttackAllowed(Mob*m,bool=true,int=0){return m!=this&&m->attackable;}
 FACTION_VALUE GetReverseFactionCon(Mob*){return faction;}
 bool CheckAggro(Mob*m){return m->aggro;}bool InSameGroup(Mob*){return false;}bool InSameRaid(Mob*){return false;}
 bool CheckLosFN(Mob*m,bool=true){return m->los;}int GetID(){return 1;}
 template<typename...T>void Message_StringID(T...){} Inv GetInv(){return{};}void DeleteItemInInventory(int){}
 void SpellOnTarget(int,Mob*m,bool=false,bool=false,int16=0,bool=false,uint16=0){landed.push_back(m);}
};
bool IsPlayerBot(Mob*m){return m&&m->bot;}
bool IsValidSpell(int i){return i>0&&i<4000;}bool IsBeneficialSpell(int i){return !spells[i].bad;}
struct PlayerBot:Mob{PlayerBot(){bot=true;}bool Allowed(Mob*m){return IsAttackAllowed(m);}bool Friendly(Mob*m){return m==this;}bool IsBeneficialAllowed(Mob*m){return m->beneficial;}};
bool PlayerBotSpellTargetAllowed(Mob *m,Mob *target,unsigned short id){
 auto *b=dynamic_cast<PlayerBot*>(m);if(!b)return true;
 if(!target || !IsValidSpell(id))return false;
 if(target==b)return true;
 if(IsHarmonySpell(id))return b->Allowed(target);
 if(IsBeneficialSpell(id) || IsNeutralSpell(id)) {
  if(b->Friendly(target) || b->IsBeneficialAllowed(target))return true;
  if(IsNeutralSpell(id))return b->Allowed(target);
  return false;
 }
 return b->Allowed(target);
}
struct EntityList{std::map<int,Mob*>mob_list;void AESpell(Mob*,Mob*,uint16,bool,int16,Mob*,bool);void MassGroupBuff(Mob*,Mob*,uint16);};
void EntityList::AESpell(Mob *caster, Mob *center, uint16 spell_id, bool affect_caster, int16 resist_adjust, Mob* spell_target, bool initial_cast)
{
	Mob *curmob = nullptr;

	if (!caster) return;
	float dist = caster->GetAOERange(spell_id);
	// raid boss NPCs get a radius extension to PBAoE spells
	if (caster->IsNPC() && !IsPlayerBot(caster) && spells[spell_id].targettype == ST_AECaster && spells[spell_id].mana == 0)
		dist *= 1.25f;

	float dist2 = dist * dist;
	float dist_targ = 0;

	bool detrimental = IsDetrimentalSpell(spell_id);
	const bool bot_caster = IsPlayerBot(caster);
	bool clientcaster = caster->IsClient() || bot_caster;
	int MAX_TARGETS_ALLOWED = 5;

	// Wizard's Al'Kabor line of spells hits 5 targets.
	static const int16 target_exemptions[] = { 382, 458, 459, 460, 731, 1650, 1651, 1652 };

	if (caster->IsNPC() && !bot_caster)
		MAX_TARGETS_ALLOWED = 999;
	else if (HasDirectDamageEffect(spell_id))
	{
		// Damage Spells were limited to 4 targets.
		bool exempt = false;
		int8 size = sizeof(target_exemptions) / sizeof(target_exemptions[0]);
		for (int i = 0; i < size; i++) {
			if (spell_id == target_exemptions[i])
			{
				exempt = true;
				break;
			}
		}

		if (!exempt)
		{
			MAX_TARGETS_ALLOWED = 4;
		}
	}

	int targets_hit = 0;
	if (center->IsBeacon())
		targets_hit = center->CastToBeacon()->GetTargetsHit();

	for (auto it = mob_list.begin(); it != mob_list.end(); ++it) {
		curmob = it->second;
		if (!curmob)
			continue;
		// test to fix possible cause of random zone crashes..external methods accessing client properties before they're initialized
		if (curmob->IsClient() && !curmob->CastToClient()->ClientFinishedLoading())
			continue;
		if (curmob == center && (center->IsBeacon() || (spells[spell_id].targettype != ST_AETarget && spells[spell_id].targettype != ST_GroupTeleport && spells[spell_id].targettype != ST_Group)))
			continue;
		if (curmob == caster && !affect_caster)	//watch for caster too
			continue;
		// Some scripts have the trigger cast on itself (Nexus Scions) so we want to allow that.
		if (curmob->IsUnTargetable() && (curmob != spell_target || clientcaster))
		{
			LogSpellsDetail("Invalid target: Attempting to cast an AE spell on [{}], which is untargetable.", curmob->GetName());
			continue;
		}
		if (curmob->IsTrap())
			continue;
		if (curmob->IsHorse())
			continue;
		// undead aoe
		if (spells[spell_id].targettype == ST_UndeadAE)
		{
			if (curmob->GetOrigBodyType() != BodyType::SummonedUndead && curmob->GetOrigBodyType() != BodyType::Undead && curmob->GetOrigBodyType() != BodyType::Vampire)
				continue;
		}
		// summoned aoe
		if (spells[spell_id].targettype == ST_SummonedAE)
		{
			if (curmob->GetOrigBodyType() != BodyType::SummonedUndead && curmob->GetOrigBodyType() != BodyType::Summoned && curmob->GetOrigBodyType() != BodyType::Summoned2 && curmob->GetOrigBodyType() != BodyType::Summoned3)
				continue;
		}

		dist_targ = DistanceSquared(curmob->GetPosition(), center->GetPosition());

		if (dist_targ > dist2)	//make sure they are in range
			continue;
		// Bots use owner-aware permissions for all area spells, not NPC faction.
		if (bot_caster && !PlayerBotSpellTargetAllowed(caster,curmob,spell_id))
			continue;
		if (!clientcaster && curmob->IsNPC()) {	//check npc->npc casting
			FACTION_VALUE f = curmob->GetReverseFactionCon(caster);
			if (detrimental) {
				//affect mobs that are on our hate list, or
				//which have bad faction with us
				if (!(caster->CheckAggro(curmob) || f == FACTION_THREATENINGLY || f == FACTION_SCOWLS))
					continue;
			}
			else {
				//only affect mobs we would assist.
				if (!(f <= FACTION_AMIABLY) && !IsLuclinPortSpell(spell_id))
					continue;
				if ((spells[spell_id].targettype == ST_Group || spells[spell_id].targettype == ST_GroupTeleport) && curmob->IsPet())
					continue;
			}
		}
		if (detrimental) {
			// aoe spells do hit other players except if in same raid or group.  their pets get hit even when grouped.  SpellOnTarget checks pvp protection
			if (caster != curmob && (caster->InSameGroup(curmob) || caster->InSameRaid(curmob)))
				continue;

			if (!zone->SkipLoS() && !spells[spell_id].npc_no_los && curmob != caster && !center->CheckLosFN(curmob, true))
				continue;
		}
		else {
			// Balance of the Nameless, Cazic's Gift, recourse spells
			// AESpell is called for ST_GroupTeleport spells cast by NPCs.  The faction check above already filtered out unfriendly NPCs.
			if(!clientcaster && (spells[spell_id].targettype == ST_GroupTeleport || spells[spell_id].targettype == ST_Group))
			{
				if (!curmob->IsNPC())
					continue;
			}
			// check to stop casting beneficial ae buffs (to wit: bard songs) on enemies...
			// This does not check faction for beneficial AE buffs..only agro and attackable.
			// I've tested for spells that I can find without problem, but a faction-based
			// check may still be needed. Any changes here should also reflect in AEBardPulse()
			else if (!IsNeutralSpell(spell_id))
			{
				if (caster->IsAttackAllowed(curmob, true))
				{
					LogSpells("Invalid target: Attempting to cast a beneficial AE spell/song on [{}].", curmob->GetName());
					if(!IsBardSong(spell_id) && spells[spell_id].targettype != ST_AEBard)
						caster->Message_StringID(Chat::SpellFailure, StringID::SPELL_NO_HOLD);
					continue;
				}
				else if (IsBardSong(spell_id) && curmob->IsPet())
				{
					LogSpells("Invalid target: Attempting to cast a beneficial AE song on [{}] who is a pet.", curmob->GetName());
					//caster->Message_StringID(Chat::SpellFailure, SPELL_NO_HOLD);
					continue;
				}
			}
			if (caster->CheckAggro(curmob) && spell_id != SPELL_DIMENSIONAL_RETURN) // exception for An_unseen_entity returning people from stomach event in potorment
				continue;
		}

		//Check Journey: Luclin spell for Spire Stone in the player's inventory.
		if(spell_id == 2935 && curmob->IsClient())
		{
			int16 slotid = curmob->CastToClient()->GetInv().HasItem(19720);
			if(slotid == INVALID_INDEX)
			{
				curmob->Message_StringID(Chat::SpellFailure, StringID::NO_COMPONENT_LUCLIN);
				LogSpellsDetail("[{}] does not have the correct component to travel to Luclin.", curmob->GetName());
				continue;
			}
			else
			{
				curmob->CastToClient()->DeleteItemInInventory(slotid);
			}
		}

		uint16 ae_caster_id = center && !initial_cast ? center->GetID() : 0;

		//if we get here... cast the spell.
		if (IsTargetableAESpell(spell_id) && detrimental && !IsHarmonySpell(spell_id) && (!IsMemBlurSpell(spell_id) || IsMezSpell(spell_id)))
		{
			if (targets_hit < MAX_TARGETS_ALLOWED || curmob->IsClient())
			{
				caster->SpellOnTarget(spell_id, curmob, false, true, resist_adjust, false, ae_caster_id);
				if (curmob->IsNPC() && caster->IsAttackAllowed(curmob, true, spell_id))
					++targets_hit;
				LogSpellsDetail("Targeted AE Spell: [{}] has hit target #[{}]/[{}]: [{}]", spell_id, targets_hit, MAX_TARGETS_ALLOWED, curmob->GetCleanName());
			}
		}
		else
		{
			LogSpellsDetail("Non-limited AE Spell: [{}] has hit target [{}]", spell_id, curmob->GetCleanName());
			caster->SpellOnTarget(spell_id, curmob, false, true, resist_adjust, false, ae_caster_id);
		}
	}

	if(center->IsBeacon())
		center->CastToBeacon()->SetTargetsHit(targets_hit);
}
void EntityList::MassGroupBuff(Mob *caster, Mob *center, uint16 spell_id)
{
	float dist = caster->GetAOERange(spell_id);
	float dist2 = dist * dist;

	for (auto it = mob_list.begin(); it != mob_list.end(); ++it)
	{
		Mob *curclient = it->second;
        if(!curclient->IsClient() && !IsPlayerBot(curclient))continue;
		if (curclient->IsClient() && !curclient->CastToClient()->ClientFinishedLoading())
			continue;
		if (DistanceSquared(center->GetPosition(), curclient->GetPosition()) > dist2)	//make sure they are in range
			continue;

		caster->SpellOnTarget(spell_id, curclient);
	}
}
int main(){EntityList e;PlayerBot bot;Mob npc,target,ally,protectedMob;bot.bot=true;target.pos=2;ally.client=true;ally.attackable=false;ally.beneficial=true;protectedMob.attackable=false;
 e.mob_list={{1,&target}};e.AESpell(&bot,&target,1,false,0,&target,true);assert(bot.landed.size()==1);
 e.AESpell(&npc,&target,1,false,0,&target,true);assert(npc.landed.empty());
 target.aggro=true;e.AESpell(&npc,&target,1,false,0,&target,true);assert(npc.landed.size()==1);target.aggro=false;
 bot.landed.clear();e.mob_list={{1,&target},{2,&ally},{3,&protectedMob}};e.AESpell(&bot,&target,1,false,0,&target,true);assert(bot.landed.size()==1&&bot.landed[0]==&target);
 target.los=false;bot.landed.clear();e.AESpell(&bot,&target,1,false,0,&target,true);assert(bot.landed.empty());target.los=true;
 spells[2].bad=false;spells[2].direct=false;bot.landed.clear();e.AESpell(&bot,&target,2,false,0,&target,true);assert(bot.landed.size()==1&&bot.landed[0]==&ally);
 std::vector<Mob> enemies(7);e.mob_list.clear();for(int i=0;i<7;++i)e.mob_list[i]=&enemies[i];bot.landed.clear();e.AESpell(&bot,&target,1,false,0,&target,true);assert(bot.landed.size()==4);
 spells[382]=spells[1];bot.landed.clear();e.AESpell(&bot,&target,382,false,0,&target,true);assert(bot.landed.size()==5);
 spells[3]=spells[1];spells[3].harmony=true;spells[3].direct=false;bot.landed.clear();e.AESpell(&bot,&target,3,false,0,&target,true);assert(bot.landed.size()==7);
 npc.landed.clear();for(auto&m:enemies)m.aggro=true;e.AESpell(&npc,&target,1,false,0,&target,true);assert(npc.landed.size()==7);
 e.mob_list={{1,&target}};target.pos=55;spells[4].targettype=ST_AECaster;spells[4].mana=0;bot.landed.clear();e.AESpell(&bot,&bot,4,false,0,&target,true);assert(bot.landed.empty());target.aggro=true;npc.landed.clear();e.AESpell(&npc,&npc,4,false,0,&target,true);assert(npc.landed.size()==1);

 assert(PlayerBotSpellTargetAllowed(&bot,&bot,2));
 assert(!PlayerBotSpellTargetAllowed(&bot,&target,2));
 assert(PlayerBotSpellTargetAllowed(&bot,&ally,2));
 bot.landed.clear();bot.pos=0;ally.pos=1;target.pos=2;e.mob_list={{1,&bot},{2,&ally},{3,&target}};e.MassGroupBuff(&bot,&bot,2);
 assert(bot.landed.size()==2);assert(bot.landed[0]==&bot);assert(bot.landed[1]==&ally);
 std::cout<<"18 extracted AE, spell permission and mass-group-buff regression cases passed\n";
}
