#include "SDLInclude.h"
#include "GameInput.h"

void GameInput_PopulateSDLScancodes(SDL_Scancode * scancode, const char ** text, size_t count)
{
   size_t i;
   for(i = 0; i < count; i++)
   {
      scancode[i] = SDL_GetScancodeFromName(text[i]);
      //printf("i: %i, s: %s, c: %i\n", i, text[i], scancode[i]); 
   }
}


