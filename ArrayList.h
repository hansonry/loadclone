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
#ifndef __ARRAYLIST_H__
#define __ARRAYLIST_H__


typedef struct ArrayList_S ArrayList_T;

struct ArrayList_S
{
   void   * array;
   size_t   count;
   size_t   size;
   size_t   element_size;
   size_t   grow_by;
};

void   ArrayList_Init(ArrayList_T * list, size_t element_size, size_t grow_by);
void   ArrayList_Destroy(ArrayList_T * list);

void * ArrayList_Add(ArrayList_T * list, size_t * new_index);
void   ArrayList_AddArray(ArrayList_T * list, void * array, size_t count);

void * ArrayList_Insert(ArrayList_T * list, size_t before_index);
void   ArrayList_InsertArray(ArrayList_T * list, size_t before_index, void * array, size_t count);

void * ArrayList_Get(const ArrayList_T * list, size_t * count, size_t * element_size);
void * ArrayList_GetCopy(const ArrayList_T * list, size_t * count, size_t * element_size);

void * ArrayList_GetIndex(const ArrayList_T * list, size_t index);
void   ArrayList_Swap(ArrayList_T * list, size_t index1, size_t index2);

void   ArrayList_Remove(ArrayList_T * list, size_t index);
void   ArrayList_Clear(ArrayList_T * list);



#endif // __ARRAYLIST_H__

