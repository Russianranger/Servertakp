// Adapted from this EQMacEmu revision; GPL-2.0, EQEmu Development Team.
uint8 PlayerBot::DisciplineUseLevel(uint8 disc_id)
{
	switch(disc_id)
	{
		case disc_aggressive:
			if(GetClass() == Class::Warrior)
				return 60;
			else
				return 0;
			break;
		case disc_precision:
			if(GetClass() == Class::Warrior)
				return 57;
			else
				return 0;
			break;
		case disc_defensive:
			if(GetClass() == Class::Warrior)
				return 55;
			else
				return 0;
			break;
		case disc_evasive:
			if(GetClass() == Class::Warrior)
				return 52;
			else
				return 0;
			break;
		case disc_ashenhand:
			if(GetClass() == Class::Monk)
				return 60;
			else
				return 0;
			break;
		case disc_furious:
			if(GetClass() == Class::Warrior)
				return 56;
			else if(GetClass() == Class::Monk || GetClass() == Class::Rogue)
				return 53;
			else
				return 0;
			break;
		case disc_stonestance:
			if(GetClass() == Class::Monk)
				return 51;
			else if(GetClass() == Class::Beastlord)
				return 55;
			else
				return 0;
			break;
		case disc_thunderkick:
			if(GetClass() == Class::Monk)
				return 52;
			else
				return 0;
			break;
		case disc_fortitude:
			if(GetClass() == Class::Warrior)
				return 59;
			else if(GetClass() == Class::Monk)
				return 54;
			else
				return 0;
			break;
		case disc_fellstrike:
			if(GetClass() == Class::Warrior)
				return 58;
			else if(GetClass() == Class::Monk)
				return 56;
			else if(GetClass() == Class::Beastlord)
				return 60;
			else if(GetClass() == Class::Rogue)
				return 59;
			else
				return 0;
			break;
		case disc_hundredfist:
			if(GetClass() == Class::Monk)
				return 57;
			else if(GetClass() == Class::Rogue)
				return 58;
			else
				return 0;
			break;
		case disc_charge:
			if(GetClass() == Class::Warrior)
				return 53;
			else if(GetClass() == Class::Rogue)
				return 54;
			else
				return 0;
			break;
		case disc_mightystrike:
			if(GetClass() == Class::Warrior)
				return 54;
			else
				return 0;
			break;
		case disc_nimble:
			if(GetClass() == Class::Rogue)
				return 55;
			else
				return 0;
			break;
		case disc_silentfist:
			if(GetClass() == Class::Monk)
				return 59;
			else
				return 0;
			break;
		case disc_kinesthetics:
			if(GetClass() == Class::Rogue)
				return 57;
			else
				return 0;
			break;
		case disc_holyforge:
			if(GetClass() == Class::Paladin)
				return 55;
			else
				return 0;
			break;
		case disc_sanctification:
			if(GetClass() == Class::Paladin)
				return 60;
			else
				return 0;
			break;
		case disc_trueshot:
			if(GetClass() == Class::Ranger)
				return 55;
			else
				return 0;
			break;
		case disc_weaponshield:
			if(GetClass() == Class::Ranger)
				return 60;
			else
				return 0;
			break;
		case disc_unholyaura:
			if(GetClass() == Class::ShadowKnight)
				return 55;
			else
				return 0;
			break;
		case disc_leechcurse:
			if(GetClass() == Class::ShadowKnight)
				return 60;
			else
				return 0;
			break;
		case disc_deftdance:
			if(GetClass() == Class::Bard)
				return 55;
			else
				return 0;
			break;
		case disc_puretone:
			if(GetClass() == Class::Bard)
				return 60;
			else
				return 0;
			break;
		case disc_resistant:
			if(GetClass() == Class::Warrior || GetClass() == Class::Monk || GetClass() == Class::Rogue)
				return 30;
			else if(GetClass() == Class::Paladin || GetClass() == Class::Ranger || GetClass() == Class::ShadowKnight || GetClass() == Class::Bard || GetClass() == Class::Beastlord)
				return 51;
			else
				return 0;
			break;
		case disc_fearless:
			if(GetClass() == Class::Warrior || GetClass() == Class::Monk || GetClass() == Class::Rogue)
				return 40;
			else if(GetClass() == Class::Paladin || GetClass() == Class::Ranger || GetClass() == Class::ShadowKnight || GetClass() == Class::Bard || GetClass() == Class::Beastlord)
				return 54;
			else
				return 0;
			break;
		default:
			return 0;
			break;
	}
}

uint8 PlayerBot::GetDiscTimerID(uint8 disc_id)
{
	if (RuleB(Combat, UseDiscTimerGroups)) {
		// This is for Discipline timer groups. Bard, Paladin, Ranger, SK, and Beastlord all use a single group.
		// Warrior, Monk, and Rogue can all have up to 3 discs available at a time.

		switch (disc_id)
		{
		case disc_kinesthetics:
			return 1;

			break;
		case disc_aggressive:
		case disc_precision:
		case disc_ashenhand:
		case disc_thunderkick:
		case disc_charge:
		case disc_mightystrike:
		case disc_silentfist:
			return 2;

			break;

		case disc_furious:
			if (GetClass() == Class::Warrior)
				return 1;
			else
				return 0;

			break;
		case disc_fortitude:
			if (GetClass() == Class::Warrior)
				return 1;
			else if (GetClass() == Class::Monk)
				return 0;

			break;
		case disc_fellstrike:
			if (GetClass() == Class::Beastlord)
				return 0;
			else if (GetClass() == Class::Warrior)
				return 2;
			else
				return 1;

			break;
		case disc_hundredfist:
			if (GetClass() == Class::Monk)
				return 1;
			else if (GetClass() == Class::Rogue)
				return 2;

			break;
		default:
			return 0;
		}
	}

	return 0;
}

