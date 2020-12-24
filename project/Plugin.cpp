#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "DecisionMaking/EBehaviourTree.h"
#include "DecisionMaking/Behaviours.h"

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
	pBlackboard->AddData("Target", Elite::Vector2{});
	pBlackboard->AddData("CurrentSlot", UINT{});

	m_pBehaviourTree = new BehaviorTree{ pBlackboard,
	//DO any of the following states
	new BehaviorSelector
	{
		{
			//OR DO CHECK fight/flight
			new BehaviorSequence
			{
				{
					//IF in danger ELSE continue
					new BehaviorConditional{ IsInDanger },
					//DO either FIGHT when (in danger)
					new BehaviorSelector
					{
						{
							//AND has weapon AND has ammo DO use item
							new BehaviorSequence
							{
								{
									new BehaviorConditional{ HasWeapon },
									new BehaviorConditional{ HasAmmo },
									//new BehaviorAction{ AimWeapon },
									new BehaviorAction{ UseItem },
								}
							},
							//OR DO flight WHEN (in danger)
							//AND has no weapon (OR has no ammo) DO flee
							new BehaviorAction{ ChangeToFlee },
							new BehaviorSelector
							{
								{
									//OR IF tired
									new BehaviorConditional{ IsTired },
									//OR IF NOT tired DO run
									new BehaviorAction{ EnableRunning },
								}
							}
						}
					},
				}
			},

			//(IF NOT in danger)
			new BehaviorSelector
			{
				{
					//OR IF hurt DO use medkit
					new BehaviorSequence
					{
						{
							new BehaviorSequence
							{
								{
									//IF
									new BehaviorSelector
										{
											{
												//hurt OR bitten
												new BehaviorConditional{ IsHurt },
												new BehaviorConditional{ IsBitten }
											}
										}
								}
							},
							//AND has medkit
							new BehaviorConditional{ HasMedkit },
							//DO use medkit
							new BehaviorAction{ UseItem },
						}
					},
					//OR IF hungry DO use food
					new BehaviorSequence
					{
						{
							new BehaviorConditional{ IsHungry },
							new BehaviorConditional{ HasFood },
							new BehaviorAction{ UseItem },
						}
					},
					//OR IF tired DO rest
					new BehaviorSequence
					{
						{
							new BehaviorConditional{ IsTired },
							new BehaviorAction{ ChangeToRest },
						}
					},
				}
			},

			//OR DO wander
			new BehaviorAction{ ChangeToWander }
		}
	}
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

	//CheckEntitiesInFOV();
	m_pBehaviourTree->Update(dt);
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	SteeringPlugin_Output steering;
	if (!m_pBehaviourTree->GetBlackboard()->GetData("Steering", steering))
		steering = SteeringPlugin_Output{};

	return steering;

	//Use the Interface (IAssignmentInterface) to 'interface' with the AI_Framework
	auto agentInfo = m_pInterface->Agent_GetInfo();

	auto nextTargetPos = m_Target; //To start you can use the mouse position as guidance

	auto vHousesInFOV = GetHousesInFOV();//uses m_pInterface->Fov_GetHouseByIndex(...)
	auto vEntitiesInFOV = GetEntitiesInFOV(); //uses m_pInterface->Fov_GetEntityByIndex(...)

	for (auto& e : vEntitiesInFOV)
	{
		if (e.Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo zoneInfo;
			m_pInterface->PurgeZone_GetInfo(e, zoneInfo);
			std::cout << "Purge Zone in FOV:" << e.Location.x << ", "<< e.Location.y <<  " ---EntityHash: " << e.EntityHash << "---Radius: "<< zoneInfo.Radius << std::endl;
		}
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

	//Simple Seek Behaviour (towards Target)
	steering.LinearVelocity = nextTargetPos - agentInfo.Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed

	if (Distance(nextTargetPos, agentInfo.Position) < 2.f)
	{
		steering.LinearVelocity = Elite::ZeroVector2;
	}

	//steering.AngularVelocity = m_AngSpeed; //Rotate your character to inspect the world while walking
	steering.AutoOrient = true; //Setting AutoOrientate to TRue overrides the AngularVelocity

	steering.RunMode = m_CanRun; //If RunMode is True > MaxLinSpd is increased for a limited time (till your stamina runs out)

								 //SteeringPlugin_Output is works the exact same way a SteeringBehaviour output

								 //@End (Demo Purposes)
	m_GrabItem = false; //Reset State
	m_UseItem = false;
	m_RemoveItem = false;

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