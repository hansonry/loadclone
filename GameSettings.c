#include <stdio.h>
#include "GameInput.h"
#include "GameConfigData.h"
#include "GameSettings.h"
#include "ConfigLoader.h"
#include "SDLInclude.h"

#include "GameConfigData.inl"


static ConfigLoader_T loader;
static GameSettings_T settings;
static int settings_loaded = 0;

static void GameSettings_ParseFile(void);

void GameSettings_Load(const char * filename)
{
   ConfigLoader_LoadFilename(&loader, filename);
   settings_loaded = 1;
   GameSettings_ParseFile();
}

GameSettings_T * GameSettings_Get(void)
{
   GameSettings_T * result;
   if(settings_loaded == 1)
   {
      result = &settings;
   }
   else
   {
      result = NULL;
   }
   return result;
}


void GameSettings_Cleanup(void)
{
   if(settings_loaded == 1)
   {
      settings_loaded = 0;
      ConfigLoader_Destroy(&loader);
   }
}

static void GameSettings_ParseFile(void)
{
   PopulateData(&loader, &settings.config);

   // Fill key data

   settings.game_keys[e_gigk_restart_level] = settings.config.controls_game_restart_level;
   // Fill player key data
   settings.player1_keys.key_string[e_gipk_move_up]    = settings.config.controls_player1_move_up;
   settings.player1_keys.key_string[e_gipk_move_down]  = settings.config.controls_player1_move_down;
   settings.player1_keys.key_string[e_gipk_move_left]  = settings.config.controls_player1_move_left;
   settings.player1_keys.key_string[e_gipk_move_right] = settings.config.controls_player1_move_right;
   settings.player1_keys.key_string[e_gipk_dig_left]   = settings.config.controls_player1_dig_left;
   settings.player1_keys.key_string[e_gipk_dig_right]  = settings.config.controls_player1_dig_right;


   // Compute actual music volumes based on master
   settings.raw_volume_music = (int)(MIX_MAX_VOLUME * (
                               (settings.config.volume_master / 100.0f) *
                               (settings.config.volume_music  / 100.0f)
                               ));

   settings.raw_volume_effects = (int)(MIX_MAX_VOLUME * (
                                 (settings.config.volume_master  / 100.0f) *
                                 (settings.config.volume_effects / 100.0f)
                                 ));

}



