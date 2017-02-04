#ifndef _TEXTBOX_H_
#define _TEXTBOX_H_

#include <memory>
#include <string>

#include "hge.h"

class hgeFont;

class TextBox
{
public:

	std::auto_ptr<hgeFont> font_; //!< The sprite used to display the textbox
	std::string mytext_;
	float posx_;
	float posy_;	

	TextBox(char* filename);
	~TextBox();

	void Render();
	void SetText(const char * text);
	void SetPos( float x, float y );

};

#endif