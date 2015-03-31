#include <stdlib.h>
#include <string.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

#include "FontText.h"


static void FontText_RegenImage(FontText_T * font_text);
static int FontText_CheckString(FontText_T * font_text, const char * string);
static int FontText_CheckColor(FontText_T * font_text, int r, int g, int b, int a);

void FontText_Init(FontText_T * font_text, TTF_Font * font, SDL_Renderer * rend)
{
   font_text->font = font;
   font_text->text = NULL;
   font_text->texture = NULL;
   font_text->color.r = 0xFF;
   font_text->color.g = 0xFF;
   font_text->color.b = 0xFF;
   font_text->color.a = 0xFF;
   font_text->rend = rend;

}

void FontText_Destroy(FontText_T * font_text)
{
   if(font_text->text != NULL)
   {
      free(font_text->text);
      font_text->text = NULL;
   }

   if(font_text->texture != NULL)
   {
      SDL_DestroyTexture(font_text->texture);
      font_text->texture = NULL;
   }
}

void FontText_SetString(FontText_T * font_text, const char * string)
{
   if(FontText_CheckString(font_text, string) == 1)
   {
      FontText_RegenImage(font_text);
   }
}

void FontText_SetColor(FontText_T * font_text, int r, int g, int b, int a)
{
   if(FontText_CheckColor(font_text, r, g, b, a) == 1)
   {
      FontText_RegenImage(font_text);
   }
}

void FontText_SetStringAndText(FontText_T * font_text, const char * string, int r, int g, int b, int a)
{
   if(FontText_CheckString(font_text, string)    == 1 ||
      FontText_CheckColor(font_text, r, g, b, a) == 1)
   {
      FontText_RegenImage(font_text);
   }
}

void FontText_Render(FontText_T * font_text, int x, int y)
{
   if(font_text->texture != NULL)
   {
      font_text->rend_rect.x = x;
      font_text->rend_rect.y = y;
      SDL_RenderCopy(font_text->rend, 
                     font_text->texture, 
                     NULL, 
                     &font_text->rend_rect);
   }
}


static void FontText_RegenImage(FontText_T * font_text)
{
   SDL_Surface * surf;
   if(font_text->text != NULL)
   {
      surf = TTF_RenderText_Blended(font_text->font, font_text->text, font_text->color);
      font_text->rend_rect.w = surf->w;
      font_text->rend_rect.h = surf->h;

      if(font_text->texture != NULL)
      {
         SDL_DestroyTexture(font_text->texture);
      }
      font_text->texture = SDL_CreateTextureFromSurface(font_text->rend, surf);
      SDL_FreeSurface(surf);
   }
}



static int FontText_CheckString(FontText_T * font_text, const char * string)
{
   int update;
   size_t length;
   if(font_text->text == NULL || strcmp(string, font_text->text) != 0)
   {
      update = 1;
      length = strlen(string) + 1;
      if(font_text->text != NULL)
      {
         free(font_text->text);
      }
      font_text->text = malloc(sizeof(char) * length);
      memcpy(font_text->text, string, sizeof(char) * length);
   }
   else
   {
      update = 0;
   }
   return update;
}
static int FontText_CheckColor(FontText_T * font_text, int r, int g, int b, int a)
{
   int update;
   if(font_text->color.r != r || 
      font_text->color.g != g || 
      font_text->color.b != b || 
      font_text->color.a != a)
   {
      update = 1;
      font_text->color.r = r;
      font_text->color.g = g;
      font_text->color.b = b;
      font_text->color.a = a;
   }
   else
   {
      update = 0;
   }
   return update;
}