bool PlayerBot::CastDiscipline(uint8 disc_id, uint8 level_to_use)
{
	uint8 current_level = GetLevel();

	if(level_to_use > current_level || level_to_use == 0)
		return false;

	// reuse_timer is in seconds, ability_timer is in milliseconds.
	int32 reuse_timer = 0, ability_timer = 0, string = 0;
	int16 spellid = 0;

	switch(disc_id)
	{
		case disc_aggressive:
			reuse_timer = 1620;
			ability_timer = 180000;
			spellid = 4498;
			string = StringID::DISCIPLINE_AGRESSIVE;

			break;
		case disc_precision:
			reuse_timer = 1800;
			ability_timer = 180000;
			spellid = 4501;
			string = StringID::DISCIPLINE_PRECISION;

			break;
		case disc_defensive:
			reuse_timer = 900;
			ability_timer = 180000;
			spellid = 4499;
			string = StringID::DISCIPLINE_DEFENSIVE;

			break;
		case disc_evasive:
			reuse_timer = 900;
			ability_timer = 180000;
			spellid = 4503;
			string = StringID::DISCIPLINE_EVASIVE;

			break;
		case disc_ashenhand:
			reuse_timer = 4320;
			spellid = 4508;
			string = StringID::DISCIPLINE_ASHENHAND;

			break;
		case disc_furious: // furious (WAR), whirlwind (MNK), counterattack (ROG)
			reuse_timer = 3600;
			ability_timer = 9000;
			if(GetClass() == Class::Warrior)
				spellid = 4674;
			else if(GetClass() == Class::Monk)
				spellid = 4509;
			else if(GetClass() == Class::Rogue)
				spellid = 4673;
			string = StringID::DISCIPLINE_FURIOUS;

			break;
		case disc_stonestance: // stonestance (MNK), protectivespirit (BST)
			reuse_timer = 720;
			ability_timer = 12000;
			if(GetClass() == Class::Monk)
				spellid = 4510;
			else if(GetClass() == Class::Beastlord)
				spellid = 4671;
			string = StringID::DISCIPLINE_STONESTANCE;

			break;
		case disc_thunderkick:
			reuse_timer = 540;
			spellid = 4511;
			string = StringID::DISCIPLINE_THUNDERKICK;

			break;
		case disc_fortitude: // fortitude (WAR), voiddance (MNK)
			reuse_timer = 3600;
			ability_timer = 8000;
			if(GetClass() == Class::Warrior)
				spellid = 4670;
			else if(GetClass() == Class::Monk)
				spellid = 4502;
			string = StringID::DISCIPLINE_FORTITUDE;

			break;
		case disc_fellstrike: // fellstrike (WAR), bestialrage (BST), innerflame (MNK), duelist (ROG)
			reuse_timer = 1800;
			ability_timer = 12000;
			if(GetClass() == Class::Warrior)
				spellid = 4675;
			else if(GetClass() == Class::Monk)
				spellid = 4512;
			else if(GetClass() == Class::Rogue)
				spellid = 4676;
			else if(GetClass() == Class::Beastlord)
				spellid = 4678;
			string = StringID::DISCIPLINE_FELLSTRIKE;

			break;
		case disc_hundredfist: // hundredfist (MNK), blindingspeed (ROG)
			reuse_timer = 1800;
			ability_timer = 15000;
			if(GetClass() == Class::Monk)
				spellid = 4513;
			else if(GetClass() == Class::Rogue)
				spellid = 4677;
			string = StringID::DISCIPLINE_HUNDREDFIST;

			break;
		case disc_charge: // charge (WAR), deadeye (ROG)
			reuse_timer = 1800;
			ability_timer = 14000;
			if(GetClass() == Class::Warrior)
				spellid = 4672;
			else if(GetClass() == Class::Rogue)
				spellid = 4505;
			string = StringID::DISCIPLINE_CHARGE;

			break;
		case disc_mightystrike:
			reuse_timer = 3600;
			ability_timer = 10000;
			spellid = 4514;
			string = StringID::DISCIPLINE_MIGHTYSTRIKE;

			break;
		case disc_nimble:
			reuse_timer = 1800;
			ability_timer = 12000;
			spellid = 4515;
			string = StringID::DISCIPLINE_NIMBLE;

			break;
		case disc_silentfist:
			reuse_timer = 594;
			spellid = 4507;
			if(GetRace() == Race::Iksar)
				string = StringID::DISCIPLINE_SILENTFIST_IKSAR;
			else
				string = StringID::DISCIPLINE_SILENTFIST;

			break;
		case disc_kinesthetics:
			reuse_timer = 1800;
			ability_timer = 18000;
			spellid = 4517;
			string = StringID::DISCIPLINE_KINESTHETICS;

			break;
		case disc_holyforge:
			reuse_timer = 4320;
			ability_timer = 300000;
			spellid = 4500;
			string = StringID::DISCIPLINE_HOLYFORGE;

			break;
		case disc_sanctification:
			reuse_timer = 4320;
			ability_timer = 15000;
			spellid = 4518;
			string = StringID::DISCIPLINE_SANCTIFICATION;

			break;
		case disc_trueshot:
			reuse_timer = 4320;
			ability_timer = 120000;
			spellid = 4506;
			string = StringID::DISCIPLINE_TRUESHOT;

			break;
		case disc_weaponshield:
			reuse_timer = 4320;
			ability_timer = 20000;
			spellid = 4519;
			if(GetGender() == Gender::Male)
				string = StringID::DISCIPLINE_WPNSLD_MALE;
			else if(GetGender() == Gender::Female)
				string = StringID::DISCIPLINE_WPNSLD_FEMALE;
			else
				string = StringID::DISCIPLINE_WPNSLD_MONSTER;

			break;
		case disc_unholyaura:
			reuse_timer = 4320;
			spellid = 4520;
			string = StringID::DISCIPLINE_UNHOLYAURA;

			break;
		case disc_leechcurse:
			reuse_timer = 4320;
			ability_timer = 20000;
			spellid = 4504;
			string = StringID::DISCIPLINE_LEECHCURSE;

			break;
		case disc_deftdance:
			reuse_timer = 4320;
			ability_timer = 15000;
			spellid = 4516;
			string = StringID::DISCIPLINE_DEFTDANCE;

			break;
		case disc_puretone:
			reuse_timer = 4320;
			ability_timer = 240000;
			spellid = 4586;
			string = StringID::DISCIPLINE_PURETONE;

			break;
		case disc_resistant:
			reuse_timer = 3600;
			ability_timer = 300000;
			spellid = 4585;
			string = StringID::DISCIPLINE_RESISTANT;

			break;
		case disc_fearless:
			reuse_timer = 3600;
			ability_timer = 11000;
			spellid = 4587;
			string = StringID::DISCIPLINE_FEARLESS;

			break;
		default:
			LogDiscs("Invalid disc id [{}] was passed to CastDiscipline.", disc_id);
			return false;
	}

	if(string > 0 && IsValidSpell(spellid) && IsDisc(spellid))
	{
		entity_list.MessageClose_StringID(this, true, 50, Chat::Disciplines, string, GetName());

		if (reuse_timer < 1620 && current_level > 60)
			current_level = 60;
		int32 reuse_timer_mod = 0 - ((current_level - level_to_use) * 54);
		reuse_timer += reuse_timer_mod;
		if (reuse_timer > 4320)
			reuse_timer = 4320; // 72:00 maximum reuse time
		if (reuse_timer < 234)
			reuse_timer = 234; // 3:54 minimum reuse time

		if(!ClaimAbility("disc",GetDiscTimerID(disc_id),reuse_timer))return false;
		if(ability_timer > 0)
		{
			disc_expires_=Timer::GetTimeSeconds()+((ability_timer+999)/1000);
		}
		else
		{
			LogDiscs("Disc [{}] is an instant effect", disc_id);
		}

		active_disc_=disc_id; active_disc_spell_=spellid;
		if(!SpellFinished(spellid,this)||!FindBuff(spellid)){
            FadeDiscipline();database.QueryDatabase(fmt::format("DELETE FROM takp_bot_ability_recasts WHERE bot_id={} AND kind='disc' AND ability_id={}",bot_id_,GetDiscTimerID(disc_id)));return false;
        }
	}
	else
	{
		LogDiscs("Disc: [{}] Invalid stringid or spellid specified.", disc_id);
		return false;
	}

	return true;
}


