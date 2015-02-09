#include "game/gui/Label.hpp"
#include "game/audio/AudioSystem.hpp"
#include "game/ui.h"

Label::Label(const std::string &text) :
	_text()
{
	setText(text);
}

void Label::draw()
{
    //Draw text
    GL_DEBUG( glColor4fv )(Ego::white_vec);
    fnt_drawTextBox_OGL(ui_getFont(), {0xFF, 0xFF, 0xFF, 0xFF}, getX(), getY(), getWidth(), getHeight(), 25, nullptr, "%s", _text.c_str());
}

void Label::setText(const std::string &text)
{
	_text = text;

	//Recalculate our size
	int textWidth, textHeight;
	fnt_getTextBoxSize(ui_getFont(), 25, _text.c_str(), &textWidth, &textHeight);
	setSize(textWidth, textHeight);
}