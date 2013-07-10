#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include "define.h"
#include "struct.h"


void remove_weight( thing_data* obj, int i )
{
    content_array*  where;
    player_data*       pc; 
    int            w1, w2;

    if( ( where = obj->array ) == NULL )
        return;

    w1  = obj->Weight( i );
 
    where->number -= obj->Number( i );

    for( ; ; where = obj->array ) {
        if( ( obj = where->where ) == NULL || obj->array == NULL || ( ( pc = player( obj ) ) != NULL && ( where == &pc->locker || where == &pc->junked ) ) ) {
            where->weight -= w1;
            break;
        }

        w2 = obj->Weight( );
        where->weight -= w1;
        w1  = w2-obj->Weight( );
    }
}


void add_weight( thing_data* obj, int i )
{
    content_array*   where;
    player_data*        pc;
    int             w1, w2;

    if( ( where = obj->array ) == NULL )
        return;

    w1  = obj->Weight( i );
 
    where->number += obj->Number( i );
  
    for( ; ; where = obj->array ) {
        if( ( obj = where->where ) == NULL || obj->array == NULL || ( ( pc = player( obj ) ) != NULL && ( where == &pc->locker || where == &pc->junked ) ) ) {
            where->weight += w1;
            break;
        }

        w2 = obj->Weight( );
        where->weight += w1;
        w1 = obj->Weight( )-w2;
    }
}
  

/*
 *   TRANSFER FROM
 */


thing_data* Thing_Data :: From( int )
{
    remove_weight( this, number );

    *array -= this;
    array   = NULL;

    return this;
}


thing_data* Char_Data :: From( int )
{
    room_data* room;

    if (in_room) in_room->contents.light -= wearing.light;
        in_room = NULL;

    if( ( room = Room( array->where ) ) != NULL ) {
        if( pcdata != NULL )
            room->area->nplayer--;

        stop_fight( this );
        stop_events( this, execute_drown );
    }

    Thing_Data :: From( );

    return NULL;
}


thing_data* Obj_Data :: From( int i )
{
    char_data*         ch;
    obj_data*         obj;
    content_array*  where  = array;
  
    if( number > i ) {
        remove_weight( this, i );
        number        -= i;
        obj            = duplicate( this );   
        obj->number    = i;
        obj->selected  = i;
        return obj;
    }

    Thing_Data :: From( );

    if( ( ch = character( where->where ) ) != NULL && where == &ch->wearing )
        unequip( ch, this );

    for( int i = 0; i < *where; i++ ) 
        if( ( ch = character( where->list[i] ) ) != NULL && ch->cast != NULL && ch->cast->target == this )
            disrupt_spell( ch ); 

    stop_events( this, execute_decay );

    return this;
}


/*
 *   TRANSFER TO
 */

  
void Thing_Data :: To( thing_data* thing )
{
    To( &thing->contents );
}


void Thing_Data :: To( content_array* where )
{
    if( array == NULL ) {
        *where += this;
        array   = where;
    }

    add_weight( this, number );
}


void Char_Data :: To( thing_data* thing )
{
    To( &thing->contents );
}


void Char_Data :: To( content_array* where )
{
    room_data*         room;
    wizard_data*        imm;
    char_data*          rch;
    char_data*       randch;

    if( array != NULL ) {
        roach( "" );
        roach( "-- Ch = %s", this );
        From( number );
    }

    Thing_Data :: To( where );

    /* CHARACTER TO ROOM */

    if( ( room = Room( where->where ) ) != NULL ) {
        room_position = -1;
        in_room       = room;

        if( pcdata != NULL ) 
            room->area->nplayer++;

        if( ( imm = wizard( this ) ) != NULL ) {
            imm->custom_edit  = 0;
            imm->room_edit    = NULL;
            imm->action_edit  = NULL;
            imm->exit_edit    = NULL;
        }

        if( water_logged( room ) ) {
            remove_bit( &status, STAT_SNEAKING );
            remove_bit( &status, STAT_HIDING );
            remove_bit( &status, STAT_CAMOUFLAGED );
        }
        
        if( species == NULL )
            if( is_set( &status, STAT_HIDING ) ) 
                if( !is_set( this->pcdata->pfile->flags, PLR_SNEAK ) )
                    leave_shadows( this );                 

        if( is_submerged( this ) )
            enter_water( this );

        if (in_room) in_room->contents.light += wearing.light;

        // Random Attacks and Paladin Protect skill

        if ( species != NULL ) 
            if ( ( randch = rand_victim( this ) ) != NULL )
                if ( is_aggressive( this, randch ) ) {
                    if ( randch->protected_by != NULL && randch->in_room != NULL && randch->protected_by->in_room != NULL ) {
                        if ( randch->in_room == randch->protected_by->in_room ) {
                            send( randch, "%s throws %sself in front of you!\n\r", randch->protected_by, randch->protected_by->Him_Her( )  );
                            send( randch->protected_by, "You throw yourself in front of %s.\n\r", randch );
                            send_seen( randch, "%s throws %sself in front of %s.\n\r", randch->protected_by, randch->protected_by->Him_Her( ), randch );
                            randch = randch->protected_by;
                        }
                        else
                            stop_protect( randch );
                    }
                    init_attack( this, randch );
                }
        
        // End Random

        for( int i = 0; i < room->contents; i++ ) {
            if( ( rch = character( room->contents[i] ) ) == NULL || rch == this )
                continue;
 
            if( rch->species != NULL && species != NULL && rch->species->group == species->group && rch->species->group != GROUP_NONE ) {
                share_enemies( this, rch );   
                share_enemies( rch, this );   
            }
            if( is_aggressive( this, rch ) ) 
                init_attack( this, rch );
            if( is_aggressive( rch, this ) ) 
                init_attack( rch, this );
        }
    
        return; 
    }

    roach( "Attempted transfer of character to non-room object." );
}


