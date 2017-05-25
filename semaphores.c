//////////////////////////////////////////////////////////////////////////////////////
//                                                                                  //
//  Copyright (c) 2016-2017 Leonardo Consoni <consoni_2519@hotmail.com>             //
//                                                                                  //
//  This file is part of Simple Multithreading.                                     //
//                                                                                  //
//  Simple Multithreading is free software: you can redistribute it and/or modify   //
//  it under the terms of the GNU Lesser General Public License as published        //
//  by the Free Software Foundation, either version 3 of the License, or            //
//  (at your option) any later version.                                             //
//                                                                                  //
//  Simple Multithreading is distributed in the hope that it will be useful,        //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of                  //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                    //
//  GNU Lesser General Public License for more details.                             //
//                                                                                  //
//  You should have received a copy of the GNU Lesser General Public License        //
//  along with Simple Multithreading. If not, see <http://www.gnu.org/licenses/>.   //
//                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////

#include "semaphores.h"

#ifdef WIN32

#include <Windows.h>

struct _SemaphoreData
{
  HANDLE counter;
  size_t count, maxCount;
};

inline Semaphore Semaphores_Create( size_t startCount, size_t maxCount )
{
  Semaphore sem = (Semaphore) malloc( sizeof(SemaphoreData) );
  sem->counter = CreateSemaphore( NULL, startCount, maxCount, NULL );
  sem->count = startCount;
  sem->maxCount = maxCount;
  
  return sem;
}

void Semaphores_Discard( Semaphore sem )
{
  CloseHandle( sem->counter );
  free( sem );
}

void Semaphores_Increment( Semaphore sem )
{
  ReleaseSemaphore( sem->counter, 1, &(sem->count) );
  sem->count++;  
}

void Semaphores_Decrement( Semaphore sem )
{
  WaitForSingleObject( sem->counter, INFINITE );
  sem->count--;
}

size_t Semaphores_GetCount( Semaphore sem )
{
  if( sem == NULL ) return 0;
  
  return sem->count;
}

void Semaphores_SetCount( Semaphore sem, size_t count )
{
  if( sem == NULL ) return;
  
  if( count > sem->maxCount ) count = sem->maxCount;

  size_t countDelta = (size_t) abs( (int) count - (int) sem->count );
  if( count > sem->count )
  {
    for( size_t i = 0; i < countDelta; i++ )
      Semaphores_Increment( sem );
  }  
  else if( count < sem->count )
  {
    for( size_t i = 0; i < countDelta; i++ )
      Semaphores_Decrement( sem ); 
  }
}

#else // Unix

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <malloc.h>

struct _SemaphoreData
{
  sem_t upCounter;
  sem_t downCounter;
  size_t maxCount;
};

DEFINE_NAMESPACE_INTERFACE( Semaphores, SEMAPHORE_INTERFACE );

Semaphore Semaphores_Create( size_t startCount, size_t maxCount )
{
  Semaphore sem = (Semaphore) malloc( sizeof(SemaphoreData) );
  sem_init( &(sem->upCounter), 0, maxCount - startCount );
  sem_init( &(sem->downCounter), 0, startCount );
  
  sem->maxCount = maxCount;
  
  return sem;
}

void Semaphores_Discard( Semaphore sem )
{
  sem_destroy( &(sem->upCounter) );
  sem_destroy( &(sem->downCounter) );
  free( sem );
}

void Semaphores_Increment( Semaphore sem )
{
  sem_wait( &(sem->upCounter) );
  sem_post( &(sem->downCounter) );  
}

void Semaphores_Decrement( Semaphore sem )
{
  sem_wait( &(sem->downCounter) );
  sem_post( &(sem->upCounter) ); 
}

size_t Semaphores_GetCount( Semaphore sem )
{
  int countValue;
  sem_getvalue( &(sem->downCounter), &countValue );
  
  return (size_t) countValue;
}

void Semaphores_SetCount( Semaphore sem, size_t count )
{
  if( count <= sem->maxCount )
  {
    size_t currentCount = Semaphores_GetCount( sem );
    
    if( currentCount > count )
    {
      for( size_t i = count; i < currentCount; i++ )
        Semaphores_Decrement( sem );
    }
    else if( currentCount < count )
    {
      for( size_t i = currentCount; i < count; i++ )
        Semaphores_Increment( sem );
    }
  }
}

#endif // WIN32
