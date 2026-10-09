#pragma once

namespace PlayerBotTradeRecovery {
// After a lost COMMIT acknowledgement, SQL is authoritative. Returning the old
// in-memory offers through FinishTrade would duplicate items if COMMIT succeeded.
// Clear only those in-memory slots and let normal zone-entry recovery return
// whichever trade-slot items actually remain in character_inventory.
template <typename ClientType>
void DiscardStaleOffers(ClientType *owner, int first_slot, int last_slot)
{
 const auto copper = owner->trade->cp;
 const auto silver = owner->trade->sp;
 const auto gold = owner->trade->gp;
 const auto platinum = owner->trade->pp;
 for (int slot = first_slot; slot <= last_slot; ++slot)
  owner->DeleteItemInInventory(slot, 0, false, false);
 owner->trade->Reset();
 // Bot equipment transactions never transfer currency, so the offered coins
 // can be returned exactly once independently of the equipment COMMIT result.
 if (copper || silver || gold || platinum)
  owner->AddMoneyToPP(copper, silver, gold, platinum, false);
}
}
