#include <math.h>
#include <iostream>

#include "hge.h"
#include "hgeSprite.h"

#include "ship.h"
#include "asteroid.h"

Asteroid::Asteroid( char* filename ) :
	x_(200),
	y_(200), 
	w_(0),
	angular_velocity(1),
	velocity_x_(0),
	velocity_y_(0)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	tex_ = hge->Texture_Load(filename);
	hge->Release();
	sprite_.reset(new hgeSprite(tex_, 0, 0, 40, 40));
	sprite_->SetHotSpot(20,20);

}

Asteroid::~Asteroid()
{
	HGE* hge = hgeCreate(HGE_VERSION);
	hge->Texture_Free(tex_);
	hge->Release();
}

bool Asteroid::Update(std::vector<Ship*> &shiplist, float timedelta)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	float pi = 3.141592654f*2;
	float oldx, oldy;

	w_ += angular_velocity * timedelta;
	if (w_ > pi)
		w_ -= pi;

	if (w_ < 0.0f)
		w_ += pi;

	oldx = x_;
	oldy = y_;
	x_ += velocity_x_ * timedelta;
	y_ += velocity_y_ * timedelta;

	for (std::vector<Ship*>::iterator thisship = shiplist.begin(); thisship != shiplist.end(); thisship++)
	{
		if( HasCollided( (*(*thisship)) ) )
		{
			// if both are stuck
		
            if( GET_ABSOLUTE_MSG( velocity_y_ ) > GET_ABSOLUTE_MSG( (*thisship)->GetServerVelocityY( ) ) )
			{
				// asteroid transfers vel to ship
                (*thisship)->SetServerVelocityY( (*thisship)->GetServerVelocityY( ) + (velocity_y_ / 3) );
				velocity_y_ = (-1 * velocity_y_);
			}
			else
			{
				// ship transfers vel to asteroid
                velocity_y_ += ((*thisship)->GetServerVelocityY( ) / 3);
                (*thisship)->SetServerVelocityY( -1 * ((*thisship)->GetServerVelocityY( )) );
				
			}

            if( GET_ABSOLUTE_MSG( velocity_x_ ) > GET_ABSOLUTE_MSG( (*thisship)->GetServerVelocityX( ) ) )
			{
                (*thisship)->SetServerVelocityX( (*thisship)->GetServerVelocityX( ) + (velocity_x_ / 3) );
				velocity_x_ = (-1 * velocity_x_);
			}
			else
			{
                velocity_x_ += ((*thisship)->GetServerVelocityX( ) / 3);
                (*thisship)->SetServerVelocityX( -1 * ((*thisship)->GetServerVelocityX( )) );

			}

            if( velocity_x_ == 0.0 && (*thisship)->GetServerVelocityX( ) == 0.0 )
			{
                if( velocity_y_ == 0.0 && (*thisship)->GetServerVelocityY( ) == 0.0 )
				{
					std::cout << "Asteroid and Ship is stuck" << std::endl;
                    std::cout << "- asteroid x" << x_ << ", y" << y_ << ", vel_x" << velocity_x_ << ", vel_y" << velocity_y_ << std::endl;
                    std::cout << "- ship     x" << (*thisship)->GetX( ) << ", y" << (*thisship)->GetY( ) << ", vel_x" << (*thisship)->GetServerVelocityX( ) << ", vel_y" << (*thisship)->GetServerVelocityY( ) << std::endl;
				}
			}
			x_ = oldx;
			y_ = oldy;
			break;
		}
	}

	float screenwidth = static_cast<float>(hge->System_GetState(HGE_SCREENWIDTH));
	float screenheight = static_cast<float>(hge->System_GetState(HGE_SCREENHEIGHT));
	float spritewidth = sprite_->GetWidth();
	float spriteheight = sprite_->GetHeight();
	if (x_ < -spritewidth/2)
		x_ += screenwidth + spritewidth;
	else if (x_ > screenwidth + spritewidth/2)
		x_ -= screenwidth + spritewidth;

	if (y_ < -spriteheight/2)
		y_ += screenheight + spriteheight;
	else if (y_ > screenheight + spriteheight/2)
		y_ -= screenheight + spriteheight;

	return false;
}

void Asteroid::Render()
{
	sprite_->RenderEx(x_, y_, w_);
}

bool Asteroid::HasCollided( Ship &ship )
{
	sprite_->GetBoundingBox( x_, y_, &collidebox);

	return collidebox.Intersect( ship.GetBoundingBox() );
}