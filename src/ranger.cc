#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "define.h"
#include "struct.h"


obj_data* has_campfire_fuel( char_data* ch );


/*
 *   SKIN FUNCTION
 */


void do_skin( char_data* ch, char* argument )
{
    mprog_data*         mprog;
    obj_data*          corpse;
    thing_data*         thing;
    thing_array*         list;
    species_data*     species;
    char_data*            rch;

    if( is_confused_pet( ch ) ) {
        return;
    }

    if( ( thing = one_thing( ch, argument, "skin", ch->array ) ) == NULL ) {
        return;
    }

    if( ( corpse = object( thing ) ) == NULL ) {
        fsend( ch, "Skinning %s alive would be cruel and unusual.\n\r", thing );
        return;
    } 

    if( corpse->pIndexData->item_type != ITEM_CORPSE ) {
        send( ch, "You can only skin corpses.\n\r" );
        return;
    }

    if( corpse->pIndexData->vnum == OBJ_CORPSE_PC ) {
        send( ch, "You can't skin player corpses!\n\r" );
        return;
    }

    if( ( species = get_species( corpse->value[1] ) ) == NULL ) {
        send( ch, "Corpses without a species cannot be skinned.\n\r" );
        return;
    }

    for( mprog = species->mprog; mprog != NULL; mprog = mprog->next ) {
        if( mprog->trigger == MPROG_TRIGGER_SKIN ) {
            var_obj  = corpse;
            var_ch   = ch;    
            var_room = ch->in_room;
            execute( mprog ); 
            return;
        }
    }

    list = get_skin_list( species );

    if( list == (thing_array*) -1 ) {
        send( ch, "You cannot skin %s.\n\r", corpse );
        return;
    }

    if( list == NULL ) {
        fsend( ch, "You skin %s, but there seems to be nothing of value on it.", corpse );
        fsend( *ch->array, "%s skins %s but is unable to extract anything of value.", ch, corpse );
    }
    else {
        fsend( ch, "You skin %s producing %s.\n\r", corpse, list );

        for( int i = 0; i < *ch->array; i++ ) {
            if( ( rch = character( ch->array->list[i] ) ) != NULL && rch != ch && ch->Seen( rch ) ) {
                fsend( rch, "%s skins %s producing %s.\n\r", ch, corpse, list );
            }
        }

        for( int i = 0; i < *list; i++ ) {
            if( ( ch->contents.number + 1 ) <= ch->can_carry_n() )
              list->list[i]->To( ch );
            else
              list->list[i]->To( ch->in_room );
            consolidate( (obj_data*) list->list[i] );
        }

        delete list;
    }
  
    drop_contents( corpse ); 
    corpse->Extract( 1 );
}


/*
 *   SCAN FUNCTIONS
 */


bool scan_room( char_data* ch, room_data* room, const char* word, bool need_return )
{

    char           tmp  [ MAX_STRING_LENGTH ];
    const char*   name;
    char_data*     rch;
    bool         found  = FALSE;
    int         length  = 0; 

    room->distance = 0;

    select( room->contents, ch );
    rehash( ch, room->contents );

    for( int i = 0; i < room->contents; i++ ) {
        if( ( rch = character( room->contents[i] ) ) == NULL || rch->shown == 0 || !rch->Seen( ch ) || ch == rch || ( rch->species != NULL && is_set( &rch->species->act_flags, ACT_MIMIC ) ) ) {
            continue;
        }

        name = rch->Seen_Name( ch, rch->shown );

        if( !found ) {
            if( need_return ) {
                send( ch, "\n\r" );
            }
            sprintf( tmp, "%12s : %s", word, name );
            *tmp   = toupper( *tmp );
            length = strlen( tmp );
            found  = TRUE;
        }
        else {
            length += strlen( name )+2;
            if( length > 75 ) {
                length = strlen( name )+7;
                sprintf( tmp+strlen( tmp ), ",\n\r              %s", name );
            }
        else
            sprintf( tmp+strlen(tmp), ", %s", name );
        }
    }

    if( found ) {
        strcat( tmp, "\n\r" );
        send( ch, tmp );
    }

    return found;
} 


