#include "../client.h"
#include "../npc.h"
#include "../groups.h"
#include "../companion.h"
#include <algorithm>
#include <cstring>

extern volatile bool is_zone_loaded;

namespace {
// Phase one deliberately has no combat, loot, database records, or pet-slot ownership.
// Resolve both entity and character identity every tick; an entity ID can be reused.
class PrototypeCompanion final : public NPC {
public:
	PrototypeCompanion(NPCType *type, Client *owner)
		: NPC(type, nullptr, owner->GetPosition(), GravityBehavior::Water),
		  owner_entity_id_(owner->GetID()), owner_character_id_(owner->CharacterID())
	{
		GiveNPCTypeData();
		SetSpecialAbility(SpecialAbility::CharmImmunity, 1);
		SetSpecialAbility(SpecialAbility::AggroImmunity, 1);
		SetSpecialAbility(SpecialAbility::BeingAggroImmunity, 1);
		Follow();
	}

	~PrototypeCompanion() override
	{
		// Zone teardown deletes all groups after all mobs. Do not dereference
		// other group members while that bulk destruction is in progress.
		if (is_zone_loaded) LeaveCompanionGroup();
	}

	Group *GetGroup() override { return entity_list.GetGroupByMob(this); }
	bool HasGroup() override { return GetGroup() != nullptr; }

	void LeaveCompanionGroup()
	{
		if (auto *group = GetGroup()) group->DelMember(this);
		SetGrouped(false);
	}

	bool JoinOwnerGroup(Client *owner)
	{
		if (!BelongsTo(owner) || owner->HasRaid()) return false;
		auto *group = owner->GetGroup();
		if (GetGroup()) return GetGroup() == group;
		if (group && (!group->IsLeader(owner) || group->GroupCount() >= MAX_GROUP_MEMBERS)) return false;
		bool created = false;
		if (!group) {
			group = new Group(owner);
			entity_list.AddGroup(group);
			if (!group->GetID()) {
				// AddGroup does not register or delete the group when IDs run out.
				owner->SetGrouped(false);
				std::memset(owner->GetPP().groupMembers, 0, sizeof(owner->GetPP().groupMembers));
				delete group;
				return false;
			}
			created = true;
			owner->UpdateGroupID(group->GetID());
			database.SetGroupLeaderName(group->GetID(), owner->GetName());
			database.SetGroupOldLeaderName(group->GetID(), owner->GetName());
			auto *packet = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
			auto *join = reinterpret_cast<GroupJoin_Struct *>(packet->pBuffer);
			strn0cpy(join->membername, owner->GetName(), sizeof(join->membername));
			strn0cpy(join->yourname, owner->GetName(), sizeof(join->yourname));
			join->action = groupActInviteInitial;
			owner->QueuePacket(packet);
			delete packet;
		}
		if (!group->AddMember(this)) {
			if (created) group->DisbandGroup();
			return false;
		}
		group->SendHPPacketsTo(owner);
		return true;
	}

	bool BelongsTo(Client *client) const
	{
		return client && client->GetID() == owner_entity_id_ &&
			client->CharacterID() == owner_character_id_;
	}

	bool Process() override
	{
		auto *owner = entity_list.GetClientByID(owner_entity_id_);
		if (!BelongsTo(owner) || !owner->Connected() || owner->IsDead()) {
			LeaveCompanionGroup();
			Depop();
			return false;
		}
		if (auto *group = GetGroup()) {
			if (group != owner->GetGroup() || owner->HasRaid()) LeaveCompanionGroup();
			else if (group_hp_timer_.Check()) group->SendHPPacketsFrom(this);
		}
		const bool alive = NPC::Process();
		if (!alive) LeaveCompanionGroup();
		return alive;
	}

	bool Attack(Mob *, int = EQ::invslot::slotPrimary, int = 100) override
	{
		return false;
	}

	bool Death(Mob *, int32, uint16, EQ::skills::SkillType, uint8 = 0, bool = false) override
	{
		// A prototype must never award experience or create a lootable corpse.
		LeaveCompanionGroup();
		Depop();
		return true;
	}

	void Follow() { SetFollowID(owner_entity_id_); }
	void Stay()
	{
		SetFollowID(0);
		StopNavigation();
		SaveGuardSpot();
	}

private:
	uint16 owner_entity_id_;
	uint32 owner_character_id_;
	Timer group_hp_timer_{1000};
};
}

bool IsTransientCompanion(Mob *mob)
{
	return dynamic_cast<PrototypeCompanion *>(mob) != nullptr;
}

void command_companion(Client *c, const Seperator *sep)
{
	// Keep this restriction even if command_settings lowers the command's access level.
	if (c->Admin() < AccountStatus::GMImpossible) {
		c->Message(Chat::White, "Companions are restricted to development administrators.");
		return;
	}
	PrototypeCompanion *companion = nullptr;
	for (const auto &entry : entity_list.GetNPCList()) {
		auto *candidate = dynamic_cast<PrototypeCompanion *>(entry.second);
		if (candidate && candidate->BelongsTo(c)) {
			companion = candidate;
			break;
		}
	}
	const auto action = Strings::ToLower(sep->arg[1]);
	if (action == "spawn") {
		if (companion || !c->Connected() || c->IsDead()) {
			c->Message(Chat::White, "You must be alive with no active companion to spawn one.");
			return;
		}
		auto *type = new NPCType{};
		const auto name = std::string(c->GetName()) + "_Companion";
		strn0cpy(type->name, name.c_str(), sizeof(type->name));
		type->level = std::max<uint8>(1, c->GetLevel());
		type->race = 1;
		type->class_ = 1;
		type->bodytype = 1;
		type->cur_hp = type->max_hp = 100;
		type->runspeed = 1.3f;
		type->size = 6.0f;
		type->STR = type->STA = type->DEX = type->AGI = 75;
		type->INT = type->WIS = type->CHA = 75;
		type->attack_delay = 30;
		companion = new PrototypeCompanion(type, c);
		entity_list.AddNPC(companion);
		c->Message(Chat::White, "Noncombat test companion spawned. It disappears when you leave the zone.");
		return;
	}
	if (action != "follow" && action != "stay" && action != "dismiss" && action != "group" && action != "ungroup") {
		c->Message(Chat::White, "Usage: #companion spawn | follow | stay | group | ungroup | dismiss");
		return;
	}
	if (!companion) {
		c->Message(Chat::White, "You have no companion in this zone. Use #companion spawn.");
		return;
	}
	if (action == "follow") companion->Follow();
	else if (action == "stay") companion->Stay();
	else if (action == "group") {
		if (!companion->JoinOwnerGroup(c)) {
			c->Message(Chat::White, "Cannot join: you must lead a group with a free slot and cannot be in a raid.");
			return;
		}
	}
	else if (action == "ungroup") companion->LeaveCompanionGroup();
	else {
		companion->LeaveCompanionGroup();
		companion->Depop();
	}
	c->Message(Chat::White, "Companion: %s.", action.c_str());
}
