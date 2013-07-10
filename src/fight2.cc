#include <sys/types.h>
#include <stdio.h>
#include <syslog.h>
#include "define.h"
#include "struct.h"


void    disarm          ( char_data*, char_data* );


/*
 *   FIGHT SUB_ROUTINES
 */


void check_killer( char_data* ch, char_data* victim )
{
  if( ch->species != NULL || !is_enemy( victim, ch ) ) 
    return;
    
  add_enemy( victim, ch );
}


bool is_entangled( char_data* ch, const char* verb )
{
  if( !is_set( ch->affected_by, AFF_ENTANGLED ) )
    return FALSE;

  send( ch, "You are too entangled to %s anything.\n\r", verb );

  return TRUE;
}


/*
 *   SUPPORT SUBROUTINES
 */


char_data* get_victim( char_data* ch, char* argument, char* msg )
{
  char_data*    victim;
  char_data*  opponent;
  /*
  opponent = ch->fighting;

  if( *argument == '\0' ) {
    if( opponent == NULL ) {
      if( msg != empty_string )
        send( msg, ch );
      return NULL;
      }
    return opponent;
    }

  if( ( victim = get_char_room( ch, argument ) ) == NULL ) {
    if( msg != empty_string )
      send( ch, "They aren't here.\n\r" );
    return NULL;
    }

  if( opponent != NULL && opponent != victim ) {
    send( ch, "You are already fighting someone else.\n\r" );
    return NULL;
    }
  */
  return victim;
}


/*
 *   DO_KILL FUNCTION
 */
    

void do_kill( char_data* ch, char* argument )
{
  char_data*    victim;
  char_data*  opponent;

  if( ( victim = one_character( ch, argument, "kill", ch->array ) ) == NULL ){
        //send( ch, "Kill what?\n\r" );
    return;
  }
  if( victim == ch ) {
    send( ch, "Typing quit is easier.\n\r" );
    return;
    }

  if( ch->mount != NULL && ch->Wearing( WEAR_HELD_R ) == NULL ) {
    send( ch, "You can't attack without a weapon while mounted.\n\r" );
    return;
    }

  if( ch->fighting == victim ) {
    send( ch, "You are already attacking %s!\n\r", victim );
    return;
    }

  if( !can_kill( ch, victim ) )
    return;

  check_killer( ch, victim );
  ch->fighting = victim;
  react_attack( ch, victim );

  remove_bit( &ch->status, STAT_WIMPY );

  fight_round( ch );
}


/*
 *   DISARM
 */


void do_disarm( char_data* ch, char* )
{
  char_data*    victim;
  obj_data*        obj;
  int          percent;

  if( is_confused_pet( ch ) )
    return;

  if( ch->pcdata != NULL 
    && ch->shdata->skill[ SKILL_DISARM ] == UNLEARNT ) {
    send( ch, "You don't know how to disarm opponents.\n\r" );
    return;
    }

  if( ch->Wearing( WEAR_HELD_R ) == NULL ) {
    send( ch, "You must wield a weapon to disarm.\n\r" );
    return;
    }

  if( ( victim = ch->fighting ) == NULL ) {
    send( "You aren't fighting anyone.\n\r", ch );
    return;
    }

  if( ( obj = victim->Wearing( WEAR_HELD_R ) ) == NULL ) {
    send( "Your opponent is not wielding a weapon.\n\r", ch );
    return;
    }

  send( ch, "Command disabled.\n\r" );

  return;
}