void PlayerBot::ApplyAABonuses(uint32 aaid, uint32 slots, StatBonuses* newbon)
{
	if(slots == 0)	//sanity check. why bother if no slots to fill?
		return;

	//from AA_Ability struct
	uint32 effect = 0;
	int32 base1 = 0;
	int32 base2 = 0;	//only really used for SE_RaiseStatCap & SE_ReduceSkillTimer in aa_effects table
	uint32 slot = 0;

	std::map<uint32, std::map<uint32, AA_Ability> >::const_iterator find_iter = aa_effects.find(aaid);
	if(find_iter == aa_effects.end())
	{
		return;
	}

	for (std::map<uint32, AA_Ability>::const_iterator iter = aa_effects[aaid].begin(); iter != aa_effects[aaid].end(); ++iter) {
		effect = iter->second.skill_id;
		base1 = iter->second.base1;
		base2 = iter->second.base2;
		slot = iter->second.slot;

		//we default to 0 (SE_CurrentHP) for the effect, so if there aren't any base1/2 values, we'll just skip it
		if (effect == 0 && base1 == 0 && base2 == 0)
			continue;

		//IsBlankSpellEffect()
		if (effect == SE_Blank || (effect == SE_CHA && base1 == 0) || effect == SE_StackingCommand_Block || effect == SE_StackingCommand_Overwrite)
			continue;

		LogAA("Applying Effect [{}] from AA [{}] in slot [{}] (base1: [{}], base2: [{}]) on [{}] EQMacID: [{}]",
			effect,
			aaid,
			slot,
			base1,
			base2,
			this->GetCleanName(),
			zone->EmuToEQMacAA(aaid)
		);

		uint8 focus = IsFocusEffect(0, 0, true,effect);
		if (focus)
		{
			newbon->FocusEffects[focus] = effect;
			continue;
		}

		switch (effect)
		{
			//Note: AA effects that use accuracy are skill limited, while spell effect is not.
			case SE_Accuracy:
				if ((base2 == -1) && (newbon->Accuracy[EQ::skills::HIGHEST_SKILL+1] < base1))
					newbon->Accuracy[EQ::skills::HIGHEST_SKILL+1] = base1;
				else if (newbon->Accuracy[base2] < base1)
					newbon->Accuracy[base2] += base1;
				break;
			case SE_CurrentHP: //regens
				newbon->HPRegen += base1;
				break;
			case SE_MovementSpeed:
				newbon->movementspeed += base1;	//should we let these stack?
				/*if (base1 > newbon->movementspeed)	//or should we use a total value?
					newbon->movementspeed = base1;*/
				break;
			case SE_STR:
				newbon->STR += base1;
				break;
			case SE_DEX:
				newbon->DEX += base1;
				break;
			case SE_AGI:
				newbon->AGI += base1;
				break;
			case SE_STA:
				newbon->STA += base1;
				break;
			case SE_INT:
				newbon->INT += base1;
				break;
			case SE_WIS:
				newbon->WIS += base1;
				break;
			case SE_CHA:
				newbon->CHA += base1;
				break;
			case SE_CurrentMana:
				newbon->ManaRegen += base1;
				break;
			case SE_ResistFire:
				newbon->FR += base1;
				break;
			case SE_ResistCold:
				newbon->CR += base1;
				break;
			case SE_ResistPoison:
				newbon->PR += base1;
				break;
			case SE_ResistDisease:
				newbon->DR += base1;
				break;
			case SE_ResistMagic:
				newbon->MR += base1;
				break;
			case SE_IncreaseSpellHaste:
				break;
			case SE_IncreaseRange:
				break;
			case SE_MaxHPChange: // Natural Durability, Physical Enhancement and Planar Durability have harcoded logic and don't use this bonus, see PlayerBot::CalcMaxHP()
				newbon->MaxHP += base1;
				break;
			case SE_TwoHandBash:
				break;
			case SE_WaterBreathing:
				//handled by client
				newbon->WaterBreathing = true;
				break;
			case SE_SetBreathLevel:
				newbon->BreathLevel += base1;
				break;
			case SE_RaiseStatCap:
				switch(base2)
				{
					//are these #define'd somewhere?
					case 0: //str
						newbon->STRCapMod += base1;
						break;
					case 1: //sta
						newbon->STACapMod += base1;
						break;
					case 2: //agi
						newbon->AGICapMod += base1;
						break;
					case 3: //dex
						newbon->DEXCapMod += base1;
						break;
					case 4: //wis
						newbon->WISCapMod += base1;
						break;
					case 5: //int
						newbon->INTCapMod += base1;
						break;
					case 6: //cha
						newbon->CHACapMod += base1;
						break;
					case 7: //mr
						newbon->MRCapMod += base1;
						break;
					case 8: //cr
						newbon->CRCapMod += base1;
						break;
					case 9: //fr
						newbon->FRCapMod += base1;
						break;
					case 10: //pr
						newbon->PRCapMod += base1;
						break;
					case 11: //dr
						newbon->DRCapMod += base1;
						break;
				}
				break;
			case SE_PetDiscipline2:
				break;
			case SE_TotalHP:
				newbon->HP += base1;
				break;
			case SE_StunResist:
				newbon->StunResist += base1;
				break;
			case SE_SpellCritChance:
				newbon->CriticalSpellChance += base1;
				break;
			case SE_ResistSpellChance:
				newbon->ResistSpellChance += base1;
				break;
			case SE_CriticalHealChance:
				newbon->CriticalHealChance += base1;
				break;
			case SE_CriticalDoTChance:
				newbon->CriticalDoTChance += base1;
				break;
			case SE_ReduceSkillTimer:
				newbon->SkillReuseTime[base2] += base1;
				break;
			case SE_Fearless:
				newbon->Fearless = true;
				break;
			case SE_ImprovedBindWound:
				newbon->BindWound += base1;
				break;
			case SE_MaxBindWound:
				newbon->MaxBindWound += base1;
				break;
			case SE_ExtraAttackChance:
				newbon->ExtraAttackChance += base1;
				break;
			case SE_SeeInvis:
				newbon->SeeInvis = base1;
				break;
			case SE_BaseMovementSpeed:
				newbon->BaseMovementSpeed += base1;
				break;
			case SE_IncreaseRunSpeedCap:
				newbon->IncreaseRunSpeedCap += base1;
				break;
			case SE_ConsumeProjectile:
				newbon->ConsumeProjectile += base1;
				break;
			case SE_ArcheryDamageModifier:
				newbon->ArcheryDamageModifier += base1;
				break;
			case SE_DamageShield:
				newbon->DamageShield += base1;
				break;
			case SE_CharmBreakChance:
				newbon->CharmBreakChance += base1;
				break;
			case SE_OffhandRiposteFail:
				newbon->OffhandRiposteFail += base1;
				break;
			case SE_GivePetGroupTarget:
				newbon->GivePetGroupTarget = true;
				break;
			case SE_Ambidexterity:
				newbon->Ambidexterity += base1;
				break;
			case SE_AvoidMeleeChance:
				newbon->CombatAgility += base1;
				break;
			case SE_CombatStability:
				newbon->CombatStability += base1;
				break;
			case SE_AddSingingMod:
				switch (base2)
				{
					case EQ::item::ItemTypeWindInstrument:
						newbon->windMod += base1;
						break;
					case EQ::item::ItemTypeStringedInstrument:
						newbon->stringedMod += base1;
						break;
					case EQ::item::ItemTypeBrassInstrument:
						newbon->brassMod += base1;
						break;
					case EQ::item::ItemTypePercussionInstrument:
						newbon->percussionMod += base1;
						break;
					case 50:
						newbon->singingMod += base1;
						break;
				}
				break;
			case SE_SongModCap:
				newbon->songModCap += base1;
				break;
			case SE_SecondaryDmgInc:
				newbon->SecondaryDmgInc = true;
				break;
			case SE_ChangeAggro:
				newbon->hatemod += base1;
				break;
			case SE_ChannelChanceSpells:
				newbon->ChannelChanceSpells += base1;
				break;
			case SE_DoubleSpecialAttack:
				newbon->DoubleSpecialAttack += base1;
				break;
			case SE_FrontalBackstabMinDmg:
				newbon->FrontalBackstabMinDmg = true;
				break;

			case SE_StrikeThrough:
			case SE_StrikeThrough2:
				newbon->StrikeThrough += base1;
				break;
			case SE_DoubleAttackChance:
				newbon->DoubleAttackChance += base1;
				break;
			case SE_GiveDoubleAttack:
				newbon->GiveDoubleAttack += base1;
				break;
			case SE_RiposteChance:
				newbon->RiposteChance += base1;
				break;
			case SE_Flurry:
				newbon->FlurryChance += base1;
				break;
			case SE_PetFlurry:
				newbon->PetFlurry += base1;
				break;
			case SE_BardSongRange:
				newbon->SongRange += base1;
				break;
			case SE_RootBreakChance:
				newbon->RootBreakChance += base1;
				break;
			case SE_UnfailingDivinity:
				newbon->UnfailingDivinity += base1;
				break;
			case SE_CrippBlowChance:
				newbon->CrippBlowChance += base1;
				break;

			case SE_HitChance:
			{
				if(base2 == -1)
					newbon->HitChanceEffect[EQ::skills::HIGHEST_SKILL+1] += base1;
				else
					newbon->HitChanceEffect[base2] += base1;

				break;
			}

			case SE_CriticalHitChance:
			{
				if (newbon->CriticalHitChance < base1)
					newbon->CriticalHitChance = base1;

				break;
			}

			case SE_CriticalSpellChance:
			{
				newbon->CriticalSpellChance += base1;
				break;
			}

			case SE_ResistFearChance:
			{
				if(base1 == 100) // If we reach 100% in a single spell/item then we should be immune to negative fear resist effects until our immunity is over
					newbon->Fearless = true;

				newbon->ResistFearChance += base1; // these should stack
				break;
			}

			case SE_DamageModifier:
			{
				if(base2 == -1)
					newbon->DamageModifier[EQ::skills::HIGHEST_SKILL+1] += base1;
				else
					newbon->DamageModifier[base2] += base1;
				break;
			}

			case SE_SlayUndead:
			{
				if(newbon->SlayUndead[1] < base1)
					newbon->SlayUndead[0] = base1; // Rate
					newbon->SlayUndead[1] = base2; // Damage Modifier
				break;
			}

			case SE_DoubleRiposte:
			{
				newbon->DoubleRiposte += base1;
				break;
			}

			case SE_GiveDoubleRiposte:
			{
				//0=Regular Riposte 1=Skill Attack Riposte 2=Skill
				if(base2 == 0){
					if(newbon->GiveDoubleRiposte[0] < base1)
						newbon->GiveDoubleRiposte[0] = base1;
				}
				//Only for special attacks.
				else if(base2 > 0 && (newbon->GiveDoubleRiposte[1] < base1)){
					newbon->GiveDoubleRiposte[1] = base1;
					newbon->GiveDoubleRiposte[2] = base2;
				}

				break;
			}

			//Kayen: Not sure best way to implement this yet.
			//Physically raises skill cap ie if 55/55 it will raise to 55/60
			case SE_RaiseSkillCap:
			{
				if(newbon->RaiseSkillCap[0] < base1){
					newbon->RaiseSkillCap[0] = base1; //value
					newbon->RaiseSkillCap[1] = base2; //skill
				}
				break;
			}

			case SE_MasteryofPast:
			{
				if(newbon->MasteryofPast < base1)
					newbon->MasteryofPast = base1;
				break;
			}

			case SE_CastingLevel2:	// Jam Fest
			{
				newbon->effective_casting_level += base1;
				break;
			}
			case SE_CastingLevel:	// Brilliance of Ro, flappies
			{
				newbon->effective_casting_level_for_fizzle_check += base1;
				break;
			}

			case SE_DivineSave:
			{
				if(newbon->DivineSaveChance[0] < base1)
				{
					newbon->DivineSaveChance[0] = base1;
					newbon->DivineSaveChance[1] = base2;
				}
				break;
			}

			case SE_MitigateDamageShield:
			{
				if (base1 < 0)
					base1 = base1*(-1);

				newbon->DSMitigationOffHand += base1;
				break;
			}

			case SE_FinishingBlow:
			{
				//base1 = chance, base2 = damage
				if (newbon->FinishingBlow[1] < base2){
					newbon->FinishingBlow[0] = base1;
					newbon->FinishingBlow[1] = base2;
				}
				break;
			}

			case SE_FinishingBlowLvl:
			{
				//base1 = level, base2 = ??? (Set to 200 in AA data, possible proc rate mod?)
				if (newbon->FinishingBlowLvl[0] < base1){
					newbon->FinishingBlowLvl[0] = base1;
					newbon->FinishingBlowLvl[1] = base2;
				}
				break;
			}

			case SE_IncreaseChanceMemwipe:
				newbon->IncreaseChanceMemwipe += base1;
				break;

			case SE_CriticalMend:
				newbon->CriticalMend += base1;
				break;

			case SE_HealRate:
				newbon->HealRate += base1;
				break;

			case SE_MeleeLifetap:
			{

				if((base1 < 0) && (newbon->MeleeLifetap > base1))
					newbon->MeleeLifetap = base1;

				else if(newbon->MeleeLifetap < base1)
					newbon->MeleeLifetap = base1;
				break;
			}

			case SE_Vampirism:
				newbon->Vampirism += base1;
				break;

			case SE_Berserk:
				newbon->BerserkSPA = true;
				break;

			case SE_Metabolism:
				newbon->Metabolism += base1;
				break;

			case SE_ImprovedReclaimEnergy:
			{
				if((base1 < 0) && (newbon->ImprovedReclaimEnergy > base1))
					newbon->ImprovedReclaimEnergy = base1;

				else if(newbon->ImprovedReclaimEnergy < base1)
					newbon->ImprovedReclaimEnergy = base1;
				break;
			}

			case SE_HeadShot:
			{
				if(newbon->HeadShot[1] < base2){
					newbon->HeadShot[0] = base1;
					newbon->HeadShot[1] = base2;
				}
				break;
			}

			case SE_HeadShotLevel:
			{
				if(newbon->HSLevel < base1)
					newbon->HSLevel = base1;
				break;
			}

			case SE_IllusionPersistence:
				newbon->IllusionPersistence = true;
				break;

			case SE_MeleeMitigation:
				newbon->MeleeMitigationEffect += base1;
				break;

		}
	}
}
void PlayerBot::LoadAAs()
{
 aa_ranks_.clear();if(GetLevel()<51)return;
 // Automatic AAs require explicit operator opt-in, never just character level.
 auto enabled=database.QueryDatabase(fmt::format("SELECT value FROM takp_bot_options WHERE bot_id={} AND setting='autoaas'",bot_id_));
 if(!enabled.Success()||enabled.RowCount()!=1||atoi(enabled.begin()[0])!=1)return;
 auto rows=database.QueryDatabase(fmt::format("SELECT skill_id,max_level,class_type,level_inc FROM altadv_vars WHERE (classes & {})<>0 AND class_type BETWEEN 2 AND {} AND aa_expansion<=4 AND account_time_required=0 AND special_category NOT IN(1,2,8) ORDER BY skill_id",1u<<GetClass(),GetLevel()));
 if(!rows.Success())return;
 for(auto r=rows.begin();r!=rows.end();++r){uint32 id=atoi(r[0]);if(!id||id>=aaHighestID)continue;int ranks=std::min(atoi(r[1]),MAX_AA_ACTION_RANKS);if(atoi(r[3])>0)ranks=std::min(ranks,1+(GetLevel()-atoi(r[2]))/atoi(r[3]));if(ranks>0)aa_ranks_[id]=ranks;}
}

