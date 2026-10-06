
#include <cassert>
#include <vector>
#include <iostream>
namespace EQ{namespace textures{enum{weaponPrimary};} namespace item{enum{ItemType1HPiercing=2};} namespace invslot{enum{slotPrimary};} namespace skills{enum{SkillBackstab};}}
enum class DoAnimation{Piercing};const int DMG_INVUL=-5;
struct Item{int ItemType=0;};struct DB{Item *item=nullptr;Item*GetItem(int){return item;}}database;
struct Mob{int hp=500;bool immune=false;int GetHP(){return hp;}bool IsImmuneToMelee(Mob*,int){return immune;}};
struct NPC:Mob{
 bool bot=true,behind=true,dbl=false;int skill=100,level=30,weaponDamage=10,attacks=0;
 struct{bool FrontalBackstabMinDmg=false;}aabonuses;
 struct Hit{int base,min,hate;};std::vector<Hit>hits;Mob*target=nullptr;
 Mob*GetTarget(){return target;}int GetEquipment(int){return 1;}bool BehindMob(Mob*,int,int){return behind;}int GetX(){return 0;}int GetY(){return 0;}
 void Attack(Mob*){++attacks;}int GetLevel(){return level;}int GetSkill(int){return skill;}bool CheckDoubleAttack(){return dbl;}
 int GetBaseDamage(Mob*,int){return weaponDamage;}bool HasDied(){return false;}
 void DoSpecialAttackDamage(Mob*,int,int base,int min,int hate,DoAnimation){hits.push_back({base,min,hate});}
 void DoBackstab(Mob*defender);
};bool IsPlayerBot(NPC*p){return p->bot;}
void NPC::DoBackstab(Mob*defender){if(IsPlayerBot(this)) {
  if(!defender)defender=GetTarget();
  if(!defender || defender==this)return;
  const auto *weapon=database.GetItem(GetEquipment(EQ::textures::weaponPrimary));
  if(!weapon || weapon->ItemType!=EQ::item::ItemType1HPiercing)return;
  const bool frontal=!BehindMob(defender,GetX(),GetY());
  if(frontal && !aabonuses.FrontalBackstabMinDmg){Attack(defender);return;}
  int stabs=!frontal && GetLevel()>54 && CheckDoubleAttack()?2:1;
  int base=((GetSkill(EQ::skills::SkillBackstab)*0.02f)+2.0f)*GetBaseDamage(defender,EQ::invslot::slotPrimary);
  int hate=base,minHit=GetLevel()>=60?GetLevel()*2:GetLevel()>50?GetLevel()*3/2:GetLevel();
  if(defender->IsImmuneToMelee(this,EQ::invslot::slotPrimary))minHit=DMG_INVUL;
  else if(frontal)base=1;
  for(int n=0;n<stabs && defender->GetHP()>0 && GetTarget() && !HasDied();++n)
   DoSpecialAttackDamage(defender,EQ::skills::SkillBackstab,base,minHit,hate,DoAnimation::Piercing);
  return;
 }}

int main(){NPC b;Mob t;b.target=&t;Item sword,dagger;dagger.ItemType=EQ::item::ItemType1HPiercing;
 b.DoBackstab(&t);assert(b.hits.empty());database.item=&sword;b.DoBackstab(&t);assert(b.hits.empty());
 database.item=&dagger;b.DoBackstab(&t);assert(b.hits.size()==1&&b.hits[0].base==40&&b.hits[0].hate==40&&b.hits[0].min==30);
 b.hits.clear();b.weaponDamage=20;b.DoBackstab(&t);assert(b.hits[0].base==80);
 b.hits.clear();b.behind=false;b.DoBackstab(&t);assert(b.hits.empty()&&b.attacks==1);
 b.aabonuses.FrontalBackstabMinDmg=true;b.DoBackstab(&t);assert(b.hits.size()==1&&b.hits[0].base==1);
 b.hits.clear();b.behind=true;b.level=60;b.dbl=true;b.DoBackstab(&t);assert(b.hits.size()==2&&b.hits[0].min==120);
 b.hits.clear();t.immune=true;b.DoBackstab(&t);assert(b.hits[0].min==DMG_INVUL);
 std::cout<<"8 extracted backstab regression cases passed\n";
}