void Obj_Data :: To( thing_data* thing )
{
    To( &thing->contents );
}


void Obj_Data :: To( content_array* where )
{
    event_data* event;
    room_data*   room;
    char_data*     ch;
    obj_data*     obj;
    int             i;

    if( array != NULL ) {
        roach( "Adding object somewhere which isn't nowhere." );
        roach( "-- Obj = %s", this );
        From( number );
    }

    if( ( room = Room( where->where ) ) != NULL ) {
        if( pIndexData->item_type == ITEM_CORPSE && value[0] > 0 ) {
            event = new event_data( execute_decay, this );
            add_queue( event, value[0] ); 
        }

        if( pIndexData->item_type == ITEM_GATE && value[0] > 0 ) {
            event = new event_data( execute_gate, this );
            add_queue( event, value[0] ); 
        }
    }

    if( ( ch = character( where->where ) ) != NULL && where == &ch->wearing ) {
        equip( ch, this );
        for( i = 0; i < ch->wearing; i++ ) {
            obj = (obj_data*) ch->wearing[i];
            if( obj->position > position || ( obj->position == position && obj->layer > layer ) ) 
                break;
        }
        insert( *where, this, i );
        array = where; 
    }  

    Thing_Data :: To( where );
}


/*
 *  DECAY
 */

/*
void execute_decay( event_data* event )
{
    obj_data* obj = (obj_data*) event->owner;

    obj->shown = 1;
    fsend( *obj->array, "%s rots, and is quickly eaten by a grue.", obj );

    obj->Extract( );
}
*/

// sadis - rewrite execute_decay for corpses to return to life
void execute_decay( event_data* event )  
{
  obj_data* obj = (obj_data*) event->owner;
  mob_data *mob1, *mob2;  
  char_data *tch;
  species_data* species, *nspecies;
  int   zombie, skeleton;
  bool skelet=FALSE, zombi=FALSE;
  room_data* room;
  room_data* trm;
     
  obj->shown = 1;
  fsend( *obj->array, "%s rots, and is quickly eaten by a grue.", obj );
  trm=get_room_index( ROOM_LURKER );

  if( ( room = Room( obj->array->where ) ) == NULL ) {
  //not on the ground, ignore it
    obj->Extract( );
    return;
  }
  if( !( room->sector_type == SECT_WATER_SURFACE
     || room->sector_type == SECT_UNDERWATER
     || room->sector_type == SECT_RIVER
     || room->sector_type == SECT_SHALLOWS
     || room->sector_type == SECT_CITY
     || room->sector_type == SECT_AIR
     || room->sector_type == SECT_ROAD
     || room->sector_type == SECT_STREAM )
     && ( number_range(0,20) <= 1 )
     && !is_set( &room->room_flags, RFLAG_NO_SPAWN_UNDEAD ) ) {

    if( ( species = get_species( obj->value[1] ) ) == NULL ) {
    //player corpse, remove it, don't pop no corpse
      obj->Extract( );
      return;
    }
    if( ( room = Room( obj->array->where ) ) == NULL ) {
    //not on the ground, remove it, don't pop no corpse
    obj->Extract( );
    return;
    }

    if( ( nspecies = get_species( species->skeleton ) ) != NULL ) {
      mob1 = create_mobile( nspecies );
      mreset_mob( mob1 );  
      skelet=TRUE;
    }

    if( ( nspecies = get_species( species->skeleton ) ) != NULL ) {
      mob1 = create_mobile( nspecies );
      mreset_mob( mob1 );
      skelet=TRUE;
    }

    if( ( nspecies = get_species( species->zombie ) ) != NULL ) {
      mob2 = create_mobile( nspecies );
      mreset_mob( mob2 );
      zombi=TRUE;
    }

    if( skelet ) {
      if( zombi ) {
        if( number_range(0,1) == 0 ) {
          tch=mob(mob1);
          mob2->Extract( );
        }else {
          tch=mob(mob2);
          mob1->Extract( );
        }
      } else {
        tch=mob(mob1);
      }
      tch->To( trm );
      tch->lurking_in=room;
      tch->living_dead = TRUE;
    }
    if( zombi && !skelet ) {
      tch=mob(mob2);
      tch->To( trm );
      tch->lurking_in=room;
      tch->living_dead = TRUE;
    }
  }
  obj->Extract( );
}


void execute_gate( event_data* event )
{
    obj_data* obj = (obj_data*) event->owner;

    obj->shown = 1;
    fsend( *obj->array, "%s disappears.", obj );

    obj->Extract( );

}

