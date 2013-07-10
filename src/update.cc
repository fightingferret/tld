#include <sys/types.h>
#include <stdio.h>
#include "define.h"
#include "struct.h"


bool    obj_compiled   = FALSE;
bool    mob_compiled   = FALSE;
bool    room_compiled  = FALSE;

void    affect_update    ( char_data* );
void    char_update      ( void );
void    drunk_update     ( char_data* );
void    campfire_update  ( char_data* );
void    light_update     ( char_data* );
void    obj_update       ( void );
bool    plague_update    ( char_data* );
void    poison_update    ( char_data* );
void    room_update      ( void );
bool    hear_channels    ( player_data* pc);
int     d;   




/*
 *   MAIN UPDATE FUNCTION
 */

inline bool hear_channels(player_data* pc)
{
  return(pc->position != POS_EXTRACTED && (pc->link == NULL
    || pc->link->connected == CON_PLAYING));
}

void update_handler( void )
{
  static int       pulse_area;
  static int     pulse_mobile;
  static int   pulse_violence;
  static int      pulse_point;
  static int       pulse_room;
  struct timeval        start;  

  gettimeofday( &start, NULL );


  event_update( );

  delete_list( extracted );

  if( --pulse_area <= 0 ) {
    pulse_area = number_range( PULSE_AREA/2, 3*PULSE_AREA/2 );
    area_update( );
    }

  if( --pulse_mobile <= 0 ) {
    pulse_mobile = PULSE_MOBILE;
    action_update( );
    auction_update( );
    regen_update( );
    time_update( );
  
    }

  if( --pulse_point <= 0 ) {
    pulse_point = number_range( PULSE_TICK/2, 3*PULSE_TICK/2 );
    char_update( );
    obj_update( );
    }

  if( --pulse_room <= 0 ) {
    pulse_room = number_range( PULSE_ROOM/2, 3*PULSE_ROOM/2 );
    room_update( );
    w3_who( );
    }

  if( --pulse_violence <= 0 ) {
    pulse_violence = PULSE_VIOLENCE;
    update_queue( );
    }

  pulse_time[ TIME_UPDATE ] = stop_clock( start );  
}


/*
 *   UPDATE CHARACTERS
 */


inline void update( char_data* ch )
{
  light_update( ch );
  campfire_update( ch );
  affect_update( ch );

  if( !plague_update( ch ) )
    poison_update( ch );

  return;
}


inline void update( mob_data* mob )
{
/*  mprog_data* mprog; */

  if( mob->position == POS_EXTRACTED || mob->in_room == NULL )
    return;

  /*
  if( mob->timer > 0 && --mob->timer == 0 ) 
    for( mprog = mob->species->mprog; mprog != NULL; mprog = mprog->next ) 
      if( mprog->trigger == MPROG_TRIGGER_TIMER ) {
        var_mob  = mob;
        var_room = mob->in_room;
        execute( mprog );
        return;
        } 
  */

  update( (char_data*) mob );
}


inline void update( player_data* player )
{
  int idle;

  if( !player->In_Game( ) )
    return;

  if( player->save_time+600 < current_time ) {
    if( is_set( &player->pcdata->message, MSG_AUTOSAVE ) )
      send( player, "Autosaving...\n\r" );
    write( player );
    reference( player, player->contents, -1 );
    }

  idle = current_time-player->timer;

  if( player->shdata->level < LEVEL_BUILDER 
    && idle > ( player->link == NULL ? 30 : 300 ) ) {
    if( player->was_in_room == NULL && player->in_room != NULL ) {
      if( player->switched != NULL ) 
        do_return( player->switched, "" );
      player->was_in_room = player->in_room;
      send( player, "You are pulled into nothingness.\n\r" );
      send_seen( player, "%s is pulled into nothingness.\n\r", player );
      write( player );
      player->From( );
      player->To( get_room_index( ROOM_LIMBO ) );
      }
    else if( idle > 900 ) {
      forced_quit( player );
      return;
      }
    }

  if( player->was_in_room != NULL )
    return;
    
  if( player->gossip_pts < 1000
    && number_range( 0, player->pcdata->trust >= LEVEL_AVATAR ? 3 : 5 ) == 0 )
    player->gossip_pts++;
  
  player->shdata->fame  = max( --player->shdata->fame, 0 );
  player->whistle       = max( --player->whistle, 0 );
  player->prayer        = min( ++player->prayer, 1000 );

  condition_update( player );

  update( (char_data*) player );
}


void char_update( void )
{   
  struct timeval   start;
  
  gettimeofday( &start, NULL );

  for( int i = 0; i < mob_list; i++ )
    update( mob_list[i] );
   
  for( int i = 0; i < player_list; i++ )
    update( player_list[i] );

  pulse_time[ TIME_CHAR_LOOP ] = stop_clock( start );  

  return;
}


