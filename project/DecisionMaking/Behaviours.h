#pragma once
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

	Elite::Vector2 target;
	if (!pBlackboard->GetData("Target", target))
		return Failure;

	if (!pBlackboard->ChangeData("Steering", CalculateFleeSteering(pInterface->Agent_GetInfo(), target)))
		return Failure;

	return Success;
}

BehaviorState ChangeToExploreHouse(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return Failure;

	Elite::Vector2 target;
	if (!pBlackboard->GetData("Target", target))
		return Failure;

	//TODO: anywhere, change target FROM middle of house WHEN in radius of house center
	//		to NavGraph Closest Path point and continue following navgraph until item(s) found/zombies found

	if (!pBlackboard->ChangeData("Steering", CalculateSeekSteering(pInterface->Agent_GetInfo(), target)))
		return Failure;

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
}

//ACTION STATES

bool HasWeapon(Elite::Blackboard* pBlackboard)
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

bool HasAmmo(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return false;

	ItemInfo itemInfo;
	if (!pBlackboard->GetData("Item", itemInfo))
		return false;

	return pInterface->Weapon_GetAmmo(itemInfo); //if ammo >= 1
}

bool HasFood(Elite::Blackboard* pBlackboard)
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

bool HasMedkit(Elite::Blackboard* pBlackboard)
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

bool IsInDanger(Elite::Blackboard* pBlackboard)
{
	return false;
}

BehaviorState UseItem(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (!isDataAvailable || !pInterface)
		return Failure;

	UINT slot;
	if (!pBlackboard->GetData("CurrentSlot", slot))
		return Failure;

	//supposed to be the currently found item in the inventory (pistol, medkit or food)
	if (!pInterface->Inventory_UseItem(slot))
		return Failure;

	//if (!pInterface->Inventory_RemoveItem(slot))
	//	return Failure;

	return Success;
}