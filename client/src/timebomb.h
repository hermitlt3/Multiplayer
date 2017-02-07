#ifndef _TIMEBOMB_H_
#define _TIMEBOMB_H_

#include <memory>
#include <vector>

#include "hge.h"
#include "hgerect.h"
#include "hgeFont.h"

class hgeSprite;
class hgeRect;
class Ship;

class Timebomb
{
	HTEXTURE tex_; //!< Handle to the sprite's texture
	std::auto_ptr<hgeSprite> sprite_; //!< The sprite used to display the bomb
	std::auto_ptr<hgeFont> font_; 
	float x_; //!< The x-ordinate of the bomb
	float y_; //!< The y-ordinate of the bomb
	hgeRect explosionRect;
	// Assignment 2
	float destructTimer;
	float explosionRadius_;
	bool toDelete;

public:
	Timebomb(char* filename, float x, float y, float explosionRadius);
	~Timebomb();
	std::vector<Ship*> Update(std::vector<Ship*> &shiplist, float timedelta);
	void Render();
	bool InExplosionRadius(Ship &ship);

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

	float GetToDeleted() const
	{
		return toDelete;
	}
};

#endif