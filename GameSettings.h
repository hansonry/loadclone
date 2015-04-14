#ifndef __GAMESETTINGS_H__
#define __GAMESETTINGS_H__

typedef struct GameSettings_S            GameSettings_T;
typedef struct GameSettings_PlayerKeys_S GameSettings_PlayerKeys_T;

struct GameSettings_PlayerKeys_S
{
   const char * key_string[e_gipk_last];
};
struct GameSettings_S
{
   int window_width;
   int window_height;
   int window_fullscreen; // boolean
   int background_color_r;
   int background_color_g;
   int background_color_b;
   int foreground_color_r;
   int foreground_color_g;
   int foreground_color_b;
   const char * levelset_filename;
   const char * game_key_restart_level;
   GameSettings_PlayerKeys_T player1_keys;
   const char * game_keys[e_gigk_last];
};


void GameSettings_Load(const char * filename);

GameSettings_T * GameSettings_Get(void);

void GameSettings_Cleanup(void);

#endif // __GAMESETTINGS_H__

