#include "stdafx.h"
#include "Behaviours.h"

#include <bitset>

//includes
#include "EBlackboard.h"
#include "EBehaviourTree.h"
#include <IExamInterface.h>
#include "Exam_HelperStructs.h"
#include <EliteMath/EMath.h>
#include "../Steering/SteeringBehaviors.h"

using namespace Elite;

//MOVEMENT STATES

BehaviorState ChangeToRest(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return Failure;

	if (!pBlackboard->ChangeData("Steering", SteeringPlugin_Output{}))
		return Failure;

	return Success;
}

BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return Failure;

	if (!pBlackboard->ChangeData("Steering", CalculateWanderSteering(pInterface->Agent_GetInfo())))
		return Failure;

	return Success;
}

BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return Failure;

	EntityInfo entityInfo;
	if (!pBlackboard->GetData("TargetEntity", entityInfo))
		return Failure;

	if (!pBlackboard->ChangeData("Steering", CalculateFleeSteering(pInterface->Agent_GetInfo(), entityInfo.Location)))
		return Failure;

	return Success;
}

BehaviorState ChangeToSeekCurrentTarget(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return Failure;

	EntityInfo entityInfo;
	if (!pBlackboard->GetData("TargetEntity", entityInfo))
		return Failure;

	if (!pBlackboard->ChangeData("Steering", CalculateSeekSteering(pInterface->Agent_GetInfo(), entityInfo.Location)))
		return Failure;

	return Success;
}

BehaviorState ChangeToExploreHouseInFOV(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return Failure;

	HouseInfo houseInfo;
	if (!pBlackboard->GetData("TargetHouse", houseInfo))
		return Failure;

	//TODO:	anywhere, change target FROM middle of house WHEN in radius of house center
	//		to NavGraph Closest Path point and continue following navgraph until item(s) found/zombies found
	//		if agent finds nothing in FOV and is about 5.f away from the house's middle, begin exiting

	if (!pBlackboard->ChangeData("Steering", CalculateSeekSteering(pInterface->Agent_GetInfo(), pInterface->NavMesh_GetClosestPathPoint(houseInfo.Center))))
		return Failure;

	return Success;
}

BehaviorState RotateTowardsTargetInFOV(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return Failure;

	EntityInfo entityInfo;
	if (!pBlackboard->GetData("TargetEntity", entityInfo))
		return Failure;

	float angleToTarget;
	if (!pBlackboard->ChangeData("Steering", CalculateFaceSteering(pInterface->Agent_GetInfo(), entityInfo.Location, &angleToTarget)))
		return Failure;

	if (!pBlackboard->ChangeData("AngleToTarget", angleToTarget))
		return Failure;

	return Success;
}

//ACTION STATES

BehaviorState UseItem(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return Failure;

	UINT currentSlot;
	if (!pBlackboard->GetData("CurrentSlot", currentSlot))
		return Failure;

	//supposed to be the currently found item in the inventory (pistol, medkit or food)
	if (!pInterface->Inventory_UseItem(currentSlot))
		return Failure;

	ItemInfo itemInfo;
	pInterface->Inventory_GetItem(currentSlot, itemInfo);

	switch (itemInfo.Type)
	{
	case eItemType::PISTOL:
		if (pInterface->Weapon_GetAmmo(itemInfo) <= 0)
			return RemoveItemFromInventory(currentSlot, pInterface, pBlackboard);
		break;
	case eItemType::MEDKIT:
		if (pInterface->Medkit_GetHealth(itemInfo) <= 0)
			return RemoveItemFromInventory(currentSlot, pInterface, pBlackboard);
		break;
	case eItemType::FOOD:
		if (pInterface->Food_GetEnergy(itemInfo) <= 0)
			return RemoveItemFromInventory(currentSlot, pInterface, pBlackboard);
		break;
	case eItemType::GARBAGE:
			return RemoveItemFromInventory(currentSlot, pInterface, pBlackboard);
		break;
	default:
		break;
	}

	return Success;
}

BehaviorState EnableRunning(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return Failure;

	SteeringPlugin_Output steering;
	if (!pBlackboard->GetData("Steering", steering))
		return Failure;

	steering.RunMode = true;
	return BehaviorState::Success;
}