void do_scan( char_data* ch, char* argument )
{
    char             tmp  [ ONE_LINE ];
    room_data*      room  = ch->in_room;
    room_data*     room1;
    room_data*     room2;
    bool        anything  = FALSE;
    bool         is_auto  = !strcmp( argument, "shrt" );

    if( ch->in_room->is_dark () ) {
        if( !is_auto )
            send( ch, "The room is too dark to scan.\n\r" );
        return;
    }

    if( !is_auto )
        anything = scan_room( ch, room, "[Here]", FALSE );

    room->distance = 0;

    for( int i = 0; i < room->exits; i++ ) {
        if( is_set( &room->exits[i]->exit_info, EX_CLOSED ) )
            continue;
        room1     = room->exits[i]->to_room;
        anything |= scan_room( ch, room1, dir_table[room->exits[i]->direction].name, !anything && is_auto );
        if( ch->get_skill( SKILL_SCAN ) != 0 ) {
            for( int j = 0; j < room1->exits; j++ ) {
                if( is_set( &room1->exits[j]->exit_info, EX_CLOSED ) )
                    continue;
                room2 = room1->exits[j]->to_room;
                if( room2->distance != 0 ) {
                    sprintf( tmp, "%s %s", room->exits[i]->direction == room1->exits[j]->direction ? "far" : dir_table[ room->exits[i]->direction ].name, dir_table[ room1->exits[j]->direction ].name );
                    anything |= scan_room( ch, room2, tmp, !anything && is_auto );
                }
            }  
        }
    } 

    if( !anything && !is_auto ) 
        send( ch, "You see nothing in the vicinity.\n\r" );

  /*--  CLEANUP DISTANCE --*/

    room->distance = MAX_INTEGER;

    for( int i = 0; i < room->exits; i++ ) { 
        room1 = room->exits[i]->to_room;
        room1->distance = MAX_INTEGER;
        for( int j = 0; j < room1->exits; j++ )
            room1->exits[j]->to_room->distance = MAX_INTEGER;
    }
}


/*
 *   SPELLS
 */


bool spell_tame( char_data* ch, char_data* victim, void*, int level, int )
{
    if( null_caster( ch, SPELL_TAME ) )
        return TRUE;

    if( is_set( &victim->status, STAT_PET ) ) {
        send( ch, "%s is already tame.\n\r", victim );
        return TRUE;
    }

    if( victim->species == NULL || !is_set( &victim->species->act_flags, ACT_CAN_TAME ) || 
        makes_save( victim, ch, RES_MIND, SPELL_TAME, level ) || victim->leader != NULL || 
        victim->shdata->level > ch->shdata->level ) {

        send( ch, "%s ignores you.\n\r", victim );
        send( *ch->array, "%s ignores %s.\n\r", victim, ch );
        return TRUE;
    }

    if( victim->shdata->level > ch->shdata->level-pet_levels( ch ) ) {
        send( ch, "You fail as you are unable to control more animals.\n\r" );
        return TRUE;
    }

    if( is_set( &victim->species->act_flags, ACT_MOUNT ) && has_mount( ch ) )
        return TRUE;

    if( ch->leader == victim )
        stop_follower( ch );

    set_bit( &victim->status, STAT_PET );
    set_bit( &victim->status, STAT_TAMED );

    remove_bit( &victim->status, STAT_AGGR_ALL );
    remove_bit( &victim->status, STAT_AGGR_GOOD );
    remove_bit( &victim->status, STAT_AGGR_EVIL );

    add_follower( victim, ch );
    unregister_reset( victim );
    return TRUE;
}


bool spell_barkskin( char_data* ch, char_data* victim, void*, int level, int duration )
{
    spell_affect( ch, victim, level, duration, SPELL_BARKSKIN, AFF_BARKSKIN );

    return TRUE;
}

bool spell_thorn_shield( char_data* ch, char_data* victim, void*, int level, int duration )
{
    spell_affect( ch, victim, level, duration, SPELL_THORN_SHIELD, AFF_THORN_SHIELD );

    return TRUE;
}


/*
 *   HEALING SPELLS
 */


bool spell_balm( char_data* ch, char_data* victim, void*, int level, int )
{
    heal_victim( ch, victim, spell_damage( SPELL_BALM, level ) );

    return TRUE;
}


bool spell_surcease( char_data* ch, char_data* victim, void*, int level, int )
{
    heal_victim( ch, victim, spell_damage( SPELL_SURCEASE, level ) );

    return TRUE;
}


bool spell_poultice( char_data* ch, char_data* victim, void*, int level, int )
{
    heal_victim( ch, victim, spell_damage( SPELL_POULTICE, level ) );

    return TRUE;
}

void do_campfire( char_data* ch, char* argument ){

    obj_data*	    campfire;
    char_data*      keeper;
    room_data*      room;
    
    room = ch->in_room;

    if( ( ch->get_skill( SKILL_CAMPFIRE ) ) < 1 ) {
        send( ch, "You don't have the skill to make campfires. \n\r" );
        return;
    }
    if( ch->mount != NULL ) {
		send( ch, "You can't make a campfire while mounted.\n\r" );
		return;
	}
    
    switch( room->sector_type ) {
        case SECT_CITY :
            send( ch, "You don't think the town guards would like that.\n\r" );
		    return;
        case SECT_RIVER :
        case SECT_UNDERWATER :
        case SECT_SHALLOWS :
        case SECT_STREAM :
        case SECT_WATER_SURFACE :
            send( ch, "You are not skilled enough to make fire burn on water.\n\r" );
		    return;
        case SECT_AIR :
            send( ch, "You can't make floating campfires.\n\r" );
		    return;
        case SECT_SWAMP :
            send( ch, "In the swamp? I don't think so.\n\r" );
		    return;
    }

  	if( ch->fighting != NULL ) {
        send( ch, "You are in battle!\n\r" );
        return;
    }
	if ( ch->position == POS_SLEEPING ) {
		send( ch, "Are you insane?!\n\r");
		return;
	}
	if( ( keeper = active_shop( ch ) ) != NULL ) {
        send( ch, "%s won't allow you to burn up %s shop.\n\r", keeper, keeper->His_Her( ) );
		send( ch, "%s shakes %s head while staring at you menacingly.\n\r", keeper, keeper->His_Her( ) );
        return;
   	}
	if ( ( campfire = find_type( *ch->array, ITEM_CAMP ) ) != NULL) {
		send( ch, "It appears this spot is taken.\n\r");
		return;
	}
    if( ( find_vnum( ch->contents, 79 ) ) == NULL ) {
        send( ch, "You need flint to start the fire.\n\r");
        return;
    }
    if ( ch->move < (50 - 3*( ch->get_skill( SKILL_CAMPFIRE ) ) ) ) {
        send( ch, "All this tediousness has you exhausted.\n\r"); 
        return;
    } 

    if( ( campfire = has_campfire_fuel( ch ) ) == NULL ) {
        return;        
    }

    set_delay( ch, 60 );
    
    if( find_vnum( *ch->array, 3828 ) == NULL ) {
        add_queue( new event_data( campfire_one, ch ), 0 );
        return;
    }
    else {
        add_queue( new event_data( campfire_two, ch ), 0 );
        return;
    }   
    return;
}


void do_camp( char_data* ch, char* argument ){

    obj_data*       campfire;
    affect_data     affect;
    bool            nullflag  = FALSE;

    if( ch->fighting != NULL ) {
		send( ch, "Resting while fighting is a bad idea.\n\r" );
		return;
	}
    if ( ( campfire = find_type( *ch->array, ITEM_CAMP ) ) == NULL) {
        send(ch, "You do not see a good place to camp here.\n\r");
        return;
    }
    if ( is_set( ch->affected_by, AFF_CAMPING ) ) {
        send(ch, "You are already camping. \n\r");
        return;
    }
    disrupt_spell( ch ); 
    if ( ( campfire->value[2]) == NULL || (campfire->value[2] ) == 0 ) {
        nullflag = TRUE;
        campfire->value[2] = 1;
    }    

    send( ch, "You rest your weary bones in front of the campfire.\n\r");
    send( *ch->array, "%s rests lazily in front of a campfire.\n\r", ch );
    ch->position    = POS_RESTING;

    affect.type     = AFF_CAMPING;
    affect.location = APPLY_NONE;     
    affect.modifier = 0; 
    affect.duration = ( ( campfire->value[2] )*3 );
    affect.leech    = NULL;

    add_affect( ch, &affect );

    if (nullflag) {
        campfire->value[2] = 0;
    }
    return;
}

void campfire_one( event_data* event )
{
    char_data*      ch = (char_data*) event->owner;
    obj_data*       obj;

    send( *ch->array, "%s prepares a campfire pit.\n\r", ch );
    send( ch, "You prepare the campfire pit.\n\r" );        
    ch->improve_skill( SKILL_CAMPFIRE );
    obj = create( get_obj_index( 3828 ) );	
    if ( obj == NULL ) {
        code_bug( "No Campfire pit object?! Vnum = 3828" );
        return;
    }
    obj->To( ch->in_room );
    add_queue( new event_data( campfire_two, ch ), 30 );
}

void campfire_two( event_data* event )
{
    char_data*      ch = (char_data*) event->owner;
    obj_data*       obj = NULL;    

    if( ( obj = has_campfire_fuel( ch ) ) == NULL )
        return; 

    send( *ch->array, "%s arranges %s in the campfire pit.\n\r", ch, obj );
    send( ch, "You carefully arrange %s in the campfire pit.\n\r", obj );      
    ch->improve_skill( SKILL_CAMPFIRE );
	
    add_queue( new event_data( campfire_three, ch ), 30 );
}

void campfire_three( event_data* event )
{
    char_data*            ch = (char_data*) event->owner;
    obj_data*             obj = NULL;
    obj_data*	          campfire = NULL;
    room_data*            room;
    int                   skill_level = ch->get_skill( SKILL_CAMPFIRE );

    if( ( obj = has_campfire_fuel( ch ) ) == NULL )
        return; 

    send( *ch->array, "%s scrapes a piece of flint, sending showers of sparks onto the wood.\n\r", ch );
    send( ch, "You scrape the piece of flint sending hot sparks down onto the wood. \n\r");
    ch->improve_skill( SKILL_CAMPFIRE );

	if ( number_range( 0,10 ) > skill_level ) {
        send( ch, "You fail to light the campfire. \n\r" );
        send( *ch->array, "%s fails to start the fire.\n\r", ch );
        if ( ch->move < ( 50 - 3*skill_level ) ) {
            ch->move = 0;
            regenerate( ch );
            return;
        } 
        ch->move = ch->move - ( 50 - 3*skill_level );
        regenerate( ch );
		return;
    }
        
    send( *ch->array, "The campfire ignites!\n\r" );       
    campfire = create( get_obj_index( 3412 ) );
    if ( campfire == NULL ) {
        code_bug( "Oh no! There is no campfire object! vnum = 3412" );
        return;
    }
        
	campfire->value[2] = skill_level;
    room = ch->in_room;
	campfire->To( room );

    if( ( campfire = find_vnum( *ch->array, 3828 ) ) == NULL ) {
        code_bug( "Did not find pit in campfire_three?!" );
        return;
    }
    else {
        campfire->Extract ( );
    }

    if ( ch->move < ( 50 - 3*skill_level ) ) {
        ch->move = 0;
        regenerate( ch );
    } 
    else {
        ch->move = ch->move - ( 50 - 3*skill_level ) ;
        regenerate( ch );  
    }
    
    if( obj == NULL ) {
        code_bug( "Campfire was made, but no fuel to extract!?" );    
        return; 
    }

    if ( obj->pIndexData->vnum == 134 ) {
        obj->value[0] -= 1;
        if ( obj->value[0] < 1 )
            obj->Extract( );
        return;
    }

    obj->Extract( );

    return;
}


void do_make( char_data* ch, char* argument ){

    if( *argument == '\0' ) {
        send( "Make what?\n\r", ch );
        return;
    }

    if( !strcasecmp( argument, "campfire" ) ) {
        do_campfire( ch , argument );
        return;
    }
    send( "Make what?\n\r", ch );
    return;
}

obj_data* has_campfire_fuel( char_data* ch ) {

    obj_data*    obj = NULL;

    if ( ( obj = find_type( ch->contents, ITEM_FUEL ) ) != NULL ) 
       return obj;
         
    send( ch, "You don't have anything to burn.\n\r" );
    return NULL;
}
