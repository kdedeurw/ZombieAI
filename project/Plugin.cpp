#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
//#include "DecisionMaking/EBlackboard.h"
#include "DecisionMaking/EBehaviourTree.h"
#include "DecisionMaking/Behaviours.h"

using namespace Elite;

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "DegenerateBot";
	info.Student_FirstName = "Kristof";
	info.Student_LastName = "Dedeurwaerder";
	info.Student_Class = "2DAE01";

	Blackboard* pBlackboard = new Blackboard{};
	pBlackboard->AddData("Interface", m_pInterface);
	pBlackboard->AddData("Steering", SteeringPlugin_Output{});
	pBlackboard->AddData("TargetEntity", EntityInfo{});
	pBlackboard->AddData("TargetHouse", HouseInfo{});
	pBlackboard->AddData("FreeSlots", UINT{}); //memory optimisation isn't possible since padding will happen and we're not storing the values
	pBlackboard->AddData("CurrentSlot", UINT{});
	pBlackboard->AddData("HousesInFOV", std::vector<HouseInfo>{});
	pBlackboard->AddData("EntitiesInFOV", std::vector<EntityInfo>{});
	pBlackboard->AddData("AngleToTarget", float{}); //in degrees

	Elite::BehaviorConditional* pIsAimingAtTarget = new BehaviorConditional{ IsAimingAtTarget };
	Elite::BehaviorAction* pUseItem = new BehaviorAction{ UseItem };

	m_pBehaviourTree = new BehaviorTree{ pBlackboard,
	new BehaviorSelector
	{{
		//CHOOSE OR > GLOBAL
		new BehaviorSelector
		{{
			//CHOOSE OR > IF found entity
			new BehaviorSequence
			{{
				//IF found entity
				new BehaviorConditional{ HasFoundEntityTargetInFOV },

				//CHOOSE OR > DO CHECK fight/flight pickup item
				new BehaviorSelector
				{{
					//CHOOSE OR > IF enemy
					new BehaviorSelector
					{{
						//IF entity is enemy
						new BehaviorSequence
						{{
							new BehaviorConditional{ IsEnemyTargetInFOV },
							//CHOOSE OR > check and shoot or rotate
							new BehaviorSelector
							{{
								//IF enemy visible AND has weapon AND has ammo AND if aiming DO shoot
								new BehaviorSequence
								{{
									//AND IF has weapon
									new BehaviorConditional{ HasWeaponInInventory },
									//AND IF has ammo
									new BehaviorConditional{ HasAmmoInCurrentWeapon },
									//DO rotate
									new BehaviorAction{ RotateTowardsTargetInFOV },
									//CHOOSE OR > IF aiming DO shoot
									new BehaviorSequence
									{{
										//IF aiming at target
										pIsAimingAtTarget,
										//DO shoot pistol
										pUseItem,
									}},
								}},
							}}
						}}
					}},

					//OR > DO flee (AND has no weapon (OR has no ammo)))
					//new BehaviorSequence
					//{{
					//	//DO flee
					//	new BehaviorAction{ ChangeToFlee },
					//	//AND IF tired OR DO run
					//	new BehaviorSelector
					//	{{
					//		//OR IF tired
					//		new BehaviorConditional{ IsTired },
					//		//OR (IF NOT tired) DO run
					//		new BehaviorAction{ EnableRunning },
					//	}}
					//}},

					//OR > IF found item DO seek item
					new BehaviorSequence
					{{
						//IF has found item
						new BehaviorConditional{ IsItemTargetInFOV },
						//DO seek item
						new BehaviorAction{ ChangeToSeekCurrentTarget },
						//AND DO pickup item
						new BehaviorAction{ PickUpItem },
					}}
				}}
			}}
		}},

		//OR > IF NO entities found
		new BehaviorSelector
		{{
			//CHOOSE OR > IF hurt OR bitten AND has medkit DO use medkit
			new BehaviorSequence
			{{
				//AND IF hurt OR bitten
				new BehaviorSequence
				{{
					//OR > IF hurt OR bitten
					new BehaviorSelector
					{{
						new BehaviorConditional{ IsHurt },
						new BehaviorConditional{ IsBitten }
					}}
				}},
				//AND has medkit
				new BehaviorConditional{ HasMedkitInInventory },
				//DO use medkit
				new BehaviorAction{ UseItem },
			}},
			//OR > IF hungry AND has food DO use food
			new BehaviorSequence
			{{
				new BehaviorConditional{ IsHungry },
				new BehaviorConditional{ HasFoodInInventory },
				new BehaviorAction{ UseItem },
			}},
			//OR > IF tired DO rest
			new BehaviorSequence
			{{
				new BehaviorConditional{ IsTired },
				new BehaviorAction{ ChangeToRest },
			}},
		}},

		//OR > IF house found DO explore
		//new BehaviorSequence
		//{{
		//	//IF found house
		//	new BehaviorConditional{ HasFoundHouseTargetInFOV },
		//	//AND 
		//	//new BehaviorConditional{  },
		//	//DO explore
		//	new BehaviorAction{ ChangeToExploreHouseInFOV },
		//}},

		//OR > DO seek (DEBUG)
		//new BehaviorAction{ ChangeToSeekCurrentTarget },

		//OR > DO wander
		//new BehaviorAction{ ChangeToWander }
	}}
	}; //end of BT initialization
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void Plugin::DllShutdown()
{
	//Called wheb the plugin gets unloaded
	SAFE_DELETE(m_pBehaviourTree);
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be usefull to inspect certain behaviours (Default = false)
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	
	//params.StartingDifficultyStage = 3;
	params.SpawnDebugPistol = true;
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		//Update target based on input
		Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Space))
	{
		m_CanRun = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Left))
	{
		m_AngSpeed -= Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Right))
	{
		m_AngSpeed += Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_G))
	{
		m_GrabItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_U))
	{
		m_UseItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_R))
	{
		m_RemoveItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyUp(Elite::eScancode_Space))
	{
		m_CanRun = false;
	}

	//INVENTORY USAGE DEMO
	//********************

	if (m_GrabItem)
	{
		ItemInfo item;
		//Item_Grab > When DebugParams.AutoGrabClosestItem is TRUE, the Item_Grab function returns the closest item in range
		//Keep in mind that DebugParams are only used for debugging purposes, by default this flag is FALSE
		//Otherwise, use GetEntitiesInFOV() to retrieve a vector of all entities in the FOV (EntityInfo)
		//Item_Grab gives you the ItemInfo back, based on the passed EntityHash (retrieved by GetEntitiesInFOV)
		if (m_pInterface->Item_Grab({}, item))
		{
			//Once grabbed, you can add it to a specific inventory slot
			//Slot must be empty
			m_pInterface->Inventory_AddItem(0, item);
		}
	}

	if (m_UseItem)
	{
		//Use an item (make sure there is an item at the given inventory slot)
		m_pInterface->Inventory_UseItem(0);
	}

	if (m_RemoveItem)
	{
		//Remove an item from a inventory slot
		m_pInterface->Inventory_RemoveItem(0);
	}

	m_GrabItem = false; //Reset State
	m_UseItem = false;
	m_RemoveItem = false;

	//CheckEntitiesInFOV();

	EntityInfo test{};
	test.Location = m_Target;
	m_pBehaviourTree->GetBlackboard()->ChangeData("TargetEntity", test);
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	//Regular update
	//--------------

	//Update blackboard (to be used in BT and steering)
	Elite::Blackboard* pBlackboard = m_pBehaviourTree->GetBlackboard();
	pBlackboard->ChangeData("HousesInFOV", GetHousesInFOV()); //uses m_pInterface->Fov_GetHouseByIndex(...)
	pBlackboard->ChangeData("EntitiesInFOV", GetEntitiesInFOV()); //uses m_pInterface->Fov_GetEntityByIndex(...)

	//Update BT (to set steering)
	m_pBehaviourTree->Update(dt);

	//Set final steering
	SteeringPlugin_Output steering; //autoOrient is default set to true
	if (!m_pBehaviourTree->GetBlackboard()->GetData("Steering", steering))
		steering = SteeringPlugin_Output{};

	return steering;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });

	//RenderEntitiesInFOV();
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}