void disarm( char_data* ch, char_data* victim )
{
  /*
  obj_data* obj;

  if( ( obj = victim->Wearing( WEAR_HELD_R ) ) == NULL )
    return;

  if( is_set( obj->pIndexData->extra_flags, OFLAG_NO_DISARM ) ) { 
    send( ch, "The weapon %s is wielding is impossible to disarm.\n\r",
      victim->Name( ch ) );
    return;
    }

  percent = number_range( 0, 100 )+victim->shdata->level-ch->shdata->level;
  if( IS_NPC(ch) || percent < ch->shdata->skill[ SKILL_DISARM ]*2/3 )
    disarm( ch, victim );
  else
    send( ch, "You failed.\n\r" );
 
  if( get_eq_char( ch, WEAR_HELD_R ) == NULL && number_range( 0, 1 ) == 0 )
    return;


  send( victim, "+++ %s disarms you! +++\n\r", ch );
  send( ch, "You disarm %s!\n\r", victim );
  send( *victim->array, "%s disarms %s!\n\r", ch, victim );

  obj = obj_from_char( obj, 1 );

  if( victim->species != NULL ) {
    obj_to_char( obj, victim );
    }
  else {
    obj_to_room( obj, victim->in_room );
    }
*/

  return;
}


/*
 *   KICK, PUNCH, BITE ROUTINES
 */


void do_punch( char_data* ch, char* argument )
{
  char_data*     victim;
  char_data*     tmpvict;

  if(  is_confused_pet( ch )
    || !is_humanoid( ch )
    || is_mounted( ch )
    || is_entangled( ch, "punch" ) ) 
    return;

  if( *argument == '\0' ) {
    if( ch->fighting == NULL ) {
      send(ch, "Punching nothing is unproductive.\n\r");
      return;
      }
      else {
        victim = ch->fighting;
      }
    }
    else {
      if( ( tmpvict = one_character( ch, argument, "punch",
                      ch->array ) ) == NULL ) {
        return;
        }
        else {
          if( ( ch->fighting != '\0' ) && ( ch->fighting != tmpvict ) ) {
            send( ch, "You are too busy to worry about %s.\n\r", argument
);
            return;
            }
            else {
              victim = tmpvict;
            }
          }
    }

  if( victim == ch ) {
    send( ch, "Punching yourself is pointless.\n\r" );
    return;
    }

  if( ch->pcdata != NULL && ch->shdata->skill[ SKILL_PUNCH ] == UNLEARNT ) {
    send( ch, "You are untrained in the art of punching.\n\r" );
    return;
    }

  if( !can_kill( ch, victim ) )
    return;

  check_killer( ch, victim );

  ch->improve_skill( SKILL_PUNCH );

  attack( ch, victim, "punch", NULL, roll_dice( 1,4 ), 0 );

  update_pos(victim);
  if((victim->position > POS_DEAD) && (victim->in_room == ch->in_room))
  {
    ch->fighting = victim;
    react_attack( ch, victim );
    add_queue( &ch->active, 20 );
  }
  else
    ch->fighting = NULL;
  return;
}


void do_bite( char_data* ch, char* argument )
{
  char_data*     victim;
  char_data*     tmpvict;

  if(  is_confused_pet( ch )
    || is_mounted( ch )
    || is_entangled( ch, "bite" ) ) 
    return;

  if( *argument == '\0' ) {
    if( ch->fighting == NULL ) {
      send(ch, "Biting nothing is unproductive.\n\r");
      return;
      }
      else {
        victim = ch->fighting;
      }
    }
    else {
      if( ( tmpvict = one_character( ch, argument, "bite",
                      ch->array ) ) == NULL ) {
        return;
        }
        else {
          if( ( ch->fighting != '\0' ) && ( ch->fighting != tmpvict ) ) {
            send( ch, "You are too busy to worry about %s.\n\r", argument
);
            return;
            }
            else {
              victim = tmpvict;
            }
          }
    }

  if( victim == ch ) {
    send( ch, "Biting yourself is pointless.\n\r" );
    return;
    }

  if(  ch->shdata->race != RACE_LIZARD
    && ch->shdata->race != RACE_TROLL
    && ch->shdata->race != RACE_GOBLIN ) {
    send( ch, "Your teeth are not long or sharp enough for biting to be an\
 effective\n\rattack.\n\r" );
    return;
    }

  if( !can_kill( ch, victim ) )
    return;

  attack( ch, victim, "bite", NULL, roll_dice( 1,4 ), 0 );

  check_killer( ch, victim );
  update_pos(victim);
  if((victim->position > POS_DEAD) && (victim->in_room == ch->in_room))
  {
    ch->fighting = victim;
    react_attack( ch, victim );
    add_queue( &ch->active, 20 );
  }
  else
    ch->fighting = NULL;

  return;
}


