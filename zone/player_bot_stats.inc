// Player combat formulas adapted from this EQMacEmu revision's attack.cpp.
// Copyright EQEmu Development Team; GPL-2.0, as in the source repository.

int PlayerBot::GetBaseDamage(Mob *defender,int slot)
{
 if(slot<0||slot>=EQ::invslot::EQUIPMENT_COUNT)return 0;
 const auto *weapon=database.GetItem(equipment[slot]);
 if(!weapon)return slot==EQ::invslot::slotPrimary||slot==EQ::invslot::slotSecondary?GetHandToHandDamage():0;
 if(weapon->ReqLevel>GetLevel())return 0;
 auto scaled=[&](int value){return weapon->RecLevel>GetLevel()?value*GetLevel()/weapon->RecLevel:value;};
 int damage=scaled(weapon->Damage);
 if(defender) {
  if(weapon->ElemDmgAmt&&!defender->GetSpecialAbility(SpecialAbility::MagicImmunity))damage+=CalcEleWeaponResist(scaled(weapon->ElemDmgAmt),weapon->ElemDmgType,defender);
  if(weapon->BaneDmgBody==defender->GetBodyType()||weapon->BaneDmgRace==defender->GetRace())damage+=scaled(weapon->BaneDmgAmt);
 }
 return damage;
}

int PlayerBot::GetToHit(EQ::skills::SkillType skill)
{
 return 7+GetSkill(EQ::skills::SkillOffense)+GetSkill(skill)+itembonuses.Accuracy[EQ::skills::HIGHEST_SKILL+1]+spellbonuses.Accuracy[EQ::skills::HIGHEST_SKILL+1]+itembonuses.HitChance;
}

int PlayerBot::GetMitigation()
{
 int weight=0,shield_ac=0;
 for(int n=0;n<EQ::invslot::EQUIPMENT_COUNT;++n)if(const auto *item=database.GetItem(equipment[n])) {
  weight+=item->Weight;
  if(n==EQ::invslot::slotSecondary&&item->IsTypeShield())shield_ac=item->AC;
 }
 return Client::GetMitigation(false,itembonuses.AC,shield_ac,spellbonuses.AC,GetClass(),GetLevel(),GetBaseRace(),weight/10,GetAGI(),GetSkill(EQ::skills::SkillDefense),aabonuses.CombatStability);
}

void PlayerBot::SetAttackTimer(bool trigger)
{
 const auto *primary=database.GetItem(equipment[EQ::invslot::slotPrimary]);
 for(auto slot:{EQ::invslot::slotPrimary,EQ::invslot::slotSecondary,EQ::invslot::slotRange}) {
  auto *timer=slot==EQ::invslot::slotPrimary?&attack_timer:slot==EQ::invslot::slotSecondary?&attack_dw_timer:&ranged_timer;
  const auto *weapon=database.GetItem(equipment[slot]);
  if(slot==EQ::invslot::slotSecondary&&(!GetSkill(EQ::skills::SkillDualWield)||(primary&&primary->IsType2HWeapon())||(weapon&&weapon->IsTypeShield()))) {timer->Disable();continue;}
  if(slot==EQ::invslot::slotRange&&!weapon){timer->Disable();continue;}
  int delay=weapon&&weapon->Damage&&weapon->Delay?weapon->Delay:GetHandToHandDelay();
  const int speed=std::max<int>(RuleI(Combat,MinHastedDelay),delay*10000/std::max(1,GetHaste())+delay*100*(itembonuses.HundredHands+spellbonuses.HundredHands)/100);
  timer->SetDuration(speed,true);if(trigger)timer->Trigger();
 }
}


