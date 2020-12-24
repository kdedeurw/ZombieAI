#pragma once
//includes
#include "EBlackboard.h"
#include "EBehaviourTree.h"
#include <IExamInterface.h>
#include "Exam_HelperStructs.h"

using namespace Elite;

bool HasWeapon(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (isDataAvailable || !pInterface)
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
				//pBlackboard->ChangeData("Weapon", itemInfo); //store current weapon
				int ammo = pInterface->Weapon_GetAmmo(itemInfo);
				pBlackboard->ChangeData("Ammo", ammo); //store current ammo
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

	int ammo;
	if (!pBlackboard->GetData("Ammo", ammo))
		return false;

	return ammo; //if ammo >= 1
}

BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (isDataAvailable || !pInterface)
		return Failure;

	AgentInfo agentInfo = pInterface->Agent_GetInfo();

	return Success;
}

BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (isDataAvailable || !pInterface)
		return Failure;

	return Success;
}

bool IsHurt(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (isDataAvailable || !pInterface)
		return Failure;

	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	//if health is below 75%, agent is hurt and may heal
	return agentInfo.Health < 7.5f;
}

bool IsHungry(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (isDataAvailable || !pInterface)
		return Failure;

	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	return agentInfo.Energy < 7.5f;
}

bool IsTired(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (isDataAvailable || !pInterface)
		return Failure;

	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	return agentInfo.Stamina <= 0.f;
}

bool IsBitten(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (isDataAvailable || !pInterface)
		return Failure;

	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	return agentInfo.Bitten;
}

BehaviorState ChangeToCheckVitals(Elite::Blackboard* pBlackboard)
{
	IExamInterface* pInterface{};
	bool isDataAvailable = pBlackboard->GetData("Interface", pInterface);

	if (isDataAvailable || !pInterface)
		return Failure;

	AgentInfo agentInfo = pInterface->Agent_GetInfo();

	agentInfo.Health;
	agentInfo.Bitten;
	agentInfo.WasBitten;
	agentInfo.Energy;
	agentInfo.Stamina;
	agentInfo.RunMode;

	return Success;
}