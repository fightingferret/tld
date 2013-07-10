#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include "define.h"
#include "struct.h"


/*
 *   LIST GENERATING FUNCTIONS
 */


int get_trust( char_data *ch )
{
  if( ch->pcdata == NULL )
    return LEVEL_HERO - 1;

  if( ch->pcdata->trust != 0 )
    return ch->pcdata->trust;

  return ch->shdata->level;
}


/*
 *   WEIRD ROUTINES
 */

// REPAIRED BY SADIS
char_data* rand_player( room_data* room )
{

  char_data*  ch;
  int        i,j,k;

  for( i = 0, j = 0; i < room->contents; i++ )
    if( ( ch = player( room->contents[i] ) ) != NULL )
      j++;

  if( j == 0 )
    return NULL;

  j = number_range( 1, j );

  for( i = 0, k = 0; i < room->contents; i++ ) {
    if( ( ch = player( room->contents[i] ) ) != NULL ) {
      k++;
      if(j == k)
        return ch;
    }
  }

  return NULL;
} 
// REPAIRED BY SADIS
char_data* rand_char( room_data* room )
{

  char_data*  ch;
  int        i,j,k;

  for( i = 0, j = 0; i < room->contents; i++ )
    if( ( ch = character( room->contents[i] ) ) != NULL )
      j++;

  if( j == 0 )
    return NULL;

  j = number_range( 1, j );

  for( i = 0, k = 0; i < room->contents; i++ ) {
    if( ( ch = character( room->contents[i] ) ) != NULL ) {
      k++;
      if(j == k )
        return ch;
    }
  } 
  return NULL;
}

char_data* rand_victim( char_data* ch )
{
    int      count  = 0;
    room_data* room = ch->in_room;

    for( int i = 0; i < room->contents; i++ )
        if( character( room->contents[i] ) != NULL )
            count++;
  
    if( count == 0 )
        return NULL;

    count = number_range( 1, count );

    for( int i = 0; ; i++ )
        if( character( room->contents[i] ) != NULL && --count == 0 )
            return (char_data*) room->contents[i];
 
  return NULL;
} 
 