uint32 PlayerBot::RollDamageMultiplier(uint32 offense, int& damage, EQ::skills::SkillType skill)
{
	int rollChance = 51;
	int maxExtra = 210;
	int minusFactor = 105;

	if (GetClass() == Class::Monk && level >= 65)
	{
		rollChance = 83;
		maxExtra = 300;
		minusFactor = 50;
	}
	else if (level >= 65 || (GetClass() == Class::Monk && level >= 63))
	{
		rollChance = 81;
		maxExtra = 295;
		minusFactor = 55;
	}
	else if (level >= 63 || (GetClass() == Class::Monk && level >= 60))
	{
		rollChance = 79;
		maxExtra = 290;
		minusFactor = 60;
	}
	else if (level >= 60 || (GetClass() == Class::Monk && level >= 56))
	{
		rollChance = 77;
		maxExtra = 285;
		minusFactor = 65;
	}
	else if (level >= 56)
	{
		rollChance = 72;
		maxExtra = 265;
		minusFactor = 70;
	}
	else if (level >= 51 || GetClass() == Class::Monk)
	{
		rollChance = 65;
		maxExtra = 245;
		minusFactor = 80;
	}

	int baseBonus = (static_cast<int>(offense) - minusFactor) / 2;
	if (baseBonus < 10)
		baseBonus = 10;

	if (zone->random.Roll(rollChance))
	{
		uint32 roll;

		roll = zone->random.Int(0, baseBonus) + 100;
		if (roll > maxExtra)
			roll = maxExtra;

		damage = damage * roll / 100;

		if (level >= 55 && damage > 1 && skill != EQ::skills::SkillArchery && IsWarriorClass())
			damage++;

		return roll;
	}
	else
	{
		return 100;
	}
}

int PlayerBot::GetHandToHandDamage()
{
	static uint8 mnk_dmg[] = { 99,
		4, 4, 4, 4, 5, 5, 5, 5, 5, 6,           // 1-10
		6, 6, 6, 6, 7, 7, 7, 7, 7, 8,           // 11-20
		8, 8, 8, 8, 9, 9, 9, 9, 9, 10,          // 21-30
		10, 10, 10, 10, 11, 11, 11, 11, 11, 12, // 31-40
		12, 12, 12, 12, 13, 13, 13, 13, 13, 14, // 41-50
		14, 14, 14, 14, 14, 14, 14, 14, 14, 14, // 51-60
		14, 14 };                                // 61-62
	static uint8 bst_dmg[] = { 99,
		4, 4, 4, 4, 4, 5, 5, 5, 5, 5,        // 1-10
		5, 6, 6, 6, 6, 6, 6, 7, 7, 7,        // 11-20
		7, 7, 7, 8, 8, 8, 8, 8, 8, 9,        // 21-30
		9, 9, 9, 9, 9, 10, 10, 10, 10, 10,   // 31-40
		10, 11, 11, 11, 11, 11, 11, 12, 12 }; // 41-49

	if (GetClass() == Class::Monk)
	{
		if (equipment[12] == 10652 && GetLevel() > 50)		// Celestial Fists, monk epic
			return 9;
		if (level > 62)
			return 15;
		return mnk_dmg[level];
	}
	else if (GetClass() == Class::Beastlord)
	{
		if (level > 49)
			return 13;
		return bst_dmg[level];
	}
	return 2;
}

