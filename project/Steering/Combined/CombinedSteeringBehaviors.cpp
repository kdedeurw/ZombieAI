#include "stdafx.h"
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
//#include <Exam_HelperStructs.h>
#include "../SteeringHelpers.h"

//****************
//BLENDED STEERING
SteeringPlugin_OutputCustom CalculateBlendedSteering(const AgentInfo& agentInfo, const Elite::Vector2& target, const CustomSteeringData& data, const std::vector<WeightedBehaviour>& weightedBehaviours, float offset, float radius, float maxJitterOffset)
{
	SteeringPlugin_OutputCustom blendedSteering{};
	auto totalWeight = 0.f;

	for (auto weightedBehavior : weightedBehaviours)
	{
		auto steering = weightedBehavior.CalculateSteeringFunction(agentInfo, target, data);
		blendedSteering.LinearVelocity += weightedBehavior.weight * steering.LinearVelocity;
		blendedSteering.AngularVelocity += weightedBehavior.weight * steering.AngularVelocity;

		totalWeight += weightedBehavior.weight;
	}

	if (totalWeight > 0.f)
	{
		auto scale = 1.f / totalWeight;
		blendedSteering *= scale;
	}

	//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), blendedSteering.LinearVelocity, 7, { 0, 1, 1 }, 0.40f);

	return blendedSteering;
}

//*****************
//PRIORITY STEERING
//SteeringPlugin_OutputCustom PrioritySteering::CalculateSteering(Elite::Blackboard* pBlackboard)
//{
//	SteeringPlugin_OutputCustom steering{};
//
//	for (auto pBehavior : m_PriorityBehaviors)
//	{
//		steering = pBehavior->CalculateSteering(deltaT, pAgent);
//
//		if (steering.IsValid)
//			break;
//	}
//
//	//If non of the behavior return a valid output, last behavior is returned
//	return steering;
//}