void PlayerBot::CalcBonuses()
{
 NPC::CalcBonuses();memset(&aabonuses,0,sizeof(aabonuses));
 for(auto &a:aa_ranks_)ApplyAABonuses(a.first+a.second-1,zone->GetTotalAALevels(a.first+a.second-1),&aabonuses);
 itembonuses.HPRegen=std::min<int>(itembonuses.HPRegen,GetLevel()>60?GetLevel()-30:30);
 itembonuses.ManaRegen=std::min<int>(itembonuses.ManaRegen,RuleI(Character,ItemManaRegenCap));
 Mob::CalcBonuses();
}

bool PlayerBot::ClaimAbility(const std::string &kind,uint32 id,uint32 seconds)
{
 if(!database.QueryDatabase(fmt::format("INSERT IGNORE INTO takp_bot_ability_recasts VALUES({},'{}',{},'2000-01-01')",bot_id_,kind,id)).Success())return false;
 auto r=database.QueryDatabase(fmt::format("UPDATE takp_bot_ability_recasts SET ready_at=TIMESTAMPADD(SECOND,{},CURRENT_TIMESTAMP) WHERE bot_id={} AND kind='{}' AND ability_id={} AND ready_at<=CURRENT_TIMESTAMP",std::max<uint32>(1,seconds),bot_id_,kind,id));
 return r.Success()&&r.RowsAffected()==1;
}

