#pragma once
#include "../SteeringBehaviors.h"
#include <functional>
#include <vector>

struct SteeringPlugin_Output;
struct SteeringPlugin_OutputCustom;

//****************
//BLENDED STEERING
struct WeightedBehaviour
{
	std::function<SteeringPlugin_OutputCustom(const AgentInfo&, const Elite::Vector2&, const CustomSteeringData&)> CalculateSteeringFunction;
	float weight = 0.f;

	WeightedBehaviour() :
		WeightedBehaviour{ nullptr, 0.f }
	{};
	WeightedBehaviour(const std::function<SteeringPlugin_OutputCustom(const AgentInfo&, const Elite::Vector2&, const CustomSteeringData&)>& function, float weight) :
		CalculateSteeringFunction(function),
		weight(weight)
	{};
};

SteeringPlugin_OutputCustom CalculateBlendedSteering(const AgentInfo& agentInfo, const Elite::Vector2& target, const CustomSteeringData& data, const std::vector<WeightedBehaviour>& weightedBehaviours, float offset = 6.f, float radius = 4.f, float maxJitterOffset = 1.f);


//*****************
//PRIORITY STEERING
//class PrioritySteering final: public ISteeringBehavior
//{
//public:
//	PrioritySteering(vector<ISteeringBehavior*> priorityBehaviors) 
//		:m_PriorityBehaviors(priorityBehaviors) 
//	{}
//
//	void AddBehaviour(ISteeringBehavior* pBehavior) { m_PriorityBehaviors.push_back(pBehavior); }
//	SteeringPlugin_OutputCustom CalculateSteering(Elite::Blackboard* pBlackboard);
//
//private:
//	vector<ISteeringBehavior*> m_PriorityBehaviors = {};
//};