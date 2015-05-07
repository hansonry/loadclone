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
#ifndef __EVENTSYS_H__
#define __EVENTSYS_H__

typedef struct EventSys_S EventSys_T;
typedef struct ESInbox_S ESInbox_T;

struct EventSys_S
{
   ArrayList_T event_type_list;
};

struct ESInbox_S
{
   size_t event_size;
   int list_index;
   ArrayList_T event_list[2];
};


void EventSys_Init(EventSys_T * event_sys);
void EventSys_Destroy(EventSys_T * event_sys);

void EventSys_RegisterEventType(EventSys_T * event_sys, int event_type, size_t event_size);

ESInbox_T * EventSys_CreateInbox(EventSys_T * event_sys, int event_type);

void EventSys_Send(EventSys_T * event_sys, int event_type, void * event_data);

void * ESInbox_Get(ESInbox_T * inbox, size_t * count, size_t * event_size);
void ESInbox_Add(ESInbox_T * inbox, void * event_data);




#endif // __EVENTSYS_H__

