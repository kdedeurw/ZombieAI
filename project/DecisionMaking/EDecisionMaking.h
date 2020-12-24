#pragma once

class IDecisionMaking
{
public:
	IDecisionMaking() = default;
	virtual ~IDecisionMaking() = default;

	virtual void Update(float deltaTime) = 0;
};