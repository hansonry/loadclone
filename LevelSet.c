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


#include "GlobalData.h"
#include "ArrayList.h"
#include "Pos2D.h"
#include "Level.h"
#include "LevelSet.h"

void LevelSet_Init(LevelSet_T * levelset)
{
   ArrayList_Init(&levelset->level_list, sizeof(Level_T), 0);
}

void LevelSet_Destroy(LevelSet_T * levelset)
{
   size_t i, size;
   Level_T * level;

   level = ArrayList_Get(&levelset->level_list, &size, NULL);
   for(i = 0; i < size; i++)
   {
      Level_Destroy(&level[i]);
   }

   ArrayList_Destroy(&levelset->level_list);

}


#define LINE_BUFFER_SIZE 255
void LevelSet_Load(LevelSet_T * levelset, const char * filename)
{
   FILE * file;
   char buffer [LINE_BUFFER_SIZE];
   char * bp;
   Level_T * level;
   

   ArrayList_Clear(&levelset->level_list);
   
   file = fopen(filename, "r");

   if(file == NULL)
   {
      printf("Error: LevelSet could not find the levelset file named %s\n", filename);
   }
   else
   {
      while(fgets(buffer, LINE_BUFFER_SIZE, file) != NULL)
      {
         if(buffer[0] != '\r' && 
            buffer[0] != '\n' && 
            buffer[0] != '\t' &&
            buffer[0] != '#'  && 
            buffer[0] != ' ')
         {
            // Chop off ending characters
            bp = buffer;
            while((*bp) != '\0')
            {
               if((*bp) == '\r' || (*bp) == '\n')
               {
                  (*bp) = '\0';
               }
               bp ++;
            }
            // Load and Add Level to the List
            level = ArrayList_Add(&levelset->level_list, NULL);
            Level_Init(level);
            Level_Load(level, buffer);

            //printf("Found \"%s\"\n", buffer);
         }
      }

      fclose(file);
   }


}

Level_T * LevelSet_GetAll(LevelSet_T * levelset, size_t * size)
{
   return ArrayList_Get(&levelset->level_list, size, NULL);
}



