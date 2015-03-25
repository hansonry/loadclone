#include <stdio.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

#define TILE_WIDTH             32
#define TILE_HEIGHT            32

#define IMGID_BLOCK        0x0000
#define IMGID_BROKENBLOCK  0x0001
#define IMGID_LADDER       0x0002
#define IMGID_GOLD         0x0100
#define IMGID_GUY          0x0101
#define IMGID_BAR          0x0102
#define IMGID_DOORCLOSE    0x0200
#define IMGID_DOOROPEN     0x0201    

static void CheckForExit(const SDL_Event *event, int * done);

static SDL_Texture * load_texture(SDL_Renderer * rend, const char * filename);

static void draw_at(SDL_Renderer * rend, SDL_Texture * text, int imgid, int x, int y);

int main(int args, char * argc[])
{
   SDL_Window  * window;
   SDL_Renderer * rend;   
   SDL_Event event;
   SDL_Texture * t_palet;
   int done;
   int prevTicks, diffTicks, nowTicks;
   float seconds;
   
   
   
   SDL_Init(SDL_INIT_EVERYTHING);   
   window = SDL_CreateWindow("Load Clone", 
                             SDL_WINDOWPOS_CENTERED, 
                             SDL_WINDOWPOS_CENTERED, 
                             800, 600, SDL_WINDOW_SHOWN);
   
   rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
   
   t_palet = load_texture(rend, "load.png");

   prevTicks = SDL_GetTicks();
   
   done = 0;
   while(done == 0)
   {
      while(SDL_PollEvent(&event))
      {
         CheckForExit(&event, &done);
      }
      
      nowTicks = SDL_GetTicks();
      diffTicks = nowTicks - prevTicks;
      seconds = (float)diffTicks / 1000.0f;
      prevTicks = nowTicks;
      
      SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0xFF);
      SDL_RenderClear( rend );
     
      draw_at(rend, t_palet, IMGID_GUY, 0, 0); 
      SDL_RenderPresent(rend);
   }
   
   
   
   
   SDL_DestroyRenderer(rend);
   SDL_DestroyWindow(window);
   SDL_DestroyTexture(t_palet);
   SDL_Quit();
   
   
   printf("End\n");
   return 0;
}

static void CheckForExit(const SDL_Event *event, int * done)
{
   if(event->type == SDL_QUIT)
   {
      (*done) = 1;
   }
   else if(event->type == SDL_KEYDOWN)
   {
      if(event->key.keysym.sym == SDLK_ESCAPE)
      {
         (*done) = 1;
      }
   }

}

static SDL_Texture * load_texture(SDL_Renderer * rend, const char * filename)
{
   SDL_Surface * surf;
   SDL_Texture * text;
   surf = IMG_Load(filename);
   if(surf == NULL)
   {
      printf("Error: could not load %s\n", filename);
      return NULL;
   }

   text = SDL_CreateTextureFromSurface(rend, surf);

   SDL_FreeSurface(surf);
   return text;

}

static void draw_at(SDL_Renderer * rend, SDL_Texture * text, int imgid, int x, int y)
{   
   SDL_Rect r_src;
   SDL_Rect r_dest;
   int ix, iy;
   
   r_dest.x = x;
   r_dest.y = y;
   r_dest.w = TILE_WIDTH;
   r_dest.h = TILE_HEIGHT;


   ix = imgid & 0xFF;
   iy = (imgid >> 8) & 0xFF;

   r_src.x = ix * TILE_WIDTH;
   r_src.y = iy * TILE_HEIGHT;
   r_src.w = TILE_WIDTH;
   r_src.h = TILE_HEIGHT;

   SDL_RenderCopy(rend, text, &r_src, &r_dest);
}


