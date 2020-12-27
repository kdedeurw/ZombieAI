//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
//#include <EliteMath/EMath.h>

//////////////////////////
//SEEK
//****
SteeringPlugin_Output CalculateSeekSteering(const AgentInfo& agentInfo, const Elite::Vector2& target)
{
	SteeringPlugin_Output steering{};
	
	steering.LinearVelocity = target - agentInfo.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;
	
	//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0, 1, 0 }, 0.4f);
	
	return steering;
}

//WANDER (base> SEEK)
//******
SteeringPlugin_Output CalculateWanderSteering(const AgentInfo& agentInfo, const Elite::Vector2& target, float offset, float radius, float maxJitterOffset)
{
	// calculate random point on circle
	const float halfJitter = maxJitterOffset / 2;
	const Elite::Vector2 randomOffset = Elite::Vector2{ Elite::randomFloat(-halfJitter, halfJitter), Elite::randomFloat(-halfJitter, halfJitter) };
	// 'normalized' vector with length defined by halfJitter(==radius of circle?), should always point on a random point on the circle
	WanderTarget += randomOffset;
	WanderTarget.Normalize();
	WanderTarget *= radius;
	// offset is added to actual target, normalized and multiplied by radius of circle, so the point will be on the circle itself

	// add offset
	Elite::Vector2 offsetVec{ agentInfo.LinearVelocity };
	offsetVec.Normalize();
	offsetVec *= offset;
	// normalized vector pointing in the direction of the agent * offset(==add extra length)
	// this wandertarget now just gets its offset, based upon wherever the agent is looking * some length

	Elite::Vector2 newTarget = agentInfo.Position + offsetVec + WanderTarget;

	//DEBUGRENDERER2D->DrawSegment(agentInfo.Position, agentInfo.Position + offset, { 0.f, 0.f, 1.f, 0.5f }, 0.4f);
	//DEBUGRENDERER2D->DrawCircle(agentInfo.Position + offset, m_Radius, { 0.f, 0.f, 1.f, 0.5f }, 0.4f);
	//DEBUGRENDERER2D->DrawSolidCircle(agentInfo.Position + offset + m_Wandertarget, 0.5f, { 0,0 }, { 0.1f, 1.f, 0.f, 0.5f }, 0.3f);

	return CalculateSeekSteering(agentInfo, newTarget);
}

//FLEE (base> SEEK)
//******
SteeringPlugin_Output CalculateFleeSteering(const AgentInfo& agentInfo, const Elite::Vector2& target)
{
	//m_Target.Position = -m_Target.Position;
	//return Seek::CalculateSteering(deltaT, pAgent);
	//bug with evade

	//SteeringOutput steering{ Seek::CalculateSteering(deltaT, pAgent) };
	//steering.LinearVelocity = -steering.LinearVelocity;

	SteeringPlugin_Output steering{};

	//opposite of Seek
	steering.LinearVelocity = agentInfo.Position - target;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

	//DEBUGRENDERER2D->DrawDirection(agentInfo.Position, steering.LinearVelocity, 5, { 0, 1, 0 }, 0.4f);

	return steering;
}

//ARRIVE
//******
SteeringPlugin_Output CalculateArriveSteering(const AgentInfo& agentInfo, const Elite::Vector2& target, float arrivalRadius, float slowRadius)
{
	SteeringPlugin_Output steering{};

	Elite::Vector2 toTarget{ target - agentInfo.Position };
	const float distance = toTarget.Normalize();
	const float maxSpeed{ agentInfo.MaxLinearSpeed };
	//const float arrivalRadius{ pAgent->GetMass() * maxSpeed };
	if (distance < arrivalRadius)
	{
		steering.LinearVelocity = Elite::ZeroVector2;
		return steering;
	}

	if (distance < slowRadius) // if predefined distance is smaller than slowRadius, start slowing down
		toTarget *= maxSpeed * (distance / maxSpeed);
	else
		toTarget *= maxSpeed;

	steering.LinearVelocity = toTarget;

	//	DEBUGRENDERER2D->DrawDirection(agentInfo.Position,
	//		steering.LinearVelocity,
	//		steering.LinearVelocity.GetNormalized().Magnitude(),
	//		{ 0.f, 1.f, 0.f, 0.5f },
	//		0.4f);

	return steering;
	//return Seek::CalculateSteering(deltaT, pAgent);
}

//FACE
//******
SteeringPlugin_Output CalculateFaceSteering(const AgentInfo& agentInfo, const Elite::Vector2& target, float* angleToTarget)
{
	SteeringPlugin_Output steering{};
	steering.AutoOrient = false;

	const float orientation = agentInfo.Orientation; // X-value-orientation, !used for cos and sin!
	const Elite::Vector2 agentLookAtDir{ cosf(orientation), sinf(orientation) }; // instead of linear velocity, we create a lookat direction through the X-value-orientation and cos/sin
	Elite::Vector2 agentToTargetDir{ target - agentInfo.Position }; // direction of agent towards target
	agentToTargetDir.Normalize(); // normalize, obv.
	const float angleBetween = Elite::Dot(agentLookAtDir, agentToTargetDir); // dot or angle between the 2 vectors (pos and neg results)
	steering.AngularVelocity = agentInfo.MaxAngularSpeed * angleBetween; // speed * whatever signed amount we have

	if (angleToTarget)
		*angleToTarget = angleBetween;
	
	//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(),
	//	agentLookAtDir,
	//	1.f,
	//	{ 1.f, 0.f, 0.f, 0.5f },
	//	0.4f);
	//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(),
	//	agentToTargetDir,
	//	steering.AngularVelocity + 1.f,
	//	{ 0.f, 1.f, 0.f, 0.5f },
	//	0.4f);
	

	return steering;
}

//EVADE (base> FLEE)
//****
SteeringPlugin_Output CalculateEvadeSteering(const AgentInfo& agentInfo, const Elite::Vector2& target, const Elite::Vector2& targetLinVel, float safeDistance)
{
	const float distanceToTarget = Elite::Distance(agentInfo.Position, target);
	if (distanceToTarget > safeDistance)
	{
		SteeringPlugin_Output steering{};
		//steering.IsValid = false;
		return steering;
	}

	//calculate future position of target
	Elite::Vector2 newTarget = target + targetLinVel.GetNormalized() * agentInfo.MaxLinearSpeed;
	return CalculateFleeSteering(agentInfo, newTarget);
}

//PURSUIT (base> SEEK)
//****
SteeringPlugin_Output CalculatePursuitSteering(const AgentInfo& agentInfo, const Elite::Vector2& target, const Elite::Vector2& targetLinVel)
{
	Elite::Vector2 newTarget = target + targetLinVel.GetNormalized() * agentInfo.MaxLinearSpeed;
	return CalculateSeekSteering(agentInfo, newTarget);
}

//HIDE
//****
SteeringPlugin_Output CalculateHideSteering(const AgentInfo& agentInfo, const Elite::Vector2& target)
{
	SteeringPlugin_Output steering{};

	return steering;
}

//AVOIDOBSTACLE
//****
SteeringPlugin_Output CalculateAvoidObstacleSteering(const AgentInfo& agentInfo, const Elite::Vector2& target)
{
	SteeringPlugin_Output steering{};

	return steering;
}

//ALIGN
//****
SteeringPlugin_Output CalculateAlignSteering(const AgentInfo& agentInfo, const Elite::Vector2& target)
{
	SteeringPlugin_Output steering{};

	return steering;
}

//FACEDARRIVE
//****
SteeringPlugin_Output CalculateFacedArriveSteering(const AgentInfo& agentInfo, const Elite::Vector2& target)
{
	SteeringPlugin_Output steering{};

	return steering;
}

//SLOWCLAP
//****
SteeringPlugin_Output CalculateSlowClapSteering(const AgentInfo& agentInfo, const Elite::Vector2& target)
{
	SteeringPlugin_Output steering{};

	return steering;
}