/*
 *   CHARACTER UPDATE SUBROUTINES
 */


void affect_update( char_data* ch )
{
  affect_data*   aff;

  for( int i = ch->affected.size-1; i >= 0; i-- ) {
    aff = ch->affected[i];
    if( aff->leech == NULL ) {
      if( aff->duration-- == 1 && aff->type == AFF_FIRE_SHIELD
        && ch->position > POS_SLEEPING ) {
        send( ch, "The flames around you begin to fade.\n\r" );
        }   
      else if( aff->type == AFF_DEATH ) {
        update_max_hit( ch );
        update_max_move( ch );
        }
      if( aff->duration <= 0 )
        remove_affect( ch, aff );
      }
    }

  return;
}

void campfire_update( char_data* ch )
{
  obj_data*      campfire;
  
  if ( ( campfire = find_type( *ch->array, ITEM_CAMP ) ) == NULL) {
        return;
  }
  if ( campfire->pIndexData->vnum != 3412 ) {
        return;
  }
  /*if ( ( ch->get_skill( SKILL_CAMPFIRE ) ) < 1 ) {
        return;
  }*/
  if (number_range(0,49) > 1 ) {
        return;
  }
  if ( --campfire->value[2] == 1 ) {
    send( *ch->array, "%s begins to cool down.\n\r", campfire );
    return;
  }

  if ( campfire->value[2] != 0 || ch->in_room == NULL )
    return;

  send( *ch->array, "%s finally goes out and stops giving off heat.\n\r", campfire );
  
  campfire->From ();
  
  return;
}

void light_update( char_data* ch )
{
  int           i, j;
  obj_data*      obj;

for(i = 0; i < MAX_WEAR_HUMANOID; i++)
for(j = 0; j < MAX_LAYER; j++)
{
  if( ( obj = ch->Wearing ( i, j ) ) == NULL
    || obj->pIndexData->item_type != ITEM_LIGHT
    || obj->value[2] < 0 )
    continue;

  if( obj->pIndexData->vnum == OBJ_BALL_OF_LIGHT 
    && IS_AFFECTED( ch, AFF_CONTINUAL_LIGHT ) )
    continue;

  if( --obj->value[2] == 1 ) {
    if( ch->position > POS_SLEEPING ) 
      send( ch, "%s you are carrying flickers briefly.\n\r", obj );
    send( *ch->array, "%s carried by %s flickers briefly.\n\r", obj, ch );
    continue;
    }

  if( obj->value[2] != 0 || ch->in_room == NULL )
    continue;

  if( ch->position > POS_SLEEPING ) 
    send( ch, "%s you are carrying extinguishes.\n\r", obj );
  send( *ch->array, "%s carried by %s extinguishes.\n\r", obj, ch );

  obj->From ();
  obj->Extract( );

  return;
}
}


bool plague_update( char_data* ch )
{
  affect_data*  aff;
  int           con;

  if( !is_set( ch->affected_by, AFF_PLAGUE ) ) 
    return FALSE;

  for( int i = 0; ; i++ ) {
    if( i >= ch->affected ) {
      bug( "%s has the plague with no affect??", ch->real_name( ) );
      return FALSE;
      }
    aff = ch->affected[i];
    if( aff->type == AFF_PLAGUE )
      break; 
    }

  con = ch->Constitution( );
  ch->mod_con--;
  aff->level++;

  update_max_hit( ch );
  update_max_move( ch );

  if( con == 3  ) {
    death( ch, NULL, "the plague" );
    return TRUE;
    }

  if( aff->level < 3 ) {
    send( *ch->array, "%s coughs violently.\n\r", ch );
    if( aff->level == 1 ) 
      send( ch, "You cough violently.\n\r" );
    else
      send( "You cough, your throat burning in pain.\n\rThis is much worse than any cold, you have the plague!\n\r", ch );
    return FALSE;
    }

  send( "The plague continues to destroy your body.\n\r", ch );
  send( *ch->array,
    "%s coughs violently, %s has obviously contracted the plague!\n\r",
    ch, ch->He_She( ) );
 
  return FALSE;
}


void poison_update( char_data* ch )
{
  if( is_set( ch->affected_by, AFF_POISON ) ) {
    send( ch, "Your condition deteriorates from a poison affliction.\n\r" );
    send_seen( ch,
      "%s's condition deteriorates from a poison affliction.\n\r", ch );
    inflict( ch, NULL, 2, "poison" );
    return;
    }

  if( ch->position == POS_INCAP ) 
    inflict( ch, NULL, 1, "[BUG] incap??" );
  else
    if( ch->position == POS_MORTAL ) 
      inflict( ch, NULL, 2, "bleeding to death" );

  return;
}


/*
 *   TIMED ACTIONS
 */


