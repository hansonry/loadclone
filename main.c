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

typedef enum player_input_e player_input_t;
enum player_input_e
{
   e_pi_move_up,
   e_pi_move_down,
   e_pi_move_left,
   e_pi_move_right,
   e_pi_last
};

typedef struct pos_s pos_t;
struct pos_s
{
   int x;
   int y;
};

static void CheckForExit(const SDL_Event *event, int * done);

static SDL_Texture * load_texture(SDL_Renderer * rend, const char * filename);

static void draw_at(SDL_Renderer * rend, SDL_Texture * text, int imgid, int x, int y);

static void handle_input(const SDL_Event * event, int * done, int * player_input_flags);

int main(int args, char * argc[])
{
   SDL_Window  * window;
   SDL_Renderer * rend;   
   SDL_Event event;
   SDL_Texture * t_palet;
   int done;
   int prevTicks, diffTicks, nowTicks;
   float seconds;

   int player_input_flags[e_pi_last];
   int i;
   

 
   pos_t player_grid_p;  


   for(i = 0; i < e_pi_last; i++)
   {
      player_input_flags[i] = 0;
   }


   player_grid_p.x = 0;
   player_grid_p.y = 0;
   
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
         handle_input(&event, &done, player_input_flags);
      }
      
      nowTicks = SDL_GetTicks();
      diffTicks = nowTicks - prevTicks;
      seconds = (float)diffTicks / 1000.0f;
      prevTicks = nowTicks;
      
      if(player_input_flags[e_pi_move_up] == 1)
      {
         player_grid_p.y --;
      }
      if(player_input_flags[e_pi_move_down] == 1)
      {
         player_grid_p.y ++;
      }
      if(player_input_flags[e_pi_move_left] == 1)
      {
         player_grid_p.x --;
      }
      if(player_input_flags[e_pi_move_right] == 1)
      {
         player_grid_p.x ++;
      }


      
      SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0xFF);
      SDL_RenderClear( rend );
     
      draw_at(rend, t_palet, IMGID_GUY, player_grid_p.x * TILE_WIDTH, 
                                        player_grid_p.y * TILE_HEIGHT); 
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


static void handle_input(const SDL_Event * event, int * done, int * player_input_flags)
{
   player_input_t key;
   CheckForExit(event, done);

   if(event->key.keysym.sym == SDLK_KP_8)
   {
      key = e_pi_move_up;
   }
   else if(event->key.keysym.sym == SDLK_KP_5)
   {
      key = e_pi_move_down;
   }
   else if(event->key.keysym.sym == SDLK_KP_4)
   {
      key = e_pi_move_left;
   }
   else if(event->key.keysym.sym == SDLK_KP_6)
   {
      key = e_pi_move_right;
   }
   else
   {
      key = e_pi_last;
   }

   if(key != e_pi_last)
   {
      if(event->type == SDL_KEYDOWN) player_input_flags[key] = 1;
      if(event->type == SDL_KEYUP)   player_input_flags[key] = 0;
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