int PlayerBot::GetHandToHandDelay()
{
	int delay = 35;
	static uint8 mnk_hum_delay[] = { 99,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 1-10
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 11-20
		35, 35, 35, 35, 35, 35, 35, 34, 34, 34, // 21-30
		34, 33, 33, 33, 33, 32, 32, 32, 32, 31, // 31-40
		31, 31, 31, 30, 30, 30, 30, 29, 29, 29, // 41-50
		29, 28, 28, 28, 28, 27, 27, 27, 27, 26, // 51-60
		24, 22 };								// 61-62
	static uint8 mnk_iks_delay[] = { 99,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 1-10
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 11-20
		35, 35, 35, 35, 35, 35, 35, 35, 35, 34, // 21-30
		34, 34, 34, 34, 34, 33, 33, 33, 33, 33, // 31-40
		33, 32, 32, 32, 32, 32, 32, 31, 31, 31, // 41-50
		31, 31, 31, 30, 30, 30, 30, 30, 30, 29, // 51-60
		25, 23 };								// 61-62
	static uint8 bst_delay[] = { 99,
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 1-10
		35, 35, 35, 35, 35, 35, 35, 35, 35, 35, // 11-20
		35, 35, 35, 35, 35, 35, 35, 35, 34, 34, // 21-30
		34, 34, 34, 33, 33, 33, 33, 33, 32, 32, // 31-40
		32, 32, 32, 31, 31, 31, 31, 31, 30, 30, // 41-50
		30, 30, 30, 29, 29, 29, 29, 29, 28, 28, // 51-60
		28, 28, 28 };							// 61-63

	if (GetClass() == Class::Monk)
	{
		if (equipment[12] == 10652 && GetLevel() > 50)		// Celestial Fists, monk epic
			return 16;

		if (GetLevel() > 62)
			return GetRace() == Race::Iksar ? 21 : 20;

		return GetRace() == Race::Iksar ? mnk_iks_delay[level] : mnk_hum_delay[level];
	}
	else if (GetClass() == Class::Beastlord)
	{
		if (GetLevel() > 63)
			return 27;
		return bst_delay[level];
	}
	return 35;
}

int PlayerBot::GetDamageBonus()
{
	if (GetLevel() < 28 || !IsWarriorClass())
		return 0;

	int delay = 1;
	int bonus = 1 + (GetLevel() - 28) / 3;

	const EQ::ItemData* weapon = database.GetItem(equipment[EQ::invslot::slotPrimary]);

	if (!weapon)
		delay = GetHandToHandDelay();
	else
		delay = weapon->Delay;

	if ( weapon && (weapon->ItemType == EQ::item::ItemType2HSlash || weapon->ItemType == EQ::item::ItemType2HBlunt || weapon->ItemType == EQ::item::ItemType2HPiercing) )
	{
		if (delay <= 27)
			return bonus + 1;

		if (level > 29)
		{
			int level_bonus = (level - 30) / 5 + 1;
			if (level > 50)
			{
				level_bonus++;
				int level_bonus2 = level - 50;
				if (level > 67)
					level_bonus2 += 5;
				else if (level > 59)
					level_bonus2 += 4;
				else if (level > 58)
					level_bonus2 += 3;
				else if (level > 56)
					level_bonus2 += 2;
				else if (level > 54)
					level_bonus2++;
				level_bonus += level_bonus2 * delay / 40;
			}
			bonus += level_bonus;
		}
		if (delay >= 40)
		{
			int delay_bonus = (delay - 40) / 3 + 1;
			if (delay >= 45)
				delay_bonus += 2;
			else if (delay >= 43)
				delay_bonus++;
			bonus += delay_bonus;
		}
		return bonus;
	}
	return bonus;
}

int PlayerBot::GetOffense(EQ::skills::SkillType skill)
{
	int statBonus;

	if (skill == EQ::skills::SkillArchery || skill == EQ::skills::SkillThrowing)
	{
		statBonus = GetDEX();
	}
	else
	{
		statBonus = GetSTR();
	}

	int offense = GetSkill(skill) + spellbonuses.ATK + itembonuses.ATK + aabonuses.ATK + (statBonus >= 75 ? ((2 * statBonus - 150) / 3) : 0);
	if (offense < 1)
		offense = 1;

	if (GetClass() == Class::Ranger && GetLevel() > 54)
	{
		offense = offense + GetLevel() * 4 - 216;
	}

	return offense;
}

int32 PlayerBot::CalcMaxHP(bool unbuffed)
{
 int sta=GetSTA();if(sta>255)sta=255+(sta-255)/2;
 base_hp=GetLevel()*(GetClassLevelFactor()/10)*(300+sta)/300;
 int value=base_hp+itembonuses.HP;double bonus=0;
 auto nd=GetAA(aaNaturalDurability);if(nd){bonus=nd==1?2:nd==2?5:10;if(GetAA(aaPhysicalEnhancement))bonus+=2;}
 int pd=std::min<int>(GetAA(aaPlanarDurability),std::max<int>(0,GetLevel()-60));
 if(GetClass()==Class::Warrior)bonus+=1.5*pd;else if(GetClass()==Class::Paladin||GetClass()==Class::ShadowKnight)bonus+=1.25*pd;
 value+=int(value*bonus/100)+aabonuses.HP+5;if(!unbuffed)value+=spellbonuses.HP;
 value=std::min(32767,value);if(!unbuffed){max_hp=value;if(cur_hp>value)cur_hp=value;}return value;
}

