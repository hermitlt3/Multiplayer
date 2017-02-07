#ifndef _POWERUP_BASE_CLASS_
#define _POWERUP_BASE_CLASS_

class Powerup
{
	float duration;
	float timer;

public:
	Powerup();
	~Powerup();

	enum POWERUP_TYPE
	{
		SHIELD = 0,
		TOTAL_TYPES
	};

};

#endif