bool PlayerBot::UseDiscipline(uint8 id)
{
 if(HasDied()||IsStunned()||IsFeared()||IsMezzed()||IsAmnesiad()||options_["suspend"]||IsCasting())return false;
 if(active_disc_&&disc_expires_&&Timer::GetTimeSeconds()<disc_expires_)return false;
 auto required=DisciplineUseLevel(id);if(!required||required>GetLevel())return false;
 if(active_disc_)FadeDiscipline();return CastDiscipline(id,required);
}

void PlayerBot::ListAbilities()
{
 if(!Owner())return;
 for(int id=1;id<256;++id)if(DisciplineUseLevel(id)&&DisciplineUseLevel(id)<=GetLevel())Owner()->Message(Chat::White,"Discipline %d (level %u)",id,DisciplineUseLevel(id));
 for(auto &a:aa_ranks_){auto *info=zone->FindAA(a.first,true);if(info)Owner()->Message(Chat::White,"AA %u: %s, rank %u",a.first,info->name,a.second);}
}

bool PlayerBot::ActivateAA(uint32 id,Mob *target)
{
 const auto rank=GetAA(id);if(!rank||id>=aaHighestID||HasDied()||IsCasting()||IsStunned()||IsMezzed()||IsFeared()||IsSilenced()||options_["suspend"])return false;
 const auto &a=AA_Actions[id][std::min<uint32>(rank,MAX_AA_ACTION_RANKS)-1];uint16 spell=a.spell_id;
 if(id==aaDireCharm)spell=GetClass()==Class::Druid?2760:GetClass()==Class::Necromancer?2759:2761;
 if(id==aaImprovedFamiliar&&GetAA(aaAllegiantFamiliar))spell=AA_Actions[aaAllegiantFamiliar][0].spell_id;
 if(id==aaCelestialRegeneration&&GetAA(aaCelestialRenewal))spell=AA_Actions[aaCelestialRenewal][GetAA(aaCelestialRenewal)-1].spell_id;
 if(a.target==aaTargetUser||a.target==aaTargetGroup)target=this;else if(a.target==aaTargetPet)target=GetPet();
 if(!target||a.mana_cost>GetMana())return false;
 const bool has_spell=IsValidSpell(spell);
 if(has_spell){if(IsBeneficialSpell(spell)?!Friendly(target):!Allowed(target))return false;if(spells[spell].mana>GetMana()||!CheckLosFN(target)||!Components(spell,false))return false;if(target!=this&&DistanceSquared(GetPosition(),target->GetPosition())>spells[spell].range*spells[spell].range)return false;}
 else if(a.action==aaActionNone)return false;
 if(a.action==aaActionActOfValor||a.action==aaActionPurgePoison){if(!Friendly(target)||DistanceSquared(GetPosition(),target->GetPosition())>10000)return false;}
 // Unsupported data-only action codes are never charged as successful abilities.
 if(a.action!=aaActionNone&&a.action!=aaActionAETaunt&&a.action!=aaActionRampage&&a.action!=aaActionActOfValor&&a.action!=aaActionEscape&&a.action!=aaActionFadingMemories&&a.action!=aaActionPurgePoison&&a.action!=aaActionFrenziedBurnout&&a.action!=aaActionMassBuff&&a.action!=aaActionWarcry&&a.action!=aaActionProjectIllusion)return false;
 int reduction=GetAA(a.redux_aa)*a.redux_rate+GetAA(a.redux_aa2)*a.redux_rate2;
 auto *info=zone->FindAA(id,true);uint32 timer=info&&info->spell_type?info->spell_type:id;
 if(!ClaimAbility("aa",timer,a.reuse_time*(100-std::min(100,std::max(0,reduction)))/100))return false;
 if(has_spell&&!CastSpell(spell,target->GetID())){database.QueryDatabase(fmt::format("DELETE FROM takp_bot_ability_recasts WHERE bot_id={} AND kind='aa' AND ability_id={}",bot_id_,timer));return false;}
 SetMana(GetMana()-a.mana_cost);
 switch(a.action){
 case aaActionAETaunt:for(auto &e:entity_list.GetNPCList())if(Allowed(e.second)&&DistanceSquared(GetPosition(),e.second->GetPosition())<=625)Taunt(e.second,true,100);break;
 case aaActionRampage:entity_list.AEAttack(this);break;
 case aaActionEscape:case aaActionFadingMemories:entity_list.RemoveFromHateLists(this);Passive();SetInvisible(INVIS_NORMAL,false);break;
 case aaActionActOfValor:target->HealDamage(GetHP(),this);Death(this,0,SPELL_UNKNOWN,EQ::skills::SkillHandtoHand);break;
 case aaActionPurgePoison:for(int n=0;n<target->GetMaxTotalSlots();++n)if(IsValidSpell(target->GetBuffs()[n].spellid)&&IsEffectInSpell(target->GetBuffs()[n].spellid,SE_PoisonCounter))target->BuffFadeBySlot(n);break;
 case aaActionMassBuff:mass_buff_until_=Timer::GetTimeSeconds()+std::max<int>(60,a.duration);break;
 case aaActionProjectIllusion:project_illusion_until_=Timer::GetTimeSeconds()+std::max<int>(60,a.duration);break;
 case aaActionWarcry:for(auto *m:Friends())if(m&&DistanceSquared(GetPosition(),m->GetPosition())<=10000){if(m->IsClient())m->CastToClient()->EnableAAEffect(aaEffectWarcry,rank*10);else if(auto *b=dynamic_cast<PlayerBot*>(m))b->warcry_until_=Timer::GetTimeSeconds()+rank*10;}break;
 default:break;
 }
 return true;
}

