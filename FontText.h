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
  
