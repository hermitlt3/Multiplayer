#ifndef _BOOM_H_
#define _BOOM_H_

#include <memory>
#include <vector>

#include "hge.h"
#include "hgerect.h"

class hgeSprite;
class hgeRect;
class Ship;

class Boom
{
	HTEXTURE tex_; //!< Handle to the sprite's texture
	std::auto_ptr<hgeSprite> sprite_; //!< The sprite used to display the ship
	float x_; //!< The x-ordinate of the ship
	float y_; //!< The y-ordinate of the ship
	float timer_;
	float lifetime_;

public:
	Boom(char* filename, float x, float y, float lifetime);
	~Boom();
	bool Update(float timedelta);
	void Render();

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