/*
 *  Copyright (C) 2015 Ryan Hanson <hansonry@gmail.com>
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty.  In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 *
 */
#ifndef __FONTTEXT_H__
#define __FONTTEXT_H__


typedef struct FontText_S FontText_T;
struct FontText_S
{
   TTF_Font * font;
   char * text;
   SDL_Texture * texture;
   SDL_Renderer * rend;
   SDL_Rect rend_rect;
   SDL_Color color;
};

void FontText_Init(FontText_T * font_text, TTF_Font * font, SDL_Renderer * rend);
void FontText_Destroy(FontText_T * font_text);

void FontText_SetString(FontText_T * font_text, const char * string);
void FontText_SetColor(FontText_T * font_text, int r, int g, int b, int a);
void FontText_SetStringAndText(FontText_T * font_text, const char * string, int r, int g, int b, int a);

void FontText_Render(FontText_T * font_text, int x, int y);
#endif // __FONTTEXT_H__
  