int16 PlayerBot::Focus(focusType type,uint16 id)
{
 if(!IsValidSpell(id)||IsBardSong(id))return 0;
 int16 worn=0,buffed=0,aa=0;
 for(int slot=EQ::invslot::EQUIPMENT_BEGIN;slot<=EQ::invslot::EQUIPMENT_END;++slot){auto *item=database.GetItem(EquippedItem(slot));if(item&&IsValidSpell(item->Focus.Effect))worn=std::max(worn,CalcFocusEffect(type,item->Focus.Effect,id));}
 for(int slot=0;slot<GetMaxTotalSlots();++slot)if(IsValidSpell(buffs[slot].spellid))buffed=std::max(buffed,CalcFocusEffect(type,buffs[slot].spellid,id));
 for(auto &rank:aa_ranks_)aa=std::max(aa,CalcAAFocus(type,rank.first+rank.second-1,id));
 return worn+buffed+aa;
}

int32 PlayerBot::GetActSpellHealing(uint16 id,int32 value,Mob *target,bool hot)
{
 if(!target)target=this;if(!hot)value+=value*Focus(focusImprovedHeal,id)/100;
 if(spells[id].buffdurationformula==0){if(!IsPercentalHealSpell(id)&&zone->random.Roll(itembonuses.CriticalHealChance+spellbonuses.CriticalHealChance+aabonuses.CriticalHealChance))value*=2;value+=value*target->GetHealRate(id,this)/100;}
 return value;
}

int32 PlayerBot::GetActSpellDamage(uint16 id,int32 value,Mob *target)
{
 if(spells[id].targettype==ST_Self)return value;

 if(id==SPELL_IMP_HARM_TOUCH)value-=GetAA(aaUnholyTouch)*450;
 if(InstantDisc(id)){value=value*150/100;FadeDiscipline();}
 const int focused=value*Focus(focusImprovedDamage,id)/100;
 if(GetClass()==Class::ShadowKnight)value+=value*Focus(focusSpellDamageMult,id)/100;
 if(spells[id].buffdurationformula==0){
  bool critical=zone->random.Roll(itembonuses.CriticalSpellChance+spellbonuses.CriticalSpellChance+aabonuses.CriticalSpellChance);
  if(id==SPELL_IMP_HARM_TOUCH&&GetAA(aaSpellCastingFury)&&GetAA(aaUnholyTouch))critical=true;
  if(critical){int rank=GetAA(aaSpellCastingFury);value+=value*(rank==1?33:rank==2?66:100)/100;}
  else if(GetClass()==Class::Wizard&&GetLevel()>=RuleI(Spells,WizCritLevel)&&zone->random.Roll(((std::min(GetINT(),255)+std::min(GetDEX(),255))/2.0+32.0)/10000.0))value+=value*zone->random.Int(1,50)/100;
 }
 return value+focused;
}

int32 PlayerBot::CalcMaxMana()
{
 int value=0;
 if(GetCasterClass()!='N'&&!((GetClass()==Class::Ranger||GetClass()==Class::Paladin||GetClass()==Class::Beastlord)&&GetLevel()<9)){
  int stat=GetCasterClass()=='W'?GetWIS():GetINT();if(stat>200)stat=(stat-200)/2+200;if(stat>100)stat+=3*(stat-100)/2;
  int factor=15*GetLevel();value=stat*factor/200+factor+itembonuses.Mana+spellbonuses.Mana;
 }
 max_mana=std::max(0,std::min(32767,value));if(cur_mana>max_mana)cur_mana=max_mana;return max_mana;
}