int16 PlayerBot::CalcAAFocus(focusType type, uint32 aa_ID, uint16 spell_id)
{
	const SPDat_Spell_Struct &spell = spells[spell_id];

	int16 value = 0;
	int lvlModifier = 100;
	int spell_level = 0;
	int lvldiff = 0;
	uint32 effect = 0;
	int32 base1 = 0;
	int32 base2 = 0;
	uint32 slot = 0;

	bool LimitFailure = false;
	bool LimitInclude[MaxLimitInclude] = { false };
	/* Certain limits require only one of several Include conditions to be true. Ie. Add damage to fire OR ice spells.
	0/1   SE_LimitResist
	2/3   SE_LimitSpell
	4/5   SE_LimitEffect
	6/7   SE_LimitTarget
	8/9   SE_LimitSpellGroup:
	10/11 SE_LimitCastingSkill:
	12/13 SE_LimitSpellClass:
	14/15 SE_LimitSpellSubClass:
	Remember: Update MaxLimitInclude in spdat.h if adding new limits that require Includes
	*/
	int FocusCount = 0;

	std::map<uint32, std::map<uint32, AA_Ability> >::const_iterator find_iter = aa_effects.find(aa_ID);
	if(find_iter == aa_effects.end())
	{
		return 0;
	}

	for (std::map<uint32, AA_Ability>::const_iterator iter = aa_effects[aa_ID].begin(); iter != aa_effects[aa_ID].end(); ++iter)
	{
		effect = iter->second.skill_id;
		base1 = iter->second.base1;
		base2 = iter->second.base2;
		slot = iter->second.slot;

		/*
		AA Foci's can contain multiple focus effects within the same AA.
		To handle this we will not automatically return zero if a limit is found.
		Instead if limit is found and multiple focus effects, we will reset the limit check
		when the next valid focus effect is found.
		*/

		if (IsFocusEffect(0, 0, true,effect)){
			FocusCount++;
			//If limit found on prior check next, else end loop.
			if (FocusCount > 1){

				for(int e = 0; e < MaxLimitInclude; e+=2) {
					if (LimitInclude[e] && !LimitInclude[e+1])
						LimitFailure = true;
				}

				if (LimitFailure){
					value = 0;
					LimitFailure = false;

					for(int e = 0; e < MaxLimitInclude; e++) {
						LimitInclude[e] = false; //Reset array
					}
				}

				else{
					break;
				}
			}
		}


		switch (effect)
		{
			case SE_Blank:
				break;

			//Handle Focus Limits

			case SE_LimitResist:
				if(base1 < 0){
					if(spell.resisttype == -base1) //Exclude
						LimitFailure = true;
				}
				else {
				LimitInclude[0] = true;
				if (spell.resisttype == base1) //Include
					LimitInclude[1] = true;
				}
				break;

			case SE_LimitInstant:
				if(base1 == 1 && spell.buffduration) //Fail if not instant
					LimitFailure = true;
				if(base1 == 0 && (spell.buffduration == 0)) //Fail if instant
					LimitFailure = true;

				break;

			case SE_LimitMaxLevel:
				spell_level = spell.classes[(GetClass()%16) - 1];
				lvldiff = spell_level - base1;
				//every level over cap reduces the effect by base2 percent unless from a clicky when ItemCastsUseFocus is true
				if(lvldiff > 0 && (spell_level <= RuleI(Character, MaxLevel) || RuleB(Character, ItemCastsUseFocus) == false))	{
					if(base2 > 0){
						lvlModifier -= base2*lvldiff;
						if(lvlModifier < 1)
							LimitFailure = true;
					}
					else
						LimitFailure = true;
				}
				break;

			case SE_LimitMinLevel:
				if((spell.classes[(GetClass()%16) - 1]) < base1)
					LimitFailure = true;
				break;

			case SE_LimitCastTimeMin:
				if (static_cast<int32>(spell.cast_time) < base1)
					LimitFailure = true;
				break;

			case SE_LimitCastTimeMax:
				if (static_cast<int32>(spell.cast_time) > base1)
					LimitFailure = true;
				break;

			case SE_LimitSpell:
				if(base1 < 0) {	//Exclude
					if (spell_id == -base1)
						LimitFailure = true;
				}
				else {
					LimitInclude[2] = true;
					if (spell_id == base1) //Include
						LimitInclude[3] = true;
				}
				break;

			case SE_LimitMinDur:
				if (base1 > CalcBuffDuration_formula(GetLevel(), spell.buffdurationformula, spell.buffduration))
					LimitFailure = true;

				break;

			case SE_LimitEffect:
				if(base1 < 0){
					if(IsEffectInSpell(spell_id,-base1)) //Exclude
						LimitFailure = true;
				}
				else{
					LimitInclude[4] = true;
					if(IsEffectInSpell(spell_id,base1)) //Include
						LimitInclude[5] = true;
				}
				break;

			case SE_LimitSpellType:
				switch(base1)
				{
					case 0:
						if (!IsDetrimentalSpell(spell_id) || IsNeutralSpell(spell_id))
							LimitFailure = true;
						break;
					case 1:
						if (!IsBeneficialSpell(spell_id) && !IsNeutralSpell(spell_id))
							LimitFailure = true;
						break;
				}
				break;

			case SE_LimitTarget:
				if (base1 < 0) {
					if (-base1 == spell.targettype) //Exclude
						LimitFailure = true;
				}
				else {
					LimitInclude[6] = true;
					if (base1 == spell.targettype) //Include
						LimitInclude[7] = true;
				}
				break;

			case SE_LimitCombatSkills:
				if (base1 == 0 && (IsCombatSkill(spell_id) || IsCombatProc(spell_id))) //Exclude Discs / Procs
					LimitFailure = true;
				else if (base1 == 1 && (!IsCombatSkill(spell_id) || !IsCombatProc(spell_id))) //Exclude Spells
					LimitFailure = true;

			break;

			case SE_LimitSpellGroup:
				if(base1 < 0) {
					if (-base1 == spell.spellgroup) //Exclude
						LimitFailure = true;
				}
				else {
					LimitInclude[8] = true;
					if (base1 == spell.spellgroup) //Include
						LimitInclude[9] = true;
				}
				break;

			//Handle Focus Effects
			case SE_ImprovedDamage:
				if (type == focusImprovedDamage && base1 > value)
					value = base1;
				break;

			case SE_ImprovedHeal:
				if (type == focusImprovedHeal && base1 > value)
					value = base1;
				break;

			case SE_ReduceManaCost:
				if (type == focusManaCost)
					value = base1;
				break;

			case SE_IncreaseSpellHaste:
				if (type == focusSpellHaste && base1 > value)
					value = base1;
				break;

			case SE_IncreaseSpellDuration:
				if (type == focusSpellDuration && base1 > value)
					value = base1;
				break;

			case SE_IncreaseRange:
				if (type == focusRange && base1 > value)
					value = base1;
				break;

			case SE_ReduceReagentCost:
				if (type == focusReagentCost && base1 > value)
					value = base1;
				break;

			case SE_SpellHateMod:
				if (type == focusSpellHateMod ) {
					if(value != 0) {
						if(value > 0){
							if(base1 > value)
								value = base1;
						}
						else{
							if(base1 < value)
								value = base1;
						}
					}
					else
						value = base1;
				}
				break;

			case SE_FcDamagePctCrit:	// only used for SK Soul Abrasion AA
				if(type == focusSpellDamageMult)
					value = base1;
				break;

		}
	}

	for(int e = 0; e < MaxLimitInclude; e+=2) {
		if (LimitInclude[e] && !LimitInclude[e+1])
			return 0;
	}

	if (LimitFailure)
		return 0;

	return(value*lvlModifier/100);
}

