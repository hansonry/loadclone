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

#define MOVE_TIME          0.5f
#define MOTION_STATE_NOT_MOVING  0
#define MOTION_STATE_MOVEING     1


#define TMAP_TILE_AIR  0
#define TMAP_TILE_DIRT 1

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

typedef struct player_data_s player_data_t;
struct player_data_s
{
   int input_flags[e_pi_last];
   pos_t grid_p;
   pos_t next_grid_p;
   int motion_state;
   float move_timer;
};

typedef struct TerrainMap_S TerrainMap_T;
struct TerrainMap_S
{
   int width;
   int height;
   int * data;
};

static void TerrainMap_Init(TerrainMap_T * map, int width, int height);

static void TerrainMap_Destroy(TerrainMap_T * map);

static void TerrainMap_Render(TerrainMap_T * map, SDL_Renderer * rend, SDL_Texture * t_palet); 

static void CheckForExit(const SDL_Event *event, int * done);

static SDL_Texture * load_texture(SDL_Renderer * rend, const char * filename);

static void draw_at(SDL_Renderer * rend, SDL_Texture * text, int imgid, int x, int y);

static void handle_input(const SDL_Event * event, int * done, int * player_input_flags);

static void handle_update(float seconds, player_data_t * player1_data);

static void handle_render(SDL_Renderer * rend, SDL_Texture * t_palet, TerrainMap_T * tmap, player_data_t * player1_data);

int main(int args, char * argc[])
{
   SDL_Window  * window;
   SDL_Renderer * rend;   
   SDL_Event event;
   SDL_Texture * t_palet;
   int done;
   int prevTicks, diffTicks, nowTicks;
   float seconds;

   int i;
   

 
   player_data_t player1_data;
   TerrainMap_T tmap;

   for(i = 0; i < e_pi_last; i++)
   {
      player1_data.input_flags[i] = 0;
   }
   player1_data.grid_p.x = 0;
   player1_data.grid_p.y = 0;
   player1_data.motion_state = MOTION_STATE_NOT_MOVING;
   
   TerrainMap_Init(&tmap, 10, 10);

   for(i = 0; i < 10; i++)
   {
      tmap.data[i + 90] = TMAP_TILE_DIRT;
   }

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
         handle_input(&event, &done, player1_data.input_flags);
      }
      
      nowTicks = SDL_GetTicks();
      diffTicks = nowTicks - prevTicks;
      seconds = (float)diffTicks / 1000.0f;
      prevTicks = nowTicks;
      
      handle_update(seconds, &player1_data);
      
      SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0xFF);
      SDL_RenderClear( rend );
      
      handle_render(rend, t_palet, &tmap, &player1_data);
      SDL_RenderPresent(rend);
   }
   
   
   TerrainMap_Destroy(&tmap); 
   
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

static void handle_update(float seconds, player_data_t * player1_data)
{
   if(player1_data->motion_state == MOTION_STATE_NOT_MOVING)
   {
      player1_data->next_grid_p.x = player1_data->grid_p.x;
      player1_data->next_grid_p.y = player1_data->grid_p.y;
      if(player1_data->input_flags[e_pi_move_up]    == 1)
      {
         player1_data->next_grid_p.y --;
      }
      if(player1_data->input_flags[e_pi_move_down]  == 1)
      {
         player1_data->next_grid_p.y ++;
      }
      if(player1_data->input_flags[e_pi_move_left]  == 1)
      {
         player1_data->next_grid_p.x --;
      }
      if(player1_data->input_flags[e_pi_move_right] == 1)
      {
         player1_data->next_grid_p.x ++;
      }

      if(player1_data->next_grid_p.x != player1_data->grid_p.x ||
         player1_data->next_grid_p.y != player1_data->grid_p.y)
      {
         player1_data->motion_state = MOTION_STATE_MOVEING;
         player1_data->move_timer = 0;
      }
   }
   else if(player1_data->motion_state == MOTION_STATE_MOVEING)
   {
      player1_data->move_timer += seconds;
      if(player1_data->move_timer >= MOVE_TIME)
      {
         player1_data->motion_state = MOTION_STATE_NOT_MOVING;
         player1_data->grid_p.x = player1_data->next_grid_p.x;
         player1_data->grid_p.y = player1_data->next_grid_p.y;
      }
   }
}

static void handle_render(SDL_Renderer * rend, SDL_Texture * t_palet, TerrainMap_T * tmap, player_data_t * player1_data)
{
   pos_t draw_loc;
   pos_t diff;
   float move_percent;

   TerrainMap_Render(tmap, rend, t_palet);
   if(player1_data->motion_state == MOTION_STATE_NOT_MOVING)
   {
      draw_loc.x = player1_data->grid_p.x * TILE_WIDTH;
      draw_loc.y = player1_data->grid_p.y * TILE_HEIGHT;
   }
   else if(player1_data->motion_state == MOTION_STATE_MOVEING)
   {
      diff.x = (player1_data->next_grid_p.x - player1_data->grid_p.x) * TILE_WIDTH;
      diff.y = (player1_data->next_grid_p.y - player1_data->grid_p.y) * TILE_HEIGHT;
      move_percent = player1_data->move_timer / MOVE_TIME;
      draw_loc.x = (player1_data->grid_p.x * TILE_WIDTH) + 
                   (int)(diff.x * move_percent);
      draw_loc.y = (player1_data->grid_p.y * TILE_HEIGHT) + 
                   (int)(diff.y * move_percent);

   }
   else
   {
      draw_loc.x = 0;
      draw_loc.y = 0;
   }


   draw_at(rend, t_palet, IMGID_GUY, draw_loc.x, 
                                     draw_loc.y); 
 
}

// S TerrainMap


static void TerrainMap_Init(TerrainMap_T * map, int width, int height)
{
   size_t i;
   size_t size;
   map->width  = width; 
   map->height = height;
   size        = width * height;
   map->data   = malloc(sizeof(int) * size);
   for(i = 0; i < size; i ++)
   {
      map->data[i] = TMAP_TILE_AIR;
   }
}

static void TerrainMap_Destroy(TerrainMap_T * map)
{
   free(map->data);
   map->data = NULL;
}

static void TerrainMap_Render(TerrainMap_T * map, SDL_Renderer * rend, SDL_Texture * t_palet)
{
   int index;
   int size;
   int tile_rend;
   pos_t p, c;

   p.x = 0;
   p.y = 0;
   index = 0;
   c.x = 0;
   c.y = 0; 
   while(p.y < map->height)
   {

      switch(map->data[index])
      {
         case TMAP_TILE_DIRT:
         draw_at(rend, t_palet, IMGID_BLOCK, c.x, c.y);
         break;
      }

      index ++;
      p.x ++;
      c.x += TILE_WIDTH;
      if(p.x >= map->width)
      {
         p.x = 0;
         c.x = 0;
         p.y ++;
         c.y += TILE_HEIGHT;
      }
   }
}

// E TerrainMap


