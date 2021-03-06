#ifndef _EXPPACK_H_
#define _EXPPACK_H_
#include "PowerupBaseClass.h"

#include <memory>
#include <vector>

#include "hge.h"
#include "hgerect.h"

class hgeSprite;
class hgeRect;
class Ship;
class ExpPack : public PowerupBaseClass
{
	HTEXTURE tex_; //!< Handle to the sprite's texture
	std::auto_ptr<hgeSprite> sprite_; //!< The sprite used to display
	float x_; //!< The x-ordinate
	float y_; //!< The y-ordinate
	hgeRect collidebox;

public:
	ExpPack(char* filename, float x, float y, float duration);
	~ExpPack();
	Ship* Update(std::vector<Ship*> &shiplist, float timedelta);
	void Render();
	bool HasCollided(Ship &ship);

	void UpdateLoc(float x, float y)
	{
		x_ = x;
		y_ = y;
	}

	float GetX() const
	{
		return x_;
	}

	float GetY() const
	{
		return y_;
	}
};

#endif