int32 PlayerBot::GetAACastingTimeModifier(uint16 spell_id, int32 casttime)
{
	// this function is based on a client decompile
	const struct SPDat_Spell_Struct *spell = &spells[spell_id];
	int32 modified_cast_time = casttime;

	// Spell Casting Deftness and Quick Buff
	if (spell->goodEffect != 0 && casttime > 3999 && spell->buffduration)
	{
		uint32 SpellCastingDeftness_AA_Level = GetAA(aaSpellCastingDeftness);
		if (SpellCastingDeftness_AA_Level > 0)
		{
			int32 percent_mod = 100;
			switch (SpellCastingDeftness_AA_Level)
			{
			case 1: percent_mod = 95; break; // Spell Casting Deftness 1 - 5% cast time reduction
			case 2: percent_mod = 85; break; // Spell Casting Deftness 2 - 15% cast time reduction
			case 3: percent_mod = 75; break; // Spell Casting Deftness 3 - 25% cast time reduction
			}
			modified_cast_time = modified_cast_time * percent_mod / 100;
		}

		uint32 QuickBuff_AA_Level = GetAA(aaQuickBuff);
		if (QuickBuff_AA_Level > 0)
		{
			int32 percent_mod = 100;
			switch (QuickBuff_AA_Level)
			{
			case 1: percent_mod = 90; break; // Quick Buff 1 - 10% cast time reduction
			case 2: percent_mod = 75; break; // Quick Buff 2 - 25% cast time reduction
			case 3: percent_mod = 50; break; // Quick Buff 3 - 50% cast time reduction
			}
			modified_cast_time = modified_cast_time * percent_mod / 100;
		}
	}

	// Quick Damage
	if (spell->goodEffect == 0 && casttime > 3999 && spell->buffduration == 0 && IsEffectInSpell(spell_id, SE_CurrentHP) && GetAA(aaSpellCastingFury) == 3)
	{
		uint32 QuickDamage_AA_Level = GetAA(aaQuickDamage);
		if (QuickDamage_AA_Level > 0)
		{
			int32 percent_mod = 100;
			switch (QuickDamage_AA_Level)
			{
			case 1: percent_mod = 98; break; // Quick Damage 1 - 2% cast time reduction
			case 2: percent_mod = 95; break; // Quick Damage 2 - 5% cast time reduction
			case 3: percent_mod = 90; break; // Quick Damage 3 - 10% cast time reduction
			}
			modified_cast_time = modified_cast_time * percent_mod / 100;
		}
	}

	// Quick Evacuation
	if (IsEffectInSpell(spell_id, SE_Succor))
	{
		uint32 QuickEvacuation_AA_Level = GetAA(aaQuickEvacuation);
		if (QuickEvacuation_AA_Level > 0)
		{
			int32 percent_mod = 100;
			switch (QuickEvacuation_AA_Level)
			{
			case 1: percent_mod = 90; break; // Quick Evacuation 1 - 10% cast time reduction
			case 2: percent_mod = 75; break; // Quick Evacuation 2 - 25% cast time reduction
			case 3: percent_mod = 50; break; // Quick Evacuation 3 - 50% cast time reduction
			}
			modified_cast_time = modified_cast_time * percent_mod / 100;
		}
	}

	// Quick Summoning
	if (GetClass() == Class::Magician)
	{
		if (IsEffectInSpell(spell_id, SE_SummonItem) || IsEffectInSpell(spell_id, SE_SummonPet) || spell_id == SPELL_MANIFEST_ELEMENTS || spell_id == SPELL_CALL_OF_THE_HERO)
		{
			uint32 QuickSummoning_AA_Level = GetAA(aaQuickSummoning);
			if (QuickSummoning_AA_Level > 0)
			{
				int32 percent_mod = 100;
				switch (QuickSummoning_AA_Level)
				{
				case 1: percent_mod = 90; break; // Quick Summoning 1 - 10% cast time reduction
				case 2: percent_mod = 75; break; // Quick Summoning 2 - 25% cast time reduction
				case 3: percent_mod = 50; break; // Quick Summoning 3 - 50% cast time reduction
				}
				modified_cast_time = modified_cast_time * percent_mod / 100;
			}
		}
	}

	return modified_cast_time - casttime;
}