void room_update( void )
{
  action_data*   action;
  area_data*       area;
  char_data*        rch;
  room_data*       room;
  struct timeval   start;
  
  gettimeofday( &start, NULL );

  for( area = area_list; area != NULL; area = area->next ) {
    for( room = area->room_first; room != NULL; room = room->next ) {
      for( int i = 0; i < room->contents; i++) {
        if( ( rch = player( room->contents[i] ) ) != NULL ) {
          for( action = room->action; action != NULL; action = action->next ) {
            if( ( ( rch != NULL && action->trigger == TRIGGER_RANDOM )
              || action->trigger == TRIGGER_RANDOM_ALWAYS )
              && number_range( 0, 999 ) < action->value ) {
                 var_room = room; 
                 var_ch     = NULL;
                 var_victim = NULL;
                 var_mob    = NULL; 
                 execute( action );
	    }
          }
          break;
        }
      }  
    }
  }

  pulse_time[ TIME_RNDM_ACODE ] = stop_clock( start );  
     
  return;
}


/*
 *   RESETTING OF AREAS
 */


void area_update( void )
{
  area_data*        area;
  room_data*        room;
  struct timeval   start;
  bool              save  = TRUE;

  gettimeofday( &start, NULL );

  for( int i = 0; i < max_clan; i++ ) 
    if( clan_list[i]->modified )
      save_clans( clan_list[i] );

  for( area = area_list; area != NULL; area = area->next ) {
    if( ++area->age < 15 && ( area->nplayer != 0 || area->age < 5 ) )
      continue;

    for( room = area->room_first; room != NULL; room = room->next )
      if( !player_in_room( room ) ) {
        reset_room( room );
        save_room_items( room );
        }

    area->age = number_range( 0, 3 );

    if( save && area->modified ) {
      save_area( area ); 
      save = FALSE;
      }
    }
  
  shop_update( );

  pulse_time[ TIME_RESET ] = stop_clock( start );

  return;
}


/*
 *  OBJECT UPDATE
 */


void obj_update( void )
{   
  area_data*              area;
  affect_data*          affect;
  obj_data*                obj;
  obj_data*            content  = NULL;
  obj_data*       content_next;
  obj_clss_data*          fire;
  oprog_data*            oprog;
  room_data*              room;
  char*                message;
  struct timeval         start;
  
  gettimeofday( &start, NULL );

  for( area = area_list; area != NULL; area = area->next ) {
    for( room = area->room_first; room != NULL; room = room->next ) {
      for( int i = 0; i < room->contents; i++) {
        if( ( obj = object( room->contents[i] ) ) != NULL ) {
          for( oprog = obj->pIndexData->oprog; oprog != NULL; 
               oprog = oprog->next ) {
            if( ( oprog->trigger == OPROG_TRIGGER_RANDOM )
              && number_range( 0, 999 ) < oprog->value ) {
                 var_room = room; 
                 var_obj  = obj;
                 var_ch     = NULL;
                 var_victim = NULL;
                 var_mob    = NULL; 
                 execute( oprog );
	    }
          }
        }
      }  
    }
  }
  /*
  for( obj = obj_list; obj != NULL; obj = obj->next ) {
    if( obj->valid != OBJ_DATA )
      continue;

    for( int i = obj->affected.size-1; i >= 0; i-- ) {
      affect = obj->affected[i];
      if( affect->duration > 0 && --affect->duration == 0 )
        remove_affect( obj, affect );     
      }

    room = ( obj->array == NULL ? NULL : obj->array->room( ) );

    if( room == NULL || obj->timer <= 0 || --obj->timer > 0 ) 
      continue;

    for( oprog = obj->pIndexData->oprog; oprog != NULL; oprog = oprog->next )
      if( oprog->trigger == OPROG_TRIGGER_TIMER ) {
        var_obj  = obj;
        var_room = room;
        execute( oprog );
        break;
        }

    if( oprog != NULL )
      continue;

    if( obj->pIndexData->item_type == ITEM_FIRE ) {
      if( ( fire = get_obj_index( obj->value[1] ) ) != NULL 
        && ( content = create( fire ) ) != NULL )
        put_obj( content, room );
      obj->Extract( ); 
      return;
      }

    switch( obj->pIndexData->item_type ) {
     default: 
      message = "%s vanishes.";
      break;

     case ITEM_FOUNTAIN:
      message = "%s dries up."; 
      break;

     case ITEM_CORPSE_NPC:
     case ITEM_CORPSE_PC:
      message = "%s rots, and is quickly eaten by a grue.";
      break;

     case ITEM_FOOD:
      message = "%s decomposes.";
      break;
      }

    fsend_room( room, message, obj );
  */
    /*
    if( obj->pIndexData->item_type == ITEM_CORPSE_PC ) {
      for( content = obj->contents; content != NULL;
        content = content_next ) {
        content_next = content->next_content;
        if( number_range( 1, 3 ) != 3 ) {
          remove( content, content->number ); 
          put_obj( content, room );
          }
        }
      }

    obj->Extract( );
    }
    */

  pulse_time[ TIME_OBJ_LOOP ] = stop_clock( start );  

  return;
}