void Plugin::CheckEntitiesInFOV()
{
	itemInfos.clear();
	enemyInfos.clear();
	purgeZoneInfos.clear();

	auto entities = GetEntitiesInFOV();
	for (const EntityInfo& entity : entities)
	{
		ItemInfo itemInfo;
		EnemyInfo enemyInfo;
		PurgeZoneInfo purgeZoneInfo;
		switch (entity.Type)
		{
		case eEntityType::ITEM:
			m_pInterface->Item_GetInfo(entity, itemInfo);
			itemInfos.push_back(itemInfo);
			break;
		case eEntityType::ENEMY:
			m_pInterface->Enemy_GetInfo(entity, enemyInfo);
			enemyInfos.push_back(enemyInfo);
			break;
		case eEntityType::PURGEZONE:
			m_pInterface->PurgeZone_GetInfo(entity, purgeZoneInfo);
			purgeZoneInfos.push_back(purgeZoneInfo);
			break;
		default:
			break;
		}
	}
}

void Plugin::RenderEntitiesInFOV() const
{
	for (const ItemInfo& itemInfo : itemInfos)
	{
		const char* item{};
		switch (itemInfo.Type)
		{
		case eItemType::PISTOL:
			item = "Pistol";
			break;
		case eItemType::MEDKIT:
			item = "Medkit";
			break;
		case eItemType::FOOD:
			item = "Food";
			break;
		case eItemType::GARBAGE:
			item = "Garbage";
			break;
		case eItemType::RANDOM_DROP:
			item = "RandomDrop";
			break;
		case eItemType::RANDOM_DROP_WITH_CHANCE:
			item = "RandomDropWChance";
			break;
		default:
			break;
		}
		m_pInterface->Draw_Circle(itemInfo.Location, 2.f, Elite::Vector3{ 0,0,1 });
		std::cout << item << " found at: " << itemInfo.Location.x << " , " << itemInfo.Location.y << '\n';
	}

	for (const EnemyInfo& enemyInfo : enemyInfos)
	{
		const char* enemy{};
		switch (enemyInfo.Type)
		{
		case eEnemyType::DEFAULT:
			enemy = "Default";
			break;
		case eEnemyType::ZOMBIE_NORMAL:
			enemy = "Normal";
			break;
		case eEnemyType::ZOMBIE_RUNNER:
			enemy = "Runner";
			break;
		case eEnemyType::ZOMBIE_HEAVY:
			enemy = "Heavy";
			break;
		case eEnemyType::RANDOM_ENEMY:
			enemy = "Random";
			break;
		default:
			break;
		}
		m_pInterface->Draw_Circle(enemyInfo.Location, enemyInfo.Size + 1.f, Elite::Vector3{ 1,0,0 });
		std::cout << enemy << " found at: " << enemyInfo.Location.x << " , " << enemyInfo.Location.y << '\n';
		std::cout << "Health: " << enemyInfo.Health << ", size: "  << enemyInfo.Size << '\n';
		m_pInterface->Draw_Segment(enemyInfo.Location, enemyInfo.Location + enemyInfo.LinearVelocity, Elite::Vector3{0,1,0});
	}

	for (const PurgeZoneInfo& purgeZoneInfo : purgeZoneInfos)
	{
		m_pInterface->Draw_Circle(purgeZoneInfo.Center, purgeZoneInfo.Radius + 1.f, Elite::Vector3{ 1,0,0 });
	}


}