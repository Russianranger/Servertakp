#include <cassert>
#include <initializer_list>
#include <iostream>
namespace SpecialAbility { enum {AllowedToTank}; }
struct Mob {bool client,bot,melee,tank=false; int hate; bool IsClient(){return client;} bool GetSpecialAbility(int){return tank;}};
bool IsPlayerBot(Mob*m){return m->bot;}
Mob *choose(std::initializer_list<Mob*> entries) {
 bool clientInMeleeRange=false,mobInMeleeRange=false;
 Mob *topMob=nullptr,*topClient=nullptr,*topMeleeClient=nullptr;
 int topHate=-1,topClientHate=-1,topMeleeClientHate=-1;
 struct Entry {Mob*ent;};
 for(auto m:entries) {Entry entry{m}; auto cur=&entry; bool isInMeleeRange=m->melee; int currentHate=m->hate;if (isInMeleeRange)
		{
			mobInMeleeRange = true;
			if (cur->ent->IsClient() || IsPlayerBot(cur->ent))
				clientInMeleeRange = true;
		}

		if (cur->ent->IsClient() || IsPlayerBot(cur->ent))
		{
			if (currentHate > topClientHate)
			{
				topClientHate = currentHate;
				topClient = cur->ent;
			}
			if (isInMeleeRange && currentHate > topMeleeClientHate)
			{
				topMeleeClientHate = currentHate;
				topMeleeClient = cur->ent;
			}
		}
		if(currentHate>topHate){topHate=currentHate;topMob=cur->ent;}
}if (!clientInMeleeRange)
		return topMob;
	else
	{
		if (topMob == topClient)
			return topClient;
		else if (topMob && topMob->GetSpecialAbility(SpecialAbility::AllowedToTank))
			return topMob;
		else
			return topMeleeClient;
	}
}int main(){
 Mob player{true,false,true,false,20},bot{false,true,true,false,100},pet{false,false,true,false,200};
 assert(choose({&player,&bot})==&bot);
 player.hate=150; assert(choose({&player,&bot})==&player);
 assert(choose({&player,&pet})==&player);
 player.hate=20; assert(choose({&player,&bot,&pet})==&bot);
 bot.melee=false; assert(choose({&player,&bot})==&bot);
 player.melee=false; assert(choose({&player,&pet})==&pet);
 bot.melee=true; assert(choose({&bot,&pet})==&bot);
 std::cout<<"7 extracted hate-selection regression cases passed\n";
}
