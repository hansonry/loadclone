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

#include <stdlib.h>
#include <string.h>
#include "ArrayList.h"
#include "EventSys.h"

typedef struct ESType_S ESType_T;
struct ESType_S
{
   ArrayList_T inbox_list;
   size_t event_size;
   int event_type;
};

static ESType_T * EventSys_FindType(EventSys_T * event_sys, int event_type)
{
   size_t i, count;
   ESType_T * loop, * result;
   result = NULL;
   loop = ArrayList_Get(&event_sys->event_type_list, &count, NULL);
   for(i = 0; i < count; i++)
   {
      if(loop[i].event_type == event_type)
      {
         result = &loop[i];
         break;
      }
   }
   return result;
}


void EventSys_Init(EventSys_T * event_sys)
{
   ArrayList_Init(&event_sys->event_type_list, sizeof(ESType_T), 0);
}

void EventSys_Destroy(EventSys_T * event_sys)
{
   size_t type_count, i1, inbox_count, i2;
   ESType_T * type_list;
   ESInbox_T * inbox_list;

   type_list = ArrayList_Get(&event_sys->event_type_list, &type_count, NULL);
   for(i1 = 0; i1 < type_count; i1++)
   {
      inbox_list = ArrayList_Get(&type_list[i1].inbox_list, &inbox_count, NULL);

      for(i2 = 0; i2 < inbox_count; i2++)
      {
         ArrayList_Destroy(&inbox_list[i2].event_list[0]);
         ArrayList_Destroy(&inbox_list[i2].event_list[1]);
      }
      ArrayList_Destroy(&type_list[i1].inbox_list);
   }

   ArrayList_Destroy(&event_sys->event_type_list);
   
}

void EventSys_RegisterEventType(EventSys_T * event_sys, int event_type, size_t event_size)
{
   ESType_T * type;

   type = ArrayList_Add(&event_sys->event_type_list, NULL);
   type->event_size = event_size;
   type->event_type = event_type;
   ArrayList_Init(&type->inbox_list, sizeof(ESInbox_T), 0);
}

ESInbox_T * EventSys_CreateInbox(EventSys_T * event_sys, int event_type)
{
   ESType_T * type;
   ESInbox_T * inbox;

   type = EventSys_FindType(event_sys, event_type);
   if(type == NULL)
   {
      inbox = NULL;
   }
   else
   {
      inbox = ArrayList_Add(&type->inbox_list, NULL);
      inbox->event_size = type->event_size;
      inbox->list_index = 0;
      ArrayList_Init(&inbox->event_list[0], inbox->event_size, 0);
      ArrayList_Init(&inbox->event_list[1], inbox->event_size, 0);
   }
   return inbox;
}

void EventSys_Send(EventSys_T * event_sys, int event_type, void * event_data)
{
   size_t i, count;
   ESType_T * type;
   ESInbox_T * inbox_list;

   type = EventSys_FindType(event_sys, event_type);
   if(type != NULL)
   {
      inbox_list = ArrayList_Get(&type->inbox_list, &count, NULL);
      for(i = 0; i < count; i++)
      {
         ESInbox_Add(&inbox_list[i], event_data);
      }
   }
}

void * ESInbox_Get(ESInbox_T * inbox, size_t * count, size_t * event_size)
{
   void * mem;

   mem = ArrayList_Get(&inbox->event_list[inbox->list_index], count, event_size);

   if(inbox->list_index == 0)
   {
      inbox->list_index = 1;
   }
   else
   {
      inbox->list_index = 0;
   }

   ArrayList_Clear(&inbox->event_list[inbox->list_index]);
   return mem;
}
void ESInbox_Add(ESInbox_T * inbox, void * event_data)
{
   void * mem;

   mem = ArrayList_Add(&inbox->event_list[inbox->list_index], NULL);
   memcpy(mem, event_data, inbox->event_size);
}



