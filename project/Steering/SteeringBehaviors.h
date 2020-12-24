/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include <Exam_HelperStructs.h>
#include "../DecisionMaking/EBlackboard.h"
#include <EliteMath/EMath.h>
using namespace Elite;

///////////////////////////////////////
//SEEK
//****
SteeringPlugin_Output CalculateSeekSteering(const AgentInfo& agentInfo, const Elite::Vector2& target);

//////////////////////////
//WANDER
//******
SteeringPlugin_Output CalculateWanderSteering(const AgentInfo& agentInfo, const Elite::Vector2& target = {}, float offset = 6.f, float radius = 4.f, float maxJitterOffset = 1.f);
static Vector2 WanderTarget = {};

//////////////////////////
//FLEE
//******
SteeringPlugin_Output CalculateFleeSteering(const AgentInfo& agentInfo, const Elite::Vector2& target);

//////////////////////////
//ARRIVE
//******
SteeringPlugin_Output CalculateArriveSteering(const AgentInfo& agentInfo, const Elite::Vector2& target, float arrivalRadius = 5.f, float slowRadius = 10.f);

//////////////////////////
//FACE
//******
SteeringPlugin_Output CalculateFaceSteering(const AgentInfo& agentInfo, const Elite::Vector2& target);

//////////////////////////
//EVADE
//******
SteeringPlugin_Output CalculateEvadeSteering(const AgentInfo& agentInfo, const Elite::Vector2& target, const Elite::Vector2& targetLinVel, float safeDistance = 10.f);

//////////////////////////
//PURSUIT
//******
SteeringPlugin_Output CalculatePursuitSteering(const AgentInfo& agentInfo, const Elite::Vector2& target, const Elite::Vector2& targetLinVel);

//////////////////////////
//HIDE
//******
SteeringPlugin_Output CalculateHideSteering(const AgentInfo& agentInfo, const Elite::Vector2& target);

//////////////////////////
//AVOIDOBSTACLE
//******
SteeringPlugin_Output CalculateAvoidObstacleSteering(const AgentInfo& agentInfo, const Elite::Vector2& target);

//////////////////////////
//ALIGN
//******
SteeringPlugin_Output CalculateAlignSteering(const AgentInfo& agentInfo, const Elite::Vector2& target);

//////////////////////////
//FACEDARRIVE
//******
SteeringPlugin_Output CalculateFacedArriveSteering(const AgentInfo& agentInfo, const Elite::Vector2& target);

//////////////////////////
//SLOWCLAP
//******
SteeringPlugin_Output CalculateSlowClapSteering(const AgentInfo& agentInfo, const Elite::Vector2& target);