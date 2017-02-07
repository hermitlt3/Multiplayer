#include "PowerupBaseClass.h"

PowerupBaseClass::PowerupBaseClass()
{
	duration = 0.f;
	timer = 0.f;
}

PowerupBaseClass::PowerupBaseClass(float d, float t, POWERUP_TYPE type)
{
	duration = d;
	timer = t;
	this->type = type;
}

PowerupBaseClass::~PowerupBaseClass()
{

}