/*
 *   SPIN KICK ROUTINE
 */



void do_spin_kick( char_data* ch, char* )
{
  char_data*       rch;
  char_data*  rch_next;
  bool           found  = FALSE;

  if( is_confused_pet( ch ) || is_mounted( ch )
    || !is_humanoid( ch ) )
    return;

  if( ch->pcdata == NULL
    || ch->shdata->skill[SKILL_SPIN_KICK] == UNLEARNT ) {
    send( "You are untrained in that skill.\n\r", ch );
    return;
    }
  
  for( int i = 0 ; i < ch->in_room->contents ; i++ ) {
    if( ch->in_room->contents[i] != NULL ) {
      if( ( rch = character( ch->in_room->contents[i] ) ) != NULL )
        if( rch == ch )
          continue;
        if( rch != NULL && ch != NULL )
          if( rch->fighting == ch || ch->fighting == rch )
            found = TRUE;
      }
    }

  if( found ) {
    send( ch, "You leap in the air, spinning rapidly.\n\r" );
    send_seen( ch, "%s leaps into the air, becoming a deadly blur.\n\r", ch );
    ch->improve_skill( SKILL_SPIN_KICK );
    }
  else {
    send( ch, "You aren't in combat.\n\r" );
    return;
    }

  for( int i = 0 ; i < ch->in_room->contents ; i++ ) {
    if( ch->in_room->contents[i] != NULL ) {
      if( ( rch = character( ch->in_room->contents[i] ) ) != NULL )
        if( rch == ch )
          continue;
        if( rch != NULL && ch != NULL )
          if( rch->fighting == ch || ch->fighting == rch )
            attack( ch, rch, "spin kick", NULL,
              roll_dice( 5, 10+ch->shdata->skill[SKILL_SPIN_KICK] ), 0 );
      }
    }
/*
  for( int i = *ch->array-1; i >=0; i++ ) {
    if( ( rch = character( ch->array->list[i] ) ) != NULL
      && ( rch->fighting == ch || ch->fighting == rch ) ) {
      if( !found ) {
        found = TRUE;
        send( ch, "You leap in the air, spinning rapidly.\n\r" );
        send_seen( ch,
          "%s leaps into the air, becoming a deadly blur.", ch );
        }
      attack( ch, rch, "spin kick", NULL, 
        roll_dice( 5, 10+ch->shdata->skill[SKILL_SPIN_KICK] ), 0 );
      }
    }
*/

  remove_bit( &ch->status, STAT_WIMPY );
  remove_bit( &ch->status, STAT_LEAPING );

  set_delay( ch, 25 );

  return;
}


void do_melee( char_data* ch, char* )
{
  char_data*       rch;
  char_data*  rch_next;
  bool           found  = FALSE;
  /*
  if( is_confused_pet( ch ) || is_mounted( ch )
    || !is_humanoid( ch ) )
    return;

  if( ch->pcdata == NULL
    || ch->shdata->skill[SKILL_MELEE] == UNLEARNT ) {
    send( ch, "You are untrained in that skill.\n\r" );
    return;
    }
  
  ch->improve_skill( SKILL_MELEE );

  for( rch = ch->in_room->people; rch != NULL; rch = rch_next ) {
    rch_next = rch->next_in_room;
    if( rch->fighting == ch || ch->fighting == rch ) {
      if( !found ) {
        found = TRUE;
        send( ch, "You spin about, striking at all your foes!!\n\r" );
        send_seen( ch, "%s spins, striking out at all %s foes!!\n\r",  
          ch, ch->Him_Her( ) ); 
        }
      attack( ch, rch, "MELEE", NULL, 
        roll_dice( 5, 8+ch->shdata->skill[SKILL_MELEE] ), 0 );
      }
    }

  if( !found )
    send( "You are not fighting anyone!\n\r", ch );

  remove_bit( &ch->status, STAT_WIMPY );
  remove_bit( &ch->status, STAT_LEAPING );

  set_delay( ch, 30 );
  */
  return;
}







