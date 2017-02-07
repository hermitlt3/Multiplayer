#ifndef _POWERUP_BASECLASS_
#define _POWERUP_BASECLASS_

#include <vector>

class Ship;
class PowerupBaseClass
{
public:

	enum POWERUP_TYPE
	{
		EXP = 0,
		HEALTH,
		TOTAL_TYPES
	};

	PowerupBaseClass();
	PowerupBaseClass(float d, float t, POWERUP_TYPE type);
	virtual ~PowerupBaseClass();
	virtual Ship* Update(std::vector<Ship*> &shiplist, float timedelta) = 0;
	virtual void Render() = 0; 

	POWERUP_TYPE returnType() {
		return type;
	}

protected:
	float duration;
	float timer;
	POWERUP_TYPE type;
};

#endif