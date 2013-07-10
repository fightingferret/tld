#include <sys/types.h>
#include <stdio.h>
#include <syslog.h>
#include "define.h"
#include "struct.h"


void char_bash   ( char_data*, char_data* );
void obj_bash    ( char_data*, obj_data*, obj_data* );
void door_bash   ( char_data*, exit_data* );

void do_bash( char_data* ch, char* argument )
{
    char            arg  [ MAX_INPUT_LENGTH ];
    char_data*   victim;
    exit_data*     door;
    obj_data*       obj;
    obj_data*  bludgeon;
    thing_data*   thing;

    if( is_confused_pet( ch )  || is_entangled( ch, "bash" ) )
        return;

    if( ch->mount != NULL ) {
        remove_bit( &ch->status, STAT_WIMPY );
        leave_shadows( ch );
        send( ch, "You fall off your mount, quite ungracefully.\n\r" );
        send( *ch->array, "%s falls from %s mount, and lands on %s face.\n\r", ch, ch->His_Her( ), ch->His_Her( ) );
        dismount( ch );
        set_delay( ch, 32 );
        return;
    }
    
    if( argument[0] == '\0' )
    {
        if( ( victim = ch->fighting ) == NULL ) {	
	        send( ch, "Whom do you want to bash?\n\r" );
	        return;
	    }
        update_pos( victim );
        if( ( victim->position > POS_DEAD ) && ( victim->in_room = ch->in_room ) )
            char_bash( ch, victim );
        else
            ch->fighting = NULL;
        return;
    }

    argument = one_argument( argument, arg ); 
  
    thing = one_thing( ch, arg, "bash", ch->array, (thing_array*) &ch->in_room->exits, &ch->contents );
      
    if( (victim = character( thing ) ) != NULL )
    {
        update_pos(victim);
        if( ( victim->position > POS_DEAD ) && ( victim->in_room = ch->in_room ) )
            char_bash( ch, victim );
        else
            ch->fighting = NULL;
        return;
    }
      
    if( (door = exit( thing ) ) != NULL )
    {
        door_bash( ch, door );
        return;
    }
  
    argument = one_argument( argument, arg );
  
    if( !strncasecmp( arg, "with", 4 ) )
        strcpy( arg, argument );

    if( (obj = object( thing )) != NULL)
        if( (bludgeon = one_object( ch, arg, "bash with", &ch->contents, &ch->wearing)) != NULL) {
	        obj_bash( ch, obj, bludgeon );
	        return;
        }
        else
            return;

    if( thing != NULL )
        send( ch, "You can't bash that\n\r");
  
    return;
}


/*
 *   CHARACTER BASH FUNCTION
 */ 

typedef long    dword; 
void char_bash( char_data* ch, char_data* victim )
{
    // mprog comes from the mob, bash_prog is a temporary thing
    mprog_data    *mprog  = NULL, *bash_prog = NULL;
    // bash_val used for making rash decisions about what to include
    // or exclude for checking, where to execute the mprog, and
    // when to abort bash
    dword         bash_val=0;

    if( is_set( ch->affected_by, AFF_BLIND ) ) {
        remove_bit( &ch->status, STAT_WIMPY );
        leave_shadows( ch );
        send( ch, "You fall flat on your face.\n\r" );
        send( *ch->array, "%s falls flat on %s face.\n\r", ch, ch->His_Her( ) );
        ch->position = POS_RESTING;
        set_delay( ch, 32 );
        return;
    }

    // if its a mob being bashed, check its mprogs to see if it has one
    // with trigger type bash (characters obviously don't have mprogs)
    if( victim->species != NULL ) {
        for( mprog = victim->species->mprog; mprog != NULL; mprog = mprog->next ) {
            if( mprog->trigger == MPROG_TRIGGER_BASH ) {
                bash_prog=mprog;
                bash_val=mprog->value;
                break;
            }
        }
    }

    if( victim == ch ) {
        send( ch, "You attempt to bash yourself but instead just fall down.\n\r" );
        send( *ch->array, "%s attempts to bash %sself but is unsuccessful, what a failure!\n\r", ch, victim->Him_Her( ) );
        ch->position = POS_RESTING;
        return;
    }

    // along the way check trigger value, execute or abort as requred
    if( ( bash_val & (1<<5) ) ) {
        var_ch   = ch;
        var_mob  = victim;
        var_room = ch->in_room;
        execute( bash_prog );
        if( ( bash_val & (1<<0) ) )
            return;
    }

    if( !( bash_val & (1<<1) ) ) 
        if( !can_kill( ch, victim ) ) 
            return;
 
    if( ( bash_val & (1<<6) ) ) {
        var_ch   = ch;
        var_mob  = victim;
        var_room = ch->in_room;
        execute( bash_prog );
        if( ( bash_val & (1<<0) ) )
            return;
    }

    if( !( bash_val & (1<<2) ) ) {
        if( victim->species != NULL ) {
            if( is_set( &victim->species->act_flags, ACT_NO_BASH ) ) {
                send( ch, "Bashing that does not make sense.\n\r" );
                return;
            }
            if( is_set( &victim->species->act_flags, ACT_GHOST ) ) {
                send( ch, "Bashing a ghost is a completely futile exercise.\n\r" );
                return;
            }
        }
    }

    if( ( bash_val & (1<<7) ) ) {
        var_ch   = ch;
        var_mob  = victim;
        var_room = ch->in_room;
        execute( bash_prog );
        if( ( bash_val & (1<<0) ) )
            return;
    }

    if( !( bash_val & (1<<3) ) ) 
        if( victim->position < POS_FIGHTING ) {
            send( ch, "Your victim is already on the ground!\n\r" );
            return;
        }

    if( ( bash_val & (1<<8) ) ) {
        var_ch   = ch;
        var_mob  = victim;
        var_room = ch->in_room;
        execute( bash_prog );
        if( ( bash_val & (1<<0) ) )
            return;
    }

    if( !( bash_val & (1<<4) ) ) {
        if( victim->Size( ) > ch->Size( )+1 ) {
            send( ch, "%s is way too large for you to successfully bash %s.\n\r", victim, victim->Him_Her( ) );
            return;
        }
    }

    if( ( bash_val & (1<<9) ) ) {
        var_ch   = ch;
        var_mob  = victim;
        var_room = ch->in_room;
        execute( bash_prog );
        if( ( bash_val & (1<<0) ) )
            return;
    }

    check_killer( ch, victim );
    ch->fighting = victim;
    react_attack( ch, victim );  

    remove_bit( &ch->status, STAT_WIMPY );
    leave_shadows( ch );

    ch->improve_skill( SKILL_BASH );

    if( ( bash_val & (1<<10) ) )
        return;

    set_delay( ch, bash_attack( ch, victim ) );
}


int bash_attack( char_data* ch, char_data* victim )
{
    int                 roll;
    char_data*          rch;
    char                buf  [ MAX_STRING_LENGTH ];

//  roll = number_range( 0, 25 ) - (victim->Size( ) - ch->Size( ) ) - ( victim->shdata->dexterity - ch->shdata->dexterity)/2 - (victim->shdata->level - ch->shdata->level)/5;

    roll = number_range( 0, 25 ) - (victim->Size() - ch->Size( ) )*2 -
           ( victim->Dexterity( ) - ch->Dexterity( ) )*2 -
           ( victim->shdata->level - ch->shdata->level )/2;

    if( ch->move < 4 ) {
        send( ch, "You are too exhausted to bash!\n\r" );
        return 32;
    }
    ch->move -= 5;
    ch->move = max( ch->move, 0 );

    if( ch->species == NULL )  
        roll += ch->get_skill( SKILL_BASH );
    else
        roll += 2;

    if( roll < 4 ) {  
        if( number_range( 0, 26 ) < ch->shdata->dexterity )	{
        	send( ch, "You attempt to bash %s, but miss and fall down.\n\r", victim );
        	send( victim, "%s attempts to bash you, but %s misses and falls down.\n\r",	ch, ch->He_She( victim ) );
        	send( *ch->array, "%s attempts to bash %s, but %s misses and falls down.\n\r", ch, victim, ch->He_She( ) );
        	ch->position = POS_RESTING;
            ch->move -= 15;
            ch->move = max( ch->move, 0 );
	    }
        else {
	        send( ch, "You attempt to bash %s but are unsuccessful.\n\r", victim );
	        send( victim, "%s attempts to bash you but is unsuccessful.\n\r", ch );
	        send( *ch->array, "%s attempts to bash %s but is unsuccessful.\n\r", ch, victim );
	    }
        return 32;
    }
  
    if( roll > 20 ) {

        for( int i = 0; i < *ch->array; i++ ) {
            if( ( rch = character( ch->array->list[i] ) ) != NULL) {
                sprintf( buf, "%s sends %s sprawling!!\n\r", ch->Name( rch, 1, TRUE ), victim->Seen_Name( rch ) );
                if( rch->species == NULL ) {
                    if ( (rch != ch) && (rch != victim) && ( rch->position > POS_SLEEPING ) && (!is_set( rch->affected_by, AFF_BLIND ) ) ) {
                        send_color(rch, COLOR_BASH_OTHER , buf);
                    }
                }
            }
        }

        if( victim->species == NULL ){
            if( victim->mount != NULL )
                dismount( victim );
            sprintf( buf, "%s sends YOU sprawling!!\n\r", ch->Name( victim, 1, TRUE ) );
            send_color( victim, COLOR_BASH_TO , buf );
        }
        if( ch->species == NULL ){
            sprintf( buf, "You send %s sprawling!!\n\r", victim->Name( ch, 1, TRUE ) );
            send_color( ch, COLOR_BASH_FROM , buf );
        }
        disrupt_spell( victim ); 
        set_delay( victim, 32 );
        victim->position = POS_RESTING;
        return 20;
    }
  
    if( roll < 15 ) {
        send( ch, "You attempt to bash %s but fail.\n\r", victim );
        send( victim, "%s attempts to bash you but fails.\n\r", ch );
        send( *ch->array, "%s attempts to bash %s but fails.\n\r", ch, victim );
        return 20;
    }
  
    send( ch, "You attempt to bash %s, but are knocked down yourself.\n\r", victim );
    send( victim, "%s attempts to bash you, but you knock %s down instead.\n\r", ch, ch->Him_Her( ) );
    send( *ch->array, "Attempting to bash %s, %s is knocked down.\n\r", victim, ch ); 
    ch->move -= 15;
    ch->move = max( ch->move, 0 );
    ch->position = POS_RESTING;
  
    return 32;
}


/*
 *   OBJECT BASH ROUTINE
 */


void obj_bash( char_data* ch, obj_data*, obj_data* )
{
    send( ch, "You cannot bash objects.\n\r" );
    return;
}

void door_bash( char_data* ch, exit_data* door )
{
    send( ch, "You take a running step into the door but succeed only in bruising yourself.\n\r" );
    return;
}






