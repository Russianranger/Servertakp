#include <cassert>
#include <map>
#include <string>
#include <iostream>
using uint16=unsigned short; using uint32=unsigned int;
const int EFFECT_COUNT=12,SE_CurrentHP=0,SE_CurrentHPOnce=79;
struct Spell{int buffduration=0;int effectid[12]{};int base[12]{};bool detrimental=false;};
Spell spells[4096];
bool IsValidSpell(uint16 id){return id>0&&id<4096;}
bool IsDetrimentalSpell(uint16 id){return spells[id].detrimental;}
bool IsDamageSpell(uint16 spellid)
{
	for (int o = 0; o < EFFECT_COUNT; o++)
	{
		uint32 tid = spells[spellid].effectid[o];
		if ((tid == SE_CurrentHPOnce || tid == SE_CurrentHP) && spells[spellid].base[o] < 0)
			return true;
	}

	return false;
}
bool IsDOTSpell(uint16 spellid)
{
	if (spells[spellid].buffduration < 1)
		return false;

	for (int o = 0; o < EFFECT_COUNT; o++)
	{
		uint32 tid = spells[spellid].effectid[o];
		if (tid == SE_CurrentHP && spells[spellid].base[o] < 0)
			return true;
	}

	return false;
}
struct Bot {std::map<std::string,int> options_;
bool DirectDamageAllowed(uint16 id) const {
  auto setting=options_.find("dpscast");
  return setting==options_.end() || setting->second!=0 || !IsValidSpell(id) ||
         !IsDetrimentalSpell(id) || !IsDamageSpell(id) || IsDOTSpell(id);
 }
};
int main(){Bot b;
spells[91]={0,{0,254,254,254,254,254,254,254,254,254,254,254},{-23,0,0,0,0,0,0,0,0,0,0,0},true};
b.options_.clear();assert(b.DirectDamageAllowed(91));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(91)==false);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(91));
std::cout<<"Ignite: off=false"<<"\n";
spells[92]={0,{0,254,254,254,254,254,254,254,254,254,254,254},{-10,0,0,0,0,0,0,0,0,0,0,0},true};
b.options_.clear();assert(b.DirectDamageAllowed(92));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(92)==false);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(92));
std::cout<<"Burst of Fire: off=false"<<"\n";
spells[93]={0,{0,254,254,254,254,254,254,254,254,254,254,254},{-3,0,0,0,0,0,0,0,0,0,0,0},true};
b.options_.clear();assert(b.DirectDamageAllowed(93));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(93)==false);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(93));
std::cout<<"Burst of Flame: off=false"<<"\n";
spells[134]={4,{20,2,254,254,254,254,254,254,254,254,254,254},{-1,-5,0,0,0,0,0,0,0,0,0,0},true};
b.options_.clear();assert(b.DirectDamageAllowed(134));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(134)==true);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(134));
std::cout<<"Blinding Luminance: off=true"<<"\n";
spells[145]={205,{0,254,254,254,254,254,254,254,254,254,254,254},{10,0,0,0,0,0,0,0,0,0,0,0},false};
b.options_.clear();assert(b.DirectDamageAllowed(145));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(145)==true);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(145));
std::cout<<"Chloroplast: off=true"<<"\n";
spells[200]={0,{0,254,254,254,254,254,254,254,254,254,254,254},{10,0,0,0,0,0,0,0,0,0,0,0},false};
b.options_.clear();assert(b.DirectDamageAllowed(200));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(200)==true);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(200));
std::cout<<"Minor Healing: off=true"<<"\n";
spells[204]={0,{0,254,254,254,254,254,254,254,254,254,254,254},{-74,0,0,0,0,0,0,0,0,0,0,0},true};
b.options_.clear();assert(b.DirectDamageAllowed(204));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(204)==false);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(204));
std::cout<<"Shock of Poison: off=false"<<"\n";
spells[210]={4,{10,10,4,24,1,254,254,254,254,254,254,254},{0,0,10,-1,20,0,0,0,0,0,0,0},false};
b.options_.clear();assert(b.DirectDamageAllowed(210));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(210)==true);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(210));
std::cout<<"Yaulp: off=true"<<"\n";
spells[239]={8,{10,0,10,10,10,10,10,10,10,1,254,254},{0,-1,0,0,0,0,0,0,0,-12,0,0},true};
b.options_.clear();assert(b.DirectDamageAllowed(239));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(239)==true);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(239));
std::cout<<"Flame Lick: off=true"<<"\n";
spells[252]={0,{0,254,254,254,254,254,254,254,254,254,254,254},{-16,0,0,0,0,0,0,0,0,0,0,0},true};
b.options_.clear();assert(b.DirectDamageAllowed(252));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(252)==false);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(252));
std::cout<<"Invoke Lightning: off=false"<<"\n";
spells[264]={9,{0,254,254,254,254,254,254,254,254,254,254,254},{-13,0,0,0,0,0,0,0,0,0,0,0},true};
b.options_.clear();assert(b.DirectDamageAllowed(264));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(264)==true);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(264));
std::cout<<"Stinging Swarm: off=true"<<"\n";
spells[278]={360,{10,3,254,254,254,254,254,254,254,254,254,254},{0,30,0,0,0,0,0,0,0,0,0,0},false};
b.options_.clear();assert(b.DirectDamageAllowed(278));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(278)==true);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(278));
std::cout<<"Spirit of Wolf: off=true"<<"\n";
spells[313]={0,{0,254,254,254,254,254,254,254,254,254,254,254},{-8,0,0,0,0,0,0,0,0,0,0,0},true};
b.options_.clear();assert(b.DirectDamageAllowed(313));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(313)==false);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(313));
std::cout<<"Fire Flux: off=false"<<"\n";
spells[329]={0,{0,254,254,254,254,254,254,254,254,254,254,254},{-140,0,0,0,0,0,0,0,0,0,0,0},true};
b.options_.clear();assert(b.DirectDamageAllowed(329));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(329)==false);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(329));
std::cout<<"Wrath: off=false"<<"\n";
spells[341]={0,{0,254,254,254,254,254,254,254,254,254,254,254},{-3,0,0,0,0,0,0,0,0,0,0,0},true};
b.options_.clear();assert(b.DirectDamageAllowed(341));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(341)==false);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(341));
std::cout<<"Lifetap: off=false"<<"\n";
spells[434]={7,{36,79,0,254,254,254,254,254,254,254,254,254},{3,-30,-27,0,0,0,0,0,0,0,0,0},true};
b.options_.clear();assert(b.DirectDamageAllowed(434));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(434)==true);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(434));
std::cout<<"Envenomed Breath: off=true"<<"\n";
spells[826]={1,{57,64,79,254,254,254,254,254,254,254,254,254},{1,1,-100,1,0,0,0,0,0,0,0,0},true};
b.options_.clear();assert(b.DirectDamageAllowed(826));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(826)==false);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(826));
std::cout<<"Whirlwind: off=false"<<"\n";
spells[1776]={360,{10,3,254,254,254,254,254,254,254,254,254,254},{0,30,0,0,0,0,0,0,0,0,0,0},false};
b.options_.clear();assert(b.DirectDamageAllowed(1776));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(1776)==true);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(1776));
std::cout<<"Spirit of Wolf: off=true"<<"\n";
spells[2111]={0,{0,254,254,254,254,254,254,254,254,254,254,254},{-3,0,0,0,0,0,0,0,0,0,0,0},true};
b.options_.clear();assert(b.DirectDamageAllowed(2111));b.options_["dpscast"]=0;assert(b.DirectDamageAllowed(2111)==false);b.options_["dpscast"]=1;assert(b.DirectDamageAllowed(2111));
std::cout<<"Burst of Flame: off=false"<<"\n";
std::cout<<"57 actual-spell policy checks passed\n";}
