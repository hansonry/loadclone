#ifndef __GAMESETTINGS_H__
#define __GAMESETTINGS_H__

typedef struct GameSettings_S GameSettings_T;
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
};


void GameSettings_Load(const char * filename);

GameSettings_T * GameSettings_Get(void);

void GameSettings_Cleanup(void);

#endif // __GAMESETTINGS_H__

