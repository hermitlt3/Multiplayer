#include <memory>
#include <vector>

#include "hge.h"
#include "hgerect.h"

class hgeSprite;
class hgeRect;
class Ship;

#ifndef _ENERGYBALL_
#define _ENERGYBALL_

class Energyball
{
	HTEXTURE tex_; //!< Handle to the sprite's texture
	std::auto_ptr<hgeSprite> sprite_; //!< The sprite used to display the missile
	float x_; //!< The x-ordinate of the missile
	float y_; //!< The y-ordinate of the missile
	float w_; //!< The angular position of the missile
	float velocity_x_; //!< The resolved velocity of the missile along the x-axis
	float velocity_y_; //!< The resolved velocity of the missile along the y-axis
	hgeRect collidebox;
	int ownerid;
	float angular_velocity;

	bool xflip;
	bool yflip; 
	float xmax;
	float ymax;

#ifdef INTERPOLATEMOVEMENT
	float server_x_;
	float server_y_;
	float server_w_;
	float client_x_;
	float client_y_;
	float client_w_;
	float server_velx_;
	float server_vely_;
	float ratio_;
#endif

public:
	Energyball(char* filename, float x, float y, float w, int shipid);
	~Energyball();
	Ship* Update(std::vector<Ship*> &shiplist, float timedelta);
	void Render();
	bool HasCollided(Ship &ship);

	void UpdateLoc(float x, float y, float w)
	{
		x_ = x;
		y_ = y;
		w_ = w;
	}

	int GetOwnerID()
	{
		return ownerid;
	}

	float GetX() const
	{
		return x_;
	}

	float GetY() const
	{
		return y_;
	}

	float GetW() const
	{
		return w_;
	}

#ifdef INTERPOLATEMOVEMENT
	void SetServerLocation(float x, float y, float w) {
		server_x_ = x;
		server_y_ = y;
		server_w_ = w;
	}

	void SetServerVelocity(float vel_x, float vel_y, float angular)
	{
		server_velx_ = vel_x;
		server_vely_ = vel_y;
		angular_velocity = angular;
	}

	void SetServerVelocityX(float velocity) { server_velx_ = velocity; }
	void SetServerVelocityY(float velocity) { server_vely_ = velocity; }

	float GetServerVelocityX() { return server_velx_; }
	float GetServerVelocityY() { return server_vely_; }

	float GetAngularVelocity() { return angular_velocity; }
	void SetAngularVelocity(float av) { angular_velocity = av; }

	void SetServerX(float x) { server_x_ = x; }
	void SetServerY(float y) { server_y_ = y; }
	void SetServerW(float w) { server_w_ = w; }

	float GetServerX() { return server_x_; }
	float GetServerY() { return server_y_; }
	float GetServerW() { return server_w_; }

	void  SetRatio(float r) { ratio_ = r; }
	float GetRatio() { return ratio_; }

	void DoInterpolateUpdate()
	{
		client_x_ = x_;
		client_y_ = y_;
		client_w_ = w_;
		velocity_x_ = server_velx_;
		velocity_y_ = server_vely_;
		ratio_ = 0;
	}
#endif
};

#endif