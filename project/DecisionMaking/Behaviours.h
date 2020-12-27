#pragma once

class IExamInterface;
//struct ItemInfo;
struct HouseInfo;
namespace Elite
{
	enum BehaviorState;
	class Blackboard;
}

//MOVEMENT STATES

//Agent will stop moving and recover stamina
Elite::BehaviorState ChangeToRest(Elite::Blackboard* pBlackboard);
//Agent will start wandering randomly
Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard);
//Agent will flee from designated target
Elite::BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard);
//Agent will try to move towards set target
Elite::BehaviorState ChangeToSeekCurrentTarget(Elite::Blackboard* pBlackboard);
//Agent will try to explore target HouseInfo and set IsExplored to true when near its center
Elite::BehaviorState ChangeToExploreHouseInFOV(Elite::Blackboard* pBlackboard);

//ACTION STATES

//Agent will try to use current item in inventory
Elite::BehaviorState UseItem(Elite::Blackboard* pBlackboard);
//Agent will start running, added ontop of current steering
Elite::BehaviorState TryRunning(Elite::Blackboard* pBlackboard);
//Agent will try to pick up designated item
Elite::BehaviorState PickUpItem(Elite::Blackboard* pBlackboard);
//Agent will rotate itself towards set
Elite::BehaviorState RotateTowardsTargetInFOV(Elite::Blackboard* pBlackboard);
//Agent will randomly look around them to try and spot something (CCW)
Elite::BehaviorState LookAround(Elite::Blackboard* pBlackboard);
//Agent will look for house in FOV
Elite::BehaviorState TryFindDifferentHouseInFOV(Elite::Blackboard* pBlackboard);

//CONDITIONS

bool HasWeaponInInventory(Elite::Blackboard* pBlackboard);

bool HasAmmoInCurrentWeapon(Elite::Blackboard* pBlackboard);

bool IsAimingAtTarget(Elite::Blackboard* pBlackboard);

bool HasFoodInInventory(Elite::Blackboard* pBlackboard);

bool HasMedkitInInventory(Elite::Blackboard* pBlackboard);

bool IsHurt(Elite::Blackboard* pBlackboard);

bool IsHungry(Elite::Blackboard* pBlackboard);

bool IsTired(Elite::Blackboard* pBlackboard);

bool IsBitten(Elite::Blackboard* pBlackboard);
//Will check whether there is an entity in the FOV
bool HasFoundEntityTargetInFOV(Elite::Blackboard* pBlackboard);
//Will check whether there is a house in the FOV
bool HasFoundHouseTargetInFOV(Elite::Blackboard* pBlackboard);
//Will check whether a found entity is of type enemy and store it
bool IsEnemyTargetInFOV(Elite::Blackboard* pBlackboard);
//Will check whether a found entity is of type item and store it
bool IsItemTargetInFOV(Elite::Blackboard* pBlackboard);

bool IsInHouse(Elite::Blackboard* pBlackboard);

bool IsCurrentHouseExplored(Elite::Blackboard* pBlackboard);

bool IsCurrentHouseNotExplored(Elite::Blackboard* pBlackboard);

bool HasFoundEnemyRunner(Elite::Blackboard* pBlackboard);

//MISCELLANEOUS

//Redundant: cleans inventory and sets flags
void CleanInventory(Elite::Blackboard* pBlackboard);
//Only call when successfully added an item on current slot!
Elite::BehaviorState RemoveItemFromInventory(UINT slot, IExamInterface* pInterface, Elite::Blackboard* pBlackboard);

void UpdateRunning(Elite::Blackboard* pBlackboard);

bool IsDifferentHouse(const Elite::Vector2& formerCenter, const HouseInfo& houseInfo);