BehaviorState PickUpItem(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return Failure;

	AgentInfo agentInfo = pInterface->Agent_GetInfo();

	std::vector<EntityInfo> entitiesInFOV{};
	if (!pBlackboard->GetData("EntitiesInFOV", entitiesInFOV))
		return Failure;

	for (const EntityInfo& entityInfo : entitiesInFOV)
	{
		if (entityInfo.Type == eEntityType::ITEM)
		{
			if (agentInfo.Position.Distance(entityInfo.Location) > agentInfo.GrabRange)
				continue;

			ItemInfo itemInfo;
			//Item_Grab > When DebugParams.AutoGrabClosestItem is TRUE, the Item_Grab function returns the closest item in range
			//Keep in mind that DebugParams are only used for debugging purposes, by default this flag is FALSE
			//Otherwise, use GetEntitiesInFOV() to retrieve a vector of all entities in the FOV (EntityInfo)
			//Item_Grab gives you the ItemInfo back, based on the passed EntityHash (retrieved by GetEntitiesInFOV)
			if (pInterface->Item_Grab(entityInfo, itemInfo))
			{
				UINT freeSlots;
				if (!pBlackboard->GetData("FreeSlots", freeSlots))
					return Failure;

				std::bitset<8> bits{ freeSlots };
				for (UINT i{}; i < pInterface->Inventory_GetCapacity(); ++i)
				{
					if (bits[i]) //if the slot is free
						continue;

					pInterface->Inventory_AddItem(i, itemInfo);
					bits[i] = 1; //set the slot to used
					pBlackboard->ChangeData("FreeSlots", (UINT)bits.to_ulong());
					//pBlackboard->ChangeData("CurrentSlot", i); //no need since this changes upon searching inventory
					if (itemInfo.Type == eItemType::GARBAGE)
						RemoveItemFromInventory(i, pInterface, pBlackboard); //remove garbage aa
					break;
				}
			}
		}
	}

	return Success;
}

bool HasWeaponInInventory(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return false;

	const UINT inventoryCapacity = pInterface->Inventory_GetCapacity();
	for (UINT i{}; i < inventoryCapacity; ++i)
	{
		ItemInfo itemInfo;
		if (pInterface->Inventory_GetItem(i, itemInfo))
		{
			//has atleast 1 pistol in inventory, regardless of ammo
			if (itemInfo.Type == eItemType::PISTOL)
			{
				pBlackboard->ChangeData("CurrentSlot", i); //store current slot idx
				return true;
			}
		}
	}

	//pBlackboard->ChangeData("Ammo", 0); //reset current ammo
	//not needed since HasAmmo will never fire before HasWeapon
	return false;
}

bool HasAmmoInCurrentWeapon(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return false;

	UINT currentSlot;
	if (!pBlackboard->GetData("CurrentSlot", currentSlot))
		return false;

	ItemInfo itemInfo;
	if (!pInterface->Inventory_GetItem(currentSlot, itemInfo))
		return false;

	return pInterface->Weapon_GetAmmo(itemInfo); //if ammo >= 1
}

bool IsAimingAtTarget(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return false;

	float angleToTarget;
	if (!pBlackboard->GetData("AngleToTarget", angleToTarget))
		return false;

	AgentInfo agentInfo = pInterface->Agent_GetInfo();

	EntityInfo entityInfo;
	if (!pBlackboard->GetData("TargetEntity", entityInfo))
		return false;

	//if the angle is smaller than the margin, we're somewhat aiming at the set target
	const Elite::Vector2 agentToTarget{ agentInfo.Position - entityInfo.Location };
	float scaledMargin{ 5.f }; //default angular difference margin in degrees
	const float scale{ (abs(agentToTarget.Magnitude()) / 15.f) }; //scaled by distance to target (max distance from FOV is 15.f)
	scaledMargin *= scale + scale * agentInfo.AgentSize;
	//return abs(angleToTarget) > Elite::ToRadians(scaledMargin);
	return abs(angleToTarget) < Elite::ToRadians(5.f);
}

bool HasFoodInInventory(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return false;

	const UINT inventoryCapacity = pInterface->Inventory_GetCapacity();
	for (UINT i{}; i < inventoryCapacity; ++i)
	{
		ItemInfo itemInfo;
		if (pInterface->Inventory_GetItem(i, itemInfo))
		{
			if (itemInfo.Type == eItemType::FOOD)
			{
				if (pInterface->Food_GetEnergy(itemInfo) <= 0) //food has already been fully consumed
				{
					pInterface->Inventory_RemoveItem(i);
					return false;
				}

				pBlackboard->ChangeData("CurrentSlot", i); //store current slot idx
				return true;
			}
		}
	}

	return false;
}

bool HasMedkitInInventory(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return false;

	const UINT inventoryCapacity = pInterface->Inventory_GetCapacity();
	for (UINT i{}; i < inventoryCapacity; ++i)
	{
		ItemInfo itemInfo;
		if (pInterface->Inventory_GetItem(i, itemInfo))
		{
			if (itemInfo.Type == eItemType::MEDKIT)
			{
				if (pInterface->Medkit_GetHealth(itemInfo) <= 0) //medkit has already been fully used
				{
					pInterface->Inventory_RemoveItem(i);
					return false;
				}

				pBlackboard->ChangeData("CurrentSlot", i); //store current slot idx
				return true;
			}
		}
	}

	return false;
}

bool IsHurt(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return false;

	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	//if health is below 75%, agent is hurt and may heal
	return agentInfo.Health < 7.5f;
}

bool IsHungry(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return false;

	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	return agentInfo.Energy < 7.5f;
}

bool IsTired(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return false;

	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	return agentInfo.Stamina <= 0.f;
}

bool IsBitten(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return false;

	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	return agentInfo.WasBitten;
	//Bitten is only set after death?
}

bool HasFoundEntityTargetInFOV(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return false;

	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	agentInfo.Position;

	//data is untouched in between frames
	std::vector<EntityInfo> entitiesInFOV{};
	pBlackboard->GetData("EntitiesInFOV", entitiesInFOV);
	for (const EntityInfo& entityInfo : entitiesInFOV)
	{
		pBlackboard->ChangeData("TargetEntity", entityInfo); //set target to entity of current interest
		return true;
	}

	return false;
}

bool HasFoundHouseTargetInFOV(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return false;

	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	agentInfo.Position;

	//data is untouched in between frames
	std::vector<HouseInfo> housesInFOV{};
	pBlackboard->GetData("HousesInFOV", housesInFOV);
	for (const HouseInfo& houseInfo : housesInFOV)
	{
		pBlackboard->ChangeData("TargetHouse", houseInfo); //set target to entity of current interest
		return true;
	}

	return false;
}

bool IsEnemyTargetInFOV(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return false;

	//data is untouched in between frames
	EntityInfo entityInfo;
	if (!pBlackboard->GetData("TargetEntity", entityInfo))
		return false;

	if (entityInfo.Type == eEntityType::ENEMY)
		return true;

	return false;
}

bool IsItemTargetInFOV(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return false;

	//data is untouched in between frames
	EntityInfo entityInfo;
	if (!pBlackboard->GetData("TargetEntity", entityInfo))
		return false;

	if (entityInfo.Type == eEntityType::ITEM)
		return true;

	return false;
}

//MISCELLANEOUS

//Redundant
void CleanInventory(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return;

	UINT freeSlots;
	if (!pBlackboard->GetData("FreeSlots", freeSlots))
		return;

	std::bitset<5> bits{ freeSlots };

	const UINT inventoryCapacity = pInterface->Inventory_GetCapacity();
	for (UINT i{}; i < inventoryCapacity; ++i)
	{
		ItemInfo itemInfo;
		if (pInterface->Inventory_GetItem(i, itemInfo))
		{
			if (itemInfo.Type == eItemType::MEDKIT)
			{
				if (pInterface->Medkit_GetHealth(itemInfo) <= 0) //medkit has already been fully used
				{
					bits[i] = 0;
					pInterface->Inventory_RemoveItem(i);
				}
			}
			else if (itemInfo.Type == eItemType::FOOD)
			{
				if (pInterface->Food_GetEnergy(itemInfo) <= 0) //food has already been fully consumed
				{
					bits[i] = 0;
					pInterface->Inventory_RemoveItem(i);
				}
			}
			else if (itemInfo.Type == eItemType::PISTOL)
			{
				if (pInterface->Weapon_GetAmmo(itemInfo) <= 0) //pistol's clip is empty
				{
					bits[i] = 0;
					pInterface->Inventory_RemoveItem(i);
				}
			}
			else if (itemInfo.Type == eItemType::GARBAGE)
			{
				bits[i] = 0;
				pInterface->Inventory_RemoveItem(i); //remove garbage wtf
			}
		}
	}

	pBlackboard->ChangeData("FreeSlots", freeSlots);
	//currentslot is only set and used when the inventory is checked
}

BehaviorState RemoveItemFromInventory(UINT slot, IExamInterface* pInterface, Elite::Blackboard* pBlackboard)
{
	UINT freeSlots;
	if (!pBlackboard->GetData("FreeSlots", freeSlots))
		return Failure;

	std::bitset<5> bits{ freeSlots };

	if (!pInterface->Inventory_RemoveItem(slot))
		return Failure;

	bits[slot] = 0;

	pBlackboard->ChangeData("FreeSlots", (UINT)bits.to_ulong());
	return Success;
}