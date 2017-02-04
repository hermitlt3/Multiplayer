#ifndef _ASTEROID_H_
#define _ASTEROID_H_

#include <hge.h>
#include <hgerect.h>
#include <memory>
#include <vector>

class hgeSprite;
class hgeRect;
class Ship;

class Asteroid
{
	HTEXTURE tex_; //!< Handle to the sprite's texture
	std::auto_ptr<hgeSprite> sprite_; //!< The sprite used to display the asteroid
	float x_; //!< The x-ordinate of the asteroid
	float y_; //!< The y-ordinate of the asteroid
	float w_; //!< The angular position of the asteroid
	float velocity_x_; //!< The resolved velocity of the asteroid along the x-axis
	float velocity_y_; //!< The resolved velocity of the asteroid along the y-axis
	hgeRect collidebox; //!< For use in collision detection

public:
	float angular_velocity;
	Asteroid(char* filename);
	~Asteroid();
	bool Update(std::vector<Ship*> &shiplist, float timedelta);
	void Render();
	bool HasCollided( Ship &ship );
	
};

#endif