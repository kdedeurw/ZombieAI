#pragma once
#include <Exam_HelperStructs.h>
#include "SteeringBehaviors.h"

struct SteeringPlugin_OutputCustom : public SteeringPlugin_Output
{
	//bool IsValid{};
	SteeringPlugin_OutputCustom()
		: SteeringPlugin_Output{}
		//,IsValid{}
	{
	}

	SteeringPlugin_OutputCustom& operator=(const SteeringPlugin_Output& other)
	{
		LinearVelocity = other.LinearVelocity;
		AngularVelocity = other.AngularVelocity;
		//IsValid = other.IsValid;

		return *this;
	}

	SteeringPlugin_OutputCustom& operator+(const SteeringPlugin_Output& other)
	{
		LinearVelocity += other.LinearVelocity;
		AngularVelocity += other.AngularVelocity;

		return *this;
	}

	SteeringPlugin_OutputCustom& operator*=(const SteeringPlugin_Output& other)
	{
		LinearVelocity = LinearVelocity * other.LinearVelocity;
		AngularVelocity = AngularVelocity * other.AngularVelocity;

		return *this;
	}

	SteeringPlugin_OutputCustom& operator*=(float f)
	{
		LinearVelocity = f * LinearVelocity;
		AngularVelocity = f * AngularVelocity;

		return *this;
	}

	SteeringPlugin_OutputCustom& operator/=(float f)
	{
		LinearVelocity = LinearVelocity / f;
		AngularVelocity = AngularVelocity / f;

		return *this;
	}
};