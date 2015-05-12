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
#include <stdio.h>
#include "SDLInclude.h"

#include "GlobalData.h"
#include "SDLTools.h"

#include "ArrayList.h"
#include "Pos2D.h"
#include "Level.h"
#include "LevelSet.h"
#include "FontText.h"

#include "EventSys.h"
#include "GameInput.h"
#include "GameConfigData.h"
#include "GameSettings.h"
#include "GameInput.h"

#define PLAYER_STATE_NOT_MOVING  0
#define PLAYER_STATE_MOVING      1
#define PLAYER_STATE_FALLING     2
#define PLAYER_STATE_DIGGING     3
#define PLAYER_STATE_DEATH       4
#define PLAYER_STATE_WIN         5


#define MARGIN_TOP     48
#define MARGIN_BOTTOM  20
#define MARGIN_LEFT    20
#define MARGIN_RIGHT   20

typedef struct PlayerData_S PlayerData_T;
struct PlayerData_S
{
   int input_flags[e_gipk_last];
   Pos2D_T grid_p;
   Pos2D_T next_grid_p;
   int player_state;
   float move_timer;
   float move_timeout;
   ESInbox_T * inbox_levelstartpos;
   ESInbox_T * inbox_inputstate;
};

typedef struct GameLevelData_S GameLevelData_T;
struct GameLevelData_S
{
   int level_index;
   LevelSet_T * levelset;
   Level_T * level;
   ESInbox_T * inbox_playerongold;
   ESInbox_T * inbox_initlevel;
};


typedef struct GameRenderData_S GameRenderData_T;
struct GameRenderData_S
{
   SDL_Renderer * rend;
   SDL_Rect level_viewport;
   SDL_Texture * text_terrain;
   SDL_Texture * text_character;
};

typedef struct GameTextData_S GameTextData_T;
struct GameTextData_S
{
   TTF_Font * font;
   FontText_T gold_count_text;
   ESInbox_T * inbox_goldamountchanged;
};

typedef struct GameAudioData_S GameAudioData_T;
struct GameAudioData_S
{
   Mix_Music * music;
   Mix_Chunk * pickup;
   ESInbox_T * inbox_goldamountchanged;
};

#define EVENT_INITLEVEL          1
#define EVENT_LEVELSTARTPOS      2
#define EVENT_PLAYERONGOLD       3
#define EVENT_GOLDAMOUNTCHANGED  4
#define EVENT_INPUTSTATE         5

typedef struct Event_InitLevel_S         Event_InitLevel_T;
typedef struct Event_LevelStartPos_S     Event_LevelStartPos_T;
typedef struct Event_PlayerOnGold_S      Event_PlayerOnGold_T;
typedef struct Event_GoldAmountChanged_S Event_GoldAmountChanged_T;
typedef struct Event_InputState_S        Event_InputState_T;

struct Event_InitLevel_S
{
   int level_number;
};

struct Event_LevelStartPos_S
{
   int player;
   int x;
   int y;
};

struct Event_PlayerOnGold_S
{
   int player;
   int gold_index;
};

struct Event_GoldAmountChanged_S
{
   int new_amount;
   int new_max;
   int delta;
};

struct Event_InputState_S
{
   int player;
   GameInput_PlayerKeys_T key;
   int state;
};

static void Level_SetPlayerAtStart(Level_T * level, PlayerData_T * player);

static int IsTerrainPassable(LevelTile_T * from, LevelTile_T * to);
static int IsTerrainFallable(LevelTile_T * from, LevelTile_T * to);
static int IsAllGoldColected(Level_T * level);
static void FontText_UpdateGoldCount(FontText_T * gold_count_text, int gold_left, int gold_total);

static void CheckForExit(const SDL_Event *event, int * done);


static void handle_input(const SDL_Event * event, 
                         EventSys_T * event_sys, 
                         int * done, 
                         int * game_input_flags, 
                         int * player_input_flags, 
                         SDL_Scancode * game_controls, 
                         SDL_Scancode * player1_controls);

static void handle_update(float seconds,
                          EventSys_T * event_sys, 
                          GameLevelData_T * game_level_data, 
                          GameAudioData_T * game_audio_data,
                          int * game_input_flags,  
                          PlayerData_T * player1_data, 
                          GameTextData_T * game_text_data);

static void handle_render(GameRenderData_T * game_render_data, 
                          Level_T * level, 
                          PlayerData_T * player1_data);

int main(int args, char * argc[])
{
   SDL_Window  * window;
   GameRenderData_T game_render_data;
   GameAudioData_T game_audio_data;
   SDL_Event event;
   int done;
   int prevTicks, diffTicks, nowTicks;
   float seconds;

   int i;
   int game_input_flags[e_gigk_last];

   // Font
   GameTextData_T game_text_data;

   // Music

   PlayerData_T player1_data;
   LevelSet_T levelset;
   GameLevelData_T game_level_data;

   // Settings
   GameSettings_T * game_settings;
   SDL_Scancode player1_controls[e_gipk_last];
   SDL_Scancode game_controls[e_gigk_last];

   // Event
   EventSys_T event_sys;
   Event_InitLevel_T event_initlevel;
   
   // Controller 
   SDL_GameController * game_ctrl;


   EventSys_Init(&event_sys);
   EventSys_RegisterEventType(&event_sys, EVENT_PLAYERONGOLD,      sizeof(Event_PlayerOnGold_T));
   EventSys_RegisterEventType(&event_sys, EVENT_GOLDAMOUNTCHANGED, sizeof(Event_GoldAmountChanged_T));
   EventSys_RegisterEventType(&event_sys, EVENT_INITLEVEL,         sizeof(Event_InitLevel_T));
   EventSys_RegisterEventType(&event_sys, EVENT_LEVELSTARTPOS,     sizeof(Event_LevelStartPos_T));
   EventSys_RegisterEventType(&event_sys, EVENT_INPUTSTATE,        sizeof(Event_InputState_T));

   GameSettings_Load("config.txt");
   game_settings = GameSettings_Get();

   GameInput_PopulateSDLScancodes(player1_controls, 
                                  game_settings->player1_keys.key_string, 
                                  e_gipk_last);
   GameInput_PopulateSDLScancodes(game_controls, 
                                  game_settings->game_keys, 
                                  e_gigk_last);
   
   for(i = 0; i < e_gipk_last; i++)
   {
      player1_data.input_flags[i] = 0;
   }
   player1_data.inbox_levelstartpos = EventSys_CreateInbox(&event_sys, EVENT_LEVELSTARTPOS);
   player1_data.inbox_inputstate = EventSys_CreateInbox(&event_sys, EVENT_INPUTSTATE);


   for(i = 0; i < e_gigk_last; i++)
   {
      game_input_flags[i] = 0;
   }

   LevelSet_Init(&levelset);
   LevelSet_Load(&levelset, game_settings->config.game_levelset);
   
   game_level_data.level = NULL;
   game_level_data.levelset = &levelset;
   game_level_data.level_index = 0;
   game_level_data.level = LevelSet_GetLevel(game_level_data.levelset, game_level_data.level_index);
   Level_SetPlayerAtStart(game_level_data.level, &player1_data);   
   game_level_data.inbox_playerongold = EventSys_CreateInbox(&event_sys, EVENT_PLAYERONGOLD);
   game_level_data.inbox_initlevel = EventSys_CreateInbox(&event_sys, EVENT_INITLEVEL);

   player1_data.player_state = PLAYER_STATE_NOT_MOVING;
   

   SDL_Init(SDL_INIT_EVERYTHING);   
   TTF_Init();
   Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_MOD);
   Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096);

   game_audio_data.music = Mix_LoadMUS(game_settings->config.music_background);
   printf("Loading Background Music: %s\n", game_settings->config.music_background);
   game_audio_data.pickup = Mix_LoadWAV("pickup.wav");
   game_audio_data.inbox_goldamountchanged = EventSys_CreateInbox(&event_sys, EVENT_GOLDAMOUNTCHANGED);
   //printf("pickup %p %s\n", pickup, Mix_GetError());
   Mix_VolumeChunk(game_audio_data.pickup, game_settings->raw_volume_effects);
    
   window = SDL_CreateWindow("Load Clone", 
                             SDL_WINDOWPOS_CENTERED, 
                             SDL_WINDOWPOS_CENTERED, 
                             game_settings->config.window_width,
                             game_settings->config.window_height,
                             SDL_WINDOW_SHOWN | ((game_settings->config.window_fullscreen == 1) ? SDL_WINDOW_FULLSCREEN : 0) );
   
   game_render_data.rend  = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
   

   game_render_data.level_viewport.x = MARGIN_LEFT;
   game_render_data.level_viewport.y = MARGIN_TOP;
   game_render_data.level_viewport.w = game_settings->config.window_width  - (MARGIN_LEFT + MARGIN_RIGHT);
   game_render_data.level_viewport.h = game_settings->config.window_height - (MARGIN_TOP  + MARGIN_BOTTOM);

   game_render_data.text_terrain   = SDLTools_LoadTexture(game_render_data.rend, "terrain.png");
   game_render_data.text_character = SDLTools_LoadTexture(game_render_data.rend, "character.png");
   
   game_text_data.font = TTF_OpenFont("cnr.otf", 28);
   if(game_text_data.font == NULL)
   {
      printf("Font Null\n");
   }
   FontText_Init(&game_text_data.gold_count_text, game_text_data.font, game_render_data.rend);
   FontText_SetColor(&game_text_data.gold_count_text,
                     game_settings->config.foreground_color_red,
                     game_settings->config.foreground_color_green,
                     game_settings->config.foreground_color_blue, 0xFF);

   game_text_data.inbox_goldamountchanged = EventSys_CreateInbox(&event_sys, EVENT_GOLDAMOUNTCHANGED);

   event_initlevel.level_number = 0;
   EventSys_Send(&event_sys, EVENT_INITLEVEL, &event_initlevel);



   if(SDL_NumJoysticks() >= 1 && SDL_IsGameController(0))
   {
      printf("Game Controller Name: %s\n", SDL_GameControllerNameForIndex(0));
      game_ctrl = SDL_GameControllerOpen(0);
   }
   else
   {
      printf("No Game Controller Found\n");
      game_ctrl = NULL;
   }

   Mix_FadeInMusic(game_audio_data.music, -1, 1000);
   Mix_VolumeMusic(game_settings->raw_volume_music);
   

   done = 0;
   prevTicks = SDL_GetTicks();
   while(done == 0)
   {
      while(SDL_PollEvent(&event))
      {
         handle_input(&event, 
                      &event_sys,
                      &done, 
                      game_input_flags, 
                      player1_data.input_flags, 
                      game_controls, 
                      player1_controls);
      }
      
      nowTicks = SDL_GetTicks();
      diffTicks = nowTicks - prevTicks;
      seconds = (float)diffTicks / 1000.0f;
      prevTicks = nowTicks;
      
      handle_update(seconds, 
                    &event_sys,
                    &game_level_data, 
                    &game_audio_data, 
                    game_input_flags, 
                    &player1_data, 
                    &game_text_data);
      
      SDL_SetRenderDrawColor(game_render_data.rend, 
                             game_settings->config.background_color_red, 
                             game_settings->config.background_color_green,
                             game_settings->config.background_color_blue, 0xFF);
      SDL_RenderClear( game_render_data.rend );
      SDL_RenderSetViewport(game_render_data.rend, NULL);
      
      FontText_Render(&game_text_data.gold_count_text, 10, 10);
      SDL_RenderSetViewport(game_render_data.rend, &game_render_data.level_viewport);
      handle_render(&game_render_data, game_level_data.level, &player1_data);
      SDL_RenderPresent(game_render_data.rend);
   }
   
   GameSettings_Cleanup();

   LevelSet_Destroy(&levelset);
   
   SDL_DestroyTexture(game_render_data.text_terrain);
   SDL_DestroyTexture(game_render_data.text_character);


   Mix_FreeMusic(game_audio_data.music);
   Mix_FreeChunk(game_audio_data.pickup);
   Mix_CloseAudio();
   Mix_Quit();
   FontText_Destroy(&game_text_data.gold_count_text);
   TTF_CloseFont(game_text_data.font);
   
   if(game_ctrl != NULL)
   {
      SDL_GameControllerClose(game_ctrl);
   }
   SDL_DestroyRenderer(game_render_data.rend);
   SDL_DestroyWindow(window);
   SDL_Quit();
   
   EventSys_Destroy(&event_sys);
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

#define CTRL_DEADZONE 8000
static void handle_input(const SDL_Event * event, 
                         EventSys_T * event_sys,
                         int * done, 
                         int * game_input_flags, 
                         int * player_input_flags, 
                         SDL_Scancode * game_controls, 
                         SDL_Scancode * player1_controls)
{
   size_t i;
   int key_state;
   int joy_state;
   GameInput_PlayerKeys_T pkey;
   GameInput_GameKeys_T gkey;
   Event_InputState_T event_inputstate;

   static int prev_ctrl_up    = 0;
   static int prev_ctrl_down  = 0;
   static int prev_ctrl_left  = 0;
   static int prev_ctrl_right = 0;

   CheckForExit(event, done);

   if(event->type == SDL_KEYDOWN)
   {
      key_state = 1;
   }
   else if(event->type == SDL_KEYUP)
   {
      key_state = 0;
   }
   else
   {
      key_state = 3;
   }

   if(event->type == SDL_CONTROLLERBUTTONDOWN)
   {
      joy_state = 1;
   }
   else if(event->type == SDL_CONTROLLERBUTTONUP)
   {
      joy_state = 0;
   }
   else
   {
      joy_state = 3;
   }

   if(key_state < 3)
   {
      // Check Player Keys
      pkey = e_gipk_last;
      for(i = 0; i < e_gipk_last; i ++)
      {
         if(event->key.keysym.scancode == player1_controls[i])
         {
            pkey = i;
            break;
         }
      }

      if(pkey != e_gipk_last)
      {
         event_inputstate.key    = pkey;
         event_inputstate.state  = key_state;
         event_inputstate.player = 0;
         EventSys_Send(event_sys, EVENT_INPUTSTATE, &event_inputstate);
      }

      // Check Game Keys
      gkey = e_gigk_last;
      for(i = 0; i < e_gigk_last; i ++)
      {
         if(event->key.keysym.scancode == game_controls[i])
         {
            gkey = i;
            break;
         }
      }

      if(gkey != e_gigk_last)
      {
         game_input_flags[gkey] = key_state;

      }
   }
   else if(event->type == SDL_CONTROLLERAXISMOTION && event->caxis.which == 0)
   {
      if(event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
      {
         //printf("Joy X %i\n", event->caxis.value);
         if(prev_ctrl_right == 0 && event->caxis.value > CTRL_DEADZONE)
         {
            prev_ctrl_right = 1;
            
            event_inputstate.key    = e_gipk_move_right;
            event_inputstate.state  = 1;
            event_inputstate.player = 0;
            EventSys_Send(event_sys, EVENT_INPUTSTATE, &event_inputstate);
            //printf("Right\n");
         }
         else if(prev_ctrl_right == 1 && event->caxis.value < CTRL_DEADZONE)
         {
            prev_ctrl_right = 0;
            
            event_inputstate.key    = e_gipk_move_right;
            event_inputstate.state  = 0;
            event_inputstate.player = 0;
            EventSys_Send(event_sys, EVENT_INPUTSTATE, &event_inputstate);
         }
         else if(prev_ctrl_left == 0 && event->caxis.value < -CTRL_DEADZONE)
         {
            prev_ctrl_left = 1;
            
            event_inputstate.key    = e_gipk_move_left;
            event_inputstate.state  = 1;
            event_inputstate.player = 0;
            EventSys_Send(event_sys, EVENT_INPUTSTATE, &event_inputstate);
            //printf("Left\n");
         }
         else if(prev_ctrl_left == 1 && event->caxis.value > -CTRL_DEADZONE)
         {
            prev_ctrl_left = 0;
            
            event_inputstate.key    = e_gipk_move_left;
            event_inputstate.state  = 0;
            event_inputstate.player = 0;
            EventSys_Send(event_sys, EVENT_INPUTSTATE, &event_inputstate);
         }
      }
      else if(event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
      {
         //printf("Joy Y %i\n", event->caxis.value);
         if(prev_ctrl_down == 0 && event->caxis.value > CTRL_DEADZONE)
         {
            prev_ctrl_down = 1;
            
            event_inputstate.key    = e_gipk_move_down;
            event_inputstate.state  = 1;
            event_inputstate.player = 0;
            EventSys_Send(event_sys, EVENT_INPUTSTATE, &event_inputstate);
            //printf("Down\n");
         }
         else if(prev_ctrl_down == 1 && event->caxis.value < CTRL_DEADZONE)
         {
            prev_ctrl_down = 0;
            
            event_inputstate.key    = e_gipk_move_down;
            event_inputstate.state  = 0;
            event_inputstate.player = 0;
            EventSys_Send(event_sys, EVENT_INPUTSTATE, &event_inputstate);
         }
         else if(prev_ctrl_up == 0 && event->caxis.value < -CTRL_DEADZONE)
         {
            prev_ctrl_up = 1;
            
            event_inputstate.key    = e_gipk_move_up;
            event_inputstate.state  = 1;
            event_inputstate.player = 0;
            EventSys_Send(event_sys, EVENT_INPUTSTATE, &event_inputstate);
            //printf("Up\n");
         }
         else if(prev_ctrl_up == 1 && event->caxis.value > -CTRL_DEADZONE)
         {
            prev_ctrl_up = 0;
            
            event_inputstate.key    = e_gipk_move_up;
            event_inputstate.state  = 0;
            event_inputstate.player = 0;
            EventSys_Send(event_sys, EVENT_INPUTSTATE, &event_inputstate);
         }
      }

   }
   else if(joy_state < 3 && event->cbutton.which == 0)
   {
      if(event->cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER)
      {
         event_inputstate.key    = e_gipk_dig_left;
         event_inputstate.state  = joy_state;
         event_inputstate.player = 0;
         EventSys_Send(event_sys, EVENT_INPUTSTATE, &event_inputstate);
      }
      else if(event->cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
      {
         event_inputstate.key    = e_gipk_dig_right;
         event_inputstate.state  = joy_state;
         event_inputstate.player = 0;
         EventSys_Send(event_sys, EVENT_INPUTSTATE, &event_inputstate);
      }
   }
}

static void handle_update_level(float seconds,
                                EventSys_T * event_sys,
                                GameLevelData_T * game_level_data)
{

   Level_T * next_level;
   Event_LevelStartPos_T     event_levelstartpos;   
   Event_GoldAmountChanged_T event_goldamountchanged;
   Event_InitLevel_T         * list_initlevel;
   Event_PlayerOnGold_T      * list_playerongold;
   size_t count, i;
   
   
   list_initlevel = ESInbox_Get(game_level_data->inbox_initlevel, &count, NULL);
   for(i = 0; i < count; i ++)
   {

      next_level = LevelSet_GetLevel(game_level_data->levelset, list_initlevel[i].level_number);
      if(next_level != NULL)
      {
         game_level_data->level_index = list_initlevel[i].level_number;
         game_level_data->level = next_level;
         Level_Restart(game_level_data->level);

         event_levelstartpos.player = 0;
         Level_GetStartSpot(game_level_data->level,
                            &event_levelstartpos.x,
                            &event_levelstartpos.y);
         EventSys_Send(event_sys, EVENT_LEVELSTARTPOS, &event_levelstartpos);

         event_goldamountchanged.delta = 0;         
         event_goldamountchanged.new_amount = Level_GetGoldCount(game_level_data->level, &event_goldamountchanged.new_max);
         EventSys_Send(event_sys, EVENT_GOLDAMOUNTCHANGED, &event_goldamountchanged);
      

      }
   }

   list_playerongold = ESInbox_Get(game_level_data->inbox_playerongold, &count, NULL);
   event_goldamountchanged.delta = 0;
   for(i = 0; i < count; i++)
   {
      Level_RemoveGold(game_level_data->level, list_playerongold[i].gold_index);
      event_goldamountchanged.delta --;
   }
   event_goldamountchanged.new_amount = Level_GetGoldCount(game_level_data->level, &event_goldamountchanged.new_max);
   if(event_goldamountchanged.delta != 0)
   {
      EventSys_Send(event_sys, EVENT_GOLDAMOUNTCHANGED, &event_goldamountchanged);
   }
   Level_Update(game_level_data->level, seconds);

}

static void handle_update_player(float seconds,
                                 EventSys_T * event_sys,
                                 PlayerData_T * player1_data)
{
   Event_LevelStartPos_T * list_levelstartpos;
   size_t count, i;

   list_levelstartpos = ESInbox_Get(player1_data->inbox_levelstartpos, &count, NULL);
   for(i = 0; i < count; i++)
   {
      if(list_levelstartpos[i].player == 0)
      {

         player1_data->grid_p.x = list_levelstartpos[i].x;
         player1_data->grid_p.y = list_levelstartpos[i].y;
         player1_data->next_grid_p.x = player1_data->grid_p.x;
         player1_data->next_grid_p.y = player1_data->grid_p.y;
         player1_data->player_state = PLAYER_STATE_DEATH;
      }
   }
}

static void handle_update_text(float seconds,
                               EventSys_T * event_sys,
                               GameTextData_T * game_text_data)
{

   size_t count;
   Event_GoldAmountChanged_T * list_goldamountchanged;

   list_goldamountchanged = ESInbox_Get(game_text_data->inbox_goldamountchanged, &count, NULL);
   if(count > 0)
   {
      FontText_UpdateGoldCount(&game_text_data->gold_count_text,
                               list_goldamountchanged[0].new_amount,
                               list_goldamountchanged[0].new_max);
   }

}

static void handle_update_audio(float seconds,
                                EventSys_T * event_sys,
                                GameAudioData_T * game_audio_data)
{
   size_t count;
   Event_GoldAmountChanged_T * list_goldamountchanged;

   list_goldamountchanged = ESInbox_Get(game_audio_data->inbox_goldamountchanged, &count, NULL);
   if(count > 0)
   {
      if(list_goldamountchanged[0].delta < 0)
      {
         Mix_PlayChannel(-1, game_audio_data->pickup, 0);
      }
   }
}


static void handle_update(float seconds,
                          EventSys_T * event_sys, 
                          GameLevelData_T * game_level_data, 
                          GameAudioData_T * game_audio_data,
                          int * game_input_flags, 
                          PlayerData_T * player1_data, 
                          GameTextData_T * game_text_data)
{
   int cmd_dig_left_valid, cmd_dig_right_valid;
   int cmd_move_left_valid, cmd_move_right_valid;
   int cmd_move_up_valid, cmd_move_down_valid, cmd_let_go_valid, cmd_fall_valid;
   LevelTile_T player_current_tile, player_desired_tile, dig_desired_tile;
   LevelTile_T dig_above_tile, player_below_tile;
   int all_gold_colected;
   static int restart_key_prev = 0;
   Event_PlayerOnGold_T event_playerongold;
   Event_InputState_T * list_inputstate;
   size_t count, i;
   Event_InitLevel_T event_initlevel;


   list_inputstate = ESInbox_Get(player1_data->inbox_inputstate, &count, NULL);
   for(i = 0; i < count; i++)
   {
      if(list_inputstate[i].player == 0)
      {
         player1_data->input_flags[list_inputstate[i].key] = list_inputstate[i].state;
      }
   }
    
   Level_QueryTile(game_level_data->level, POS_SPLIT(player1_data->grid_p, 0, 0), &player_current_tile);
   all_gold_colected = IsAllGoldColected(game_level_data->level);
   if(all_gold_colected == 1 && player_current_tile.terrain_type == TMAP_TILE_DOOR)
   {
      // TODO: WIN!!
      player1_data->player_state = PLAYER_STATE_WIN;

   }
   else if(player1_data->player_state != PLAYER_STATE_DEATH)
   {
      if(player_current_tile.out_of_range == 1 || 
         (player_current_tile.terrain_type == TMAP_TILE_DIRT && player_current_tile.has_hole == 0))
      {
         player1_data->player_state = PLAYER_STATE_DEATH;
         // Dec Lifes Here?
      }
   }


   
   if(player1_data->player_state == PLAYER_STATE_NOT_MOVING)
   {
      // Check for valid commands/behavoirs
      
      cmd_dig_left_valid   = 0;
      cmd_dig_right_valid  = 0;
      cmd_move_left_valid  = 0;
      cmd_move_right_valid = 0;
      cmd_move_up_valid    = 0;
      cmd_move_down_valid  = 0;
      cmd_let_go_valid     = 0;
      cmd_fall_valid       = 0;
      

      Level_QueryTile(game_level_data->level, POS_SPLIT(player1_data->grid_p, 0,  1), &player_below_tile);


      if(player_current_tile.gold_index >= 0) // Remove Gold if on Gold
      {
         event_playerongold.player = 0;
         event_playerongold.gold_index = player_current_tile.gold_index;
         EventSys_Send(event_sys, EVENT_PLAYERONGOLD, &event_playerongold); 
      }

      if(IsTerrainFallable(&player_current_tile, &player_below_tile) == 1) // Falling
      {
         cmd_fall_valid = 1;
      }
      if(player1_data->input_flags[e_gipk_dig_left] == 1) // Dig Left
      {
         Level_QueryTile(game_level_data->level, POS_SPLIT(player1_data->grid_p, -1, 1), &dig_desired_tile);
         Level_QueryTile(game_level_data->level, POS_SPLIT(player1_data->grid_p, -1, 0), &dig_above_tile);
         if(dig_desired_tile.terrain_type == TMAP_TILE_DIRT && 
            (dig_above_tile.terrain_type == TMAP_TILE_AIR || dig_above_tile.has_hole == 1))
         {
            cmd_dig_left_valid = 1;
         }
      }
      if(player1_data->input_flags[e_gipk_dig_right] == 1) // Dig Right
      {
         Level_QueryTile(game_level_data->level, POS_SPLIT(player1_data->grid_p, 1, 1), &dig_desired_tile);
         Level_QueryTile(game_level_data->level, POS_SPLIT(player1_data->grid_p, 1, 0), &dig_above_tile);
         if(dig_desired_tile.terrain_type == TMAP_TILE_DIRT && 
            (dig_above_tile.terrain_type == TMAP_TILE_AIR || dig_above_tile.has_hole == 1))
         {
            cmd_dig_right_valid = 1;
         }
      }
      if(player1_data->input_flags[e_gipk_move_left] == 1) // Move Left
      {
         Level_QueryTile(game_level_data->level, POS_SPLIT(player1_data->grid_p, -1, 0), &player_desired_tile);
         if(IsTerrainPassable(&player_current_tile, &player_desired_tile) == 1)
         {
            cmd_move_left_valid = 1;
         }
      }
      if(player1_data->input_flags[e_gipk_move_right] == 1) // Move Right
      {
         Level_QueryTile(game_level_data->level, POS_SPLIT(player1_data->grid_p, 1, 0), &player_desired_tile);
         if(IsTerrainPassable(&player_current_tile, &player_desired_tile) == 1)
         {
            cmd_move_right_valid = 1;
         }
      }
      if(player1_data->input_flags[e_gipk_move_up] == 1) // Climb Up
      {
         Level_QueryTile(game_level_data->level, POS_SPLIT(player1_data->grid_p, 0, -1), &player_desired_tile);
         if(IsTerrainPassable(&player_current_tile, &player_desired_tile) == 1 &&
            player_current_tile.terrain_type == TMAP_TILE_LADDER)
         {
            cmd_move_up_valid = 1;
         }
      }
      if(player1_data->input_flags[e_gipk_move_down] == 1) // Climb Down or Fall
      {
         Level_QueryTile(game_level_data->level, POS_SPLIT(player1_data->grid_p, 0, 1), &player_desired_tile);
         if(IsTerrainPassable(&player_current_tile, &player_desired_tile) == 1)
         {
            if(player_current_tile.terrain_type == TMAP_TILE_BAR)
            {
               cmd_let_go_valid = 1;
            }
            else
            {
               cmd_move_down_valid = 1;
            }
         }
      }

      // Compute Next Position and state based on commands
      // Order
      // 1. Falling
      // 2. Digging
      // 3. Climbing
      // 4. Letting Go
      // 5. Horisontal Movement

      if(cmd_fall_valid == 1)
      {
         player1_data->player_state = PLAYER_STATE_FALLING;
         player1_data->next_grid_p.x = player1_data->grid_p.x;
         player1_data->next_grid_p.y = player1_data->grid_p.y + 1;
      }
      else if(cmd_dig_left_valid == 1 && cmd_dig_right_valid == 0)
      {
         player1_data->player_state = PLAYER_STATE_DIGGING;
         player1_data->next_grid_p.x = player1_data->grid_p.x;
         player1_data->next_grid_p.y = player1_data->grid_p.y;

         Level_AddDigSpot(game_level_data->level, 
                          player1_data->grid_p.x - 1, 
                          player1_data->grid_p.y + 1);
      }
      else if(cmd_dig_right_valid == 1 && cmd_dig_left_valid == 0)
      {
         player1_data->player_state = PLAYER_STATE_DIGGING;
         player1_data->next_grid_p.x = player1_data->grid_p.x;
         player1_data->next_grid_p.y = player1_data->grid_p.y;

         Level_AddDigSpot(game_level_data->level, 
                          player1_data->grid_p.x + 1, 
                          player1_data->grid_p.y + 1);
      }
      else if(cmd_move_up_valid == 1 && cmd_move_down_valid == 0)
      {
         player1_data->player_state = PLAYER_STATE_MOVING;
         player1_data->next_grid_p.x = player1_data->grid_p.x;
         player1_data->next_grid_p.y = player1_data->grid_p.y - 1;
      }
      else if(cmd_move_down_valid == 1 && cmd_move_up_valid == 0)
      {
         player1_data->player_state = PLAYER_STATE_MOVING;
         player1_data->next_grid_p.x = player1_data->grid_p.x;
         player1_data->next_grid_p.y = player1_data->grid_p.y + 1;
      }
      else if(cmd_let_go_valid == 1)
      {
         player1_data->player_state = PLAYER_STATE_FALLING;
         player1_data->next_grid_p.x = player1_data->grid_p.x;
         player1_data->next_grid_p.y = player1_data->grid_p.y + 1;
      }
      else if(cmd_move_left_valid == 1 && cmd_move_right_valid == 0)
      {
         player1_data->player_state = PLAYER_STATE_MOVING;
         player1_data->next_grid_p.x = player1_data->grid_p.x - 1;
         player1_data->next_grid_p.y = player1_data->grid_p.y;
      }
      else if(cmd_move_right_valid == 1 && cmd_move_left_valid == 0)
      {
         player1_data->player_state = PLAYER_STATE_MOVING;
         player1_data->next_grid_p.x = player1_data->grid_p.x + 1;
         player1_data->next_grid_p.y = player1_data->grid_p.y;
      }

      if(player1_data->player_state != PLAYER_STATE_NOT_MOVING)
      {
         player1_data->move_timer = 0;


         switch(player1_data->player_state)
         {
            case PLAYER_STATE_DIGGING: player1_data->move_timeout = DIG_TIMEOUT;  break;
            case PLAYER_STATE_MOVING:  player1_data->move_timeout = MOVE_TIMEOUT; break;
            case PLAYER_STATE_FALLING: player1_data->move_timeout = FALL_TIMEOUT; break;
            default:                   player1_data->move_timeout = 0;            break;
         }
      }
   }
   else if(player1_data->player_state == PLAYER_STATE_MOVING ||
           player1_data->player_state == PLAYER_STATE_FALLING ||
           player1_data->player_state == PLAYER_STATE_DIGGING)
   {
      player1_data->move_timer += seconds;
      if(player1_data->move_timer >= player1_data->move_timeout)
      {
         player1_data->player_state = PLAYER_STATE_NOT_MOVING;
         player1_data->grid_p.x = player1_data->next_grid_p.x;
         player1_data->grid_p.y = player1_data->next_grid_p.y;
      }
   }
   else if(player1_data->player_state == PLAYER_STATE_DEATH)
   {
      if(player1_data->input_flags[e_gipk_dig_left]   == 1 ||
         player1_data->input_flags[e_gipk_dig_right]  == 1 ||
         player1_data->input_flags[e_gipk_move_left]  == 1 ||
         player1_data->input_flags[e_gipk_move_right] == 1 ||
         player1_data->input_flags[e_gipk_move_up]    == 1 ||
         player1_data->input_flags[e_gipk_move_down]  == 1)
      {
         player1_data->player_state = PLAYER_STATE_NOT_MOVING;
         Level_SetPlayerAtStart(game_level_data->level, player1_data);
         //printf("ComeAlive!\n");
      }
   }

   if(game_input_flags[e_gigk_restart_level] == 0 && restart_key_prev == 1)
   {
      event_initlevel.level_number = game_level_data->level_index;
      EventSys_Send(event_sys, EVENT_INITLEVEL, &event_initlevel);
   }
   restart_key_prev = game_input_flags[e_gigk_restart_level];

   if(player1_data->player_state == PLAYER_STATE_WIN)
   {
      // We won, so move the next level
      event_initlevel.level_number = game_level_data->level_index + 1;
      EventSys_Send(event_sys, EVENT_INITLEVEL, &event_initlevel);
   }
   // Level update
   handle_update_level(seconds, event_sys, game_level_data);
  
   // Player Update
   handle_update_player(seconds, event_sys, player1_data);

   // Text Update
   handle_update_text(seconds, event_sys, game_text_data);

   // Audio Update
   handle_update_audio(seconds, event_sys, game_audio_data);
}

static void handle_render(GameRenderData_T * game_render_data,
                          Level_T * level, 
                          PlayerData_T * player1_data)
{
   Pos2D_T draw_loc;
   Pos2D_T diff;
   float move_percent;
   int show;
   int center_x, center_y;


   center_x = (game_render_data->level_viewport.w / 2.0f) - (TILE_WIDTH  / 2.0f);
   center_y = (game_render_data->level_viewport.h / 2.0f) - (TILE_HEIGHT / 2.0f);

   

   show = 1;
   switch(player1_data->player_state)
   {
   case PLAYER_STATE_NOT_MOVING:
  
      draw_loc.x = player1_data->grid_p.x * TILE_WIDTH;
      draw_loc.y = player1_data->grid_p.y * TILE_HEIGHT;
      break;
   case PLAYER_STATE_FALLING:
   case PLAYER_STATE_DIGGING:
   case PLAYER_STATE_MOVING:
   
      diff.x = (player1_data->next_grid_p.x - player1_data->grid_p.x) * TILE_WIDTH;
      diff.y = (player1_data->next_grid_p.y - player1_data->grid_p.y) * TILE_HEIGHT;
      move_percent = player1_data->move_timer / player1_data->move_timeout;
      draw_loc.x = (player1_data->grid_p.x * TILE_WIDTH) + 
                   (int)(diff.x * move_percent);
      draw_loc.y = (player1_data->grid_p.y * TILE_HEIGHT) + 
                   (int)(diff.y * move_percent);
      break;

   case PLAYER_STATE_DEATH:
      show = 0;
      draw_loc.x = player1_data->grid_p.x * TILE_WIDTH;
      draw_loc.y = player1_data->grid_p.y * TILE_HEIGHT;
      break;
   case PLAYER_STATE_WIN:
      show = 0; // Sure why not?
      draw_loc.x = player1_data->grid_p.x * TILE_WIDTH;
      draw_loc.y = player1_data->grid_p.y * TILE_HEIGHT;
      break;
   default:
      show = 0;
      draw_loc.x = 0;
      draw_loc.y = 0;
      break;
   }

   
   Level_Render(level, 
                game_render_data->rend, 
                center_x - draw_loc.x, 
                center_y - draw_loc.y,  
                game_render_data->text_terrain);

   if(show == 1)
   {
      SDLTools_DrawSubimage(game_render_data->rend, 
                            game_render_data->text_character, IMGID_GUY, 
                            center_x, center_y); 
   }
 
}

static int IsTerrainPassable(LevelTile_T * from, LevelTile_T * to)
{
   int result;
   if(to->out_of_range == 0 && 
     (
       (to->terrain_type == TMAP_TILE_DIRT && to->has_hole == 1) || 
       (to->terrain_type != TMAP_TILE_DIRT)
     ))
   {
      result = 1;
   }
   else
   {
      result = 0;
   }
   return result;
}

static int IsTerrainFallable(LevelTile_T *  from, LevelTile_T *  to)
{
   int result;
   if((to->terrain_type == TMAP_TILE_AIR || to->terrain_type == TMAP_TILE_BAR || to->has_hole == 1 || to->terrain_type == TMAP_TILE_DOOR) && 
      from->terrain_type != TMAP_TILE_LADDER && from->terrain_type != TMAP_TILE_BAR && to->out_of_range == 0)
   {
      result = 1;      
   }
   else
   {
      result = 0;
   }
   return result;
}


static int IsAllGoldColected(Level_T * level)
{
   int all_gold_colected;
   int gold_count;
 
   gold_count = Level_GetGoldCount(level, NULL);
   if(gold_count > 0)
   {
      all_gold_colected = 0;
   }
   else
   {
      all_gold_colected = 1;
   }
   return all_gold_colected;
}

static void FontText_UpdateGoldCount(FontText_T * gold_count_text, int gold_left, int gold_total)
{
   char buffer[255];

   sprintf(buffer, "Gold %i/%i", gold_left, gold_total);
   FontText_SetString(gold_count_text, buffer);
}


static void Level_SetPlayerAtStart(Level_T * level, PlayerData_T * player)
{
   Level_GetStartSpot(level, &player->grid_p.x, &player->grid_p.y);
   player->next_grid_p.x = player->grid_p.x;
   player->next_grid_p.y = player->grid_p.y;
}

