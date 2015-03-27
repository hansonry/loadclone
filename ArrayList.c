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

#define DEFAULT_GROW_BY 64
typedef unsigned char uint8_t;


void   ArrayList_Init(ArrayList_T * list, size_t element_size, size_t grow_by)
{
   size_t new_size;
   if(grow_by > 0)
   {
      list->grow_by = grow_by;
   }
   else
   {
      list->grow_by = DEFAULT_GROW_BY;
   }
   list->element_size = element_size;
   
   new_size = list->grow_by;
   list->array = malloc(list->element_size * new_size);
   list->count = 0;
   list->size  = new_size;
}

void   ArrayList_Destroy(ArrayList_T * list)
{
   free(list->array);
   list->array        = NULL;
   list->count        = 0;
   list->size         = 0;
   list->grow_by      = 0;
   list->element_size = 0;
}

void * ArrayList_Add(ArrayList_T * list, size_t * new_index)
{
   size_t new_size;
   size_t local_new_index;
   if(list->count >= list->size)
   {
      new_size = list->size + list->grow_by;
      list->array = realloc(list->array, list->element_size * new_size);
      list->size  = new_size;
   }
   
   local_new_index = list->count;
   list->count ++;
   
   if(new_index != NULL)
   {
      (*new_index) = local_new_index;
   }
   
   return (uint8_t *)list->array + (list->element_size * local_new_index); 
}

void   ArrayList_AddArray(ArrayList_T * list, void * array, size_t count)
{
   size_t new_size;
   uint8_t * start;
   if(count > 0)
   {  
      if((list->count + count) >= list->size)
      {
         new_size = list->count + count + list->grow_by;
         list->array = realloc(list->array, list->element_size * new_size);
         list->size  = new_size;      
      }
      
      start = (uint8_t*)list->array + (list->count * list->element_size);
      memcpy(start, array, count * list->element_size);
      list->count = list->count + count;
   }
   
}

void * ArrayList_Insert(ArrayList_T * list, size_t before_index)
{
   size_t new_size;
   size_t i;
   uint8_t * loop;
   if(list->count >= list->size)
   {
      new_size = list->size + list->grow_by;
      list->array = realloc(list->array, list->element_size * new_size);
      list->size  = new_size;
   }
   
   if(before_index < list->count)
   {
      loop = (uint8_t *)list->array + (list->element_size * (list->count - 1));
      for(i = list->count - 1; i > before_index; i --)
      {
         memcpy(loop + list->element_size, loop, list->element_size);
         loop = loop - list->element_size;
      }
   
      memcpy(loop + list->element_size, loop, list->element_size);
   }
   else
   {
      loop = (uint8_t *)list->array + (list->element_size * list->count);
   }

   list->count ++;
   return loop;
}

void   ArrayList_InsertArray(ArrayList_T * list, size_t before_index, void * array, size_t count)
{
   size_t new_size;
   size_t i;
   uint8_t * from, *to, * start;
   if(count > 0)
   {
      if((list->count + count) >= list->size)
      {
         new_size = list->count + count + list->grow_by;
         list->array = realloc(list->array, list->element_size * new_size);
         list->size  = new_size;      
      }
      
      if(before_index < list->count)
      {
         start = (uint8_t*)list->array + (before_index * list->element_size);
         from  = (uint8_t*)list->array + ((list->count - 1) * list->element_size);
         to    = (uint8_t*)list->array + ((list->count + count - 1)  * list->element_size);
         for(i = before_index; i < list->count; i++)
         {
            memcpy(to, from, list->element_size);
            to   = to   - list->element_size;
            from = from - list->element_size;
         }
      }
      else
      {
         start = (uint8_t*)list->array + (list->count * list->element_size);
      }
      
      memcpy(start, array, count * list->element_size);
      list->count = list->count + count;
   }

}

void * ArrayList_Get(const ArrayList_T * list, size_t * count, size_t * element_size)
{
   if(count != NULL)
   {
      (*count) = list->count;
   }
   
   if(element_size != NULL)
   {
      (*element_size) = list->element_size;
   }
   
   return list->array;
}


void * ArrayList_GetCopy(const ArrayList_T * list, size_t * count, size_t * element_size)
{
   void * copy;
   size_t byte_size;
   if(count != NULL)
   {
      (*count) = list->count;
   }
   
   if(element_size != NULL)
   {
      (*element_size) = list->element_size;
   }
   
   byte_size = list->count * list->element_size;
   copy = malloc(byte_size);
   memcpy(copy, list->array, byte_size);
   return copy;
   
}

void * ArrayList_GetIndex(const ArrayList_T * list, size_t index)
{
   return (uint8_t *)list->array + (list->element_size * index);
}

void   ArrayList_Swap(ArrayList_T * list, size_t index1, size_t index2)
{
   uint8_t * temp;
   uint8_t * ptr1, * ptr2;
   
   if((index1 < list->count) && (index2 < list->count) && (index1 != index2))
   {
      ptr1 = (uint8_t *)list->array + (list->element_size * index1);
      ptr2 = (uint8_t *)list->array + (list->element_size * index2);
      temp = malloc(list->element_size);
      memcpy(temp, ptr1, list->element_size);
      memcpy(ptr1, ptr2, list->element_size);
      memcpy(ptr2, temp, list->element_size);
      free(temp); 
   }
}

void   ArrayList_Remove(ArrayList_T * list, size_t index)
{
   uint8_t * loop, * next;
   size_t i;
   
   if(index < list->count)
   {
      loop = (uint8_t*)list->array + (index * list->element_size);
      list->count --;
      for(i = index; i < list->count; i++)
      {
         next = loop + list->element_size;
         memcpy(loop, next, list->element_size);
         loop = next;     
      }
   }
}

void   ArrayList_Clear(ArrayList_T * list)
{
   list->count = 0;
}