void hour_update( void )
{
  bool          warn_them;
  action_data*  action;
  obj_data*     obj;
  oprog_data*   oprog;
  area_data*    area;
  mprog_data*   mprog;
  char_data*    rch;
  char_data*    ch;
  room_data*    room;
  struct timeval        start;

  gettimeofday( &start, NULL );

  for( area = area_list; area != NULL; area = area->next ) {
    for( room = area->room_first; room != NULL; room = room->next ) {
// BEGIN SADIS
      if( (weather.hour == 20) && (room->vnum == ROOM_LURKER) ) {
        for( int popup = room->contents-1 ; popup >= 0 ; popup-- ) {
          if( (rch = mob( room->contents[popup] ) ) != NULL ) {
            if( rch->lurking_in != NULL ) {
              rch->From( );
              rch->To( rch->lurking_in );
              rch->lurking_in->lurker_in=TRUE;
//              popup--;
            }
          }
        }
      }
      if( room->lurker_in ) {
        act_room( room, "The earth groans.\n\r" );
        room->lurker_in=FALSE;
      }
// END SADIS
      for( action = room->action; action != NULL; action = action->next ){
        if( action->trigger == TRIGGER_TIME &&
            weather.hour == action->value ) {
          var_room = room;
          var_ch     = NULL;
          var_victim = NULL;
          var_mob    = NULL;
          execute( action );
        }
      }
      for( int i = 0; i < room->contents; i++) {
        if( ( rch = player( room->contents[i] ) ) != NULL ) {
          for( int j = 0; j < rch->contents ; j++ ) {
            if( (obj = object( rch->contents[j] ) ) != NULL ) {
              for( oprog = obj->pIndexData->oprog; oprog != NULL; oprog = oprog->next ) {
                if( oprog->trigger == OPROG_TRIGGER_TIME &&
                    ( weather.hour == oprog->value ||
                      oprog->value == -1 ) ) {
                    var_ch   = rch;
                    var_obj  = obj;
                    var_room = room;
                    execute( oprog );
                }
              }
            }
          }
          for( int k = 0; k < rch->wearing ; k++ ) {
            if( (obj = object( rch->wearing[k] ) ) != NULL ) {
              for( oprog = obj->pIndexData->oprog; oprog != NULL; oprog = oprog->next ) {
                if( oprog->trigger == OPROG_TRIGGER_TIME &&
                    ( weather.hour == oprog->value ||
                      oprog->value == -1 ) ) {
                    var_ch   = rch;
                   var_obj  = obj;
                    var_room = room;
                    execute( oprog );
                }
              }  
            }    
          }      
        }        
        if( ( obj = object( room->contents[i] ) ) != NULL ) {
          for( oprog = obj->pIndexData->oprog; oprog != NULL; oprog = oprog->next ) {
            if( oprog->trigger == OPROG_TRIGGER_TIME &&
                ( weather.hour == oprog->value ||
                  oprog->value == -1 ) ) {
                var_obj  = obj;
                var_room = room;
                execute( oprog );
            }
          }  
        }    
        if( (rch = mob( room->contents[i] ) ) != NULL ) {
          for( mprog = rch->species->mprog ; mprog != NULL ; mprog = mprog->next ) {
            if( mprog->trigger == MPROG_TRIGGER_TIME &&
                ( weather.hour == mprog->value ||
                  mprog->value == -1 ) ) {
                var_ch  = rch;
                var_room = room;
                execute( mprog );
            }
          }
          if( weather.hour == 6 ) {
            if( is_set( &rch->species->act_flags, ACT_NIGHT_ONLY) ) {
              send( *rch->array,  "A %s groans loudly as they sink into the ground!!\n\r", rch->descr->singular );
              rch->lurking_in=room;
              rch->From( );
              rch->To( get_room_index(ROOM_LURKER) );
              if( ( room->vnum != ROOM_LURKER ) ) i--;
            }
            if( rch->living_dead && ( room->vnum != ROOM_LURKER ) ) {
              send( *rch->array,  "A %s groans loudly as they sink into  the ground!!\n\r", rch->descr->singular );

// reverse comments on next 4 lines to have living dead survive until killed
//              rch->lurking_in=room;
//              rch->From( );  
//              rch->To( get_room_index(ROOM_LURKER) );
              rch->Extract( );
              i--;
            }
         }
        }
      }
    }
  }
  pulse_time[ TIME_RNDM_ACODE ] = stop_clock( start );
          
}
// END CHANGES BY SADIS



