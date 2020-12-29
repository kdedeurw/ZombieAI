#pragma once
/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
struct AgentInfo;
struct SteeringPlugin_Output;
struct SteeringPlugin_OutputCustom;
namespace Elite
{
	struct Vector2;
}

//for blended steering purposes
struct CustomSteeringData
{
	float paramX{};
	float paramY{};
	float radius{};
	//float* pAngle{};
};

///////////////////////////////////////
//SEEK
//****
SteeringPlugin_OutputCustom CalculateSeekSteering(const AgentInfo& agentInfo, const Elite::Vector2& target);
SteeringPlugin_OutputCustom CalculateSeekSteeringData(const AgentInfo& agentInfo, const Elite::Vector2& target, const CustomSteeringData& data);

//////////////////////////
//WANDER
//******
SteeringPlugin_OutputCustom CalculateWanderSteering(const AgentInfo& agentInfo, const Elite::Vector2& target = {}, float offset = 6.f, float radius = 4.f, float maxJitterOffset = 1.f);
SteeringPlugin_OutputCustom CalculateWanderSteeringData(const AgentInfo& agentInfo, const Elite::Vector2& target, const CustomSteeringData& data);

//////////////////////////
//FLEE
//******
SteeringPlugin_OutputCustom CalculateFleeSteering(const AgentInfo& agentInfo, const Elite::Vector2& target, float turnSpeed = 0.f);

//////////////////////////
//ARRIVE
//******
SteeringPlugin_OutputCustom CalculateArriveSteering(const AgentInfo& agentInfo, const Elite::Vector2& target, float arrivalRadius = 5.f, float slowRadius = 10.f);

//////////////////////////
//FACE
//******
//returns the angle in between the two vectors
SteeringPlugin_OutputCustom CalculateFaceSteering(const AgentInfo& agentInfo, const Elite::Vector2& target, float* angleToTarget = nullptr);

//////////////////////////
//EVADE
//******
SteeringPlugin_OutputCustom CalculateEvadeSteering(const AgentInfo& agentInfo, const Elite::Vector2& target, const Elite::Vector2& targetLinVel, float safeDistance = 10.f);

//////////////////////////
//PURSUIT
//******
SteeringPlugin_OutputCustom CalculatePursuitSteering(const AgentInfo& agentInfo, const Elite::Vector2& target, const Elite::Vector2& targetLinVel);

//////////////////////////
//HIDE
//******
SteeringPlugin_OutputCustom CalculateHideSteering(const AgentInfo& agentInfo, const Elite::Vector2& target);

//////////////////////////
//AVOIDOBSTACLE
//******
SteeringPlugin_OutputCustom CalculateAvoidObstacleSteering(const AgentInfo& agentInfo, const Elite::Vector2& target);

//////////////////////////
//ALIGN
//******
SteeringPlugin_OutputCustom CalculateAlignSteering(const AgentInfo& agentInfo, const Elite::Vector2& target);

//////////////////////////
//FACEDARRIVE
//******
SteeringPlugin_OutputCustom CalculateFacedArriveSteering(const AgentInfo& agentInfo, const Elite::Vector2& target);

//////////////////////////
//SLOWCLAP
//******
SteeringPlugin_OutputCustom CalculateSlowClapSteering(const AgentInfo& agentInfo, const Elite::Vector2& target);