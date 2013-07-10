#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include "define.h"
#include "struct.h"

void stop_protect( char_data* ch );

void do_hands( char_data* ch, char* argument )
{
  char_data*  victim;
  int           heal;
  int             hp;
 
  if( ch->species != NULL || ch->pcdata->clss != CLSS_PALADIN ) {
    send( ch, "Only paladins can lay hands to heal.\n\r" );
    return;
    }

  if( ch->shdata->skill[SKILL_LAY_HANDS] == 0 ) {
    send( ch, "You don't know that skill.\n\r" );
    return;
    } 

  if( *argument == '\0' ) {
    victim = ch;
    }
  else {
    if( ( victim = one_character( ch, argument, "lay hands on",
      ch->array ) ) == NULL )
      return;
    }

  if( ch->fighting != NULL ) {
    send( ch, "You can't lay hands while fighting!\n\r" );
    return;
    }
    
  if( victim->fighting != NULL ) {
    send( ch, "You can't lay hands on someone who is fighting!\n\r" );
    return;
    }

  if( ch->mana < 2 ) {
    send( ch, "You don't have enough energy to do that.\n\r" );
    return;
    }

  if( ( hp = victim->max_hit-victim->hit ) == 0 ) {
    if( ch == victim ) 
      send( ch, "You aren't hurt.\n\r" );
    else 
      send( ch, "%s is not hurt.\n\r", victim );
    return;
    }

  if( victim == ch ) {
    send( ch, "You lay hands on yourself.\n\r" );
    send_seen( ch, "%s lays hands on %sself.\n\r",
      ch, ch->Him_Her( ) );
    }
  else {    
    send( ch, "You lay hands on %s.\n\r", victim );
    send( victim, "%s lays hands on you.\n\r", ch );
    send_seen( ch, "%s lays hands on %s.\n\r", ch, victim );
    }
  
  heal = min( hp/2, ch->mana );

  victim->hit   = victim->hit+heal*(0.5
     +ch->shdata->skill[SKILL_LAY_HANDS]*0.05
     +ch->shdata->skill[SKILL_REGENERATION]*0.1);
  
  if( hp > heal ) 
    ch->mana -= heal;
    
    else 
      ch->mana -= hp*(ch->shdata->skill[SKILL_LAY_HANDS]*0.05)*
        (ch->shdata->skill[SKILL_REGENERATION]*0.1);
       

  victim->hit = min( victim->max_hit, victim->hit );

  update_pos( victim );

  if( number_range( 0, 1 ) == 0 )
    ch->improve_skill( SKILL_LAY_HANDS );
  else
    if( ch->shdata->skill[SKILL_REGENERATION] > 0 )
      ch->improve_skill( SKILL_REGENERATION );

  return;
}

void do_protect( char_data* ch, char* argument )
{
    char_data*      victim;
    
        
    if( *argument == '\0' ) {
        if( ch->protecting != NULL )
            send( ch, "You are protecting %s's life.\n\r", ch->protecting );
        else
            send( ch, "Protect who?\n\r" );
        return;
    }
    if ( !strcasecmp( argument, "none" ) ) {
        if ( ch->protecting == NULL ) 
            return;
        if( ch->protecting->link == NULL ) {
            ch->protecting->protected_by = NULL;
            ch->protecting = NULL;
            return;
            }
        send( ch, "You stop protecting %s's life.\n\r" , ch->protecting );
        send( ch->protecting, "%s has stopped protecting your life.\n\r" , ch );
        stop_protect( ch );
        return;
    }
    
    if ( ( victim = one_character( ch, argument, "protect", ch->array ) ) == NULL ) 
        return;
    if ( victim->species != NULL) {
        send( ch, "You cannot protect mobs.\n\r" );
        return;
    }
    if ( ch->protecting != NULL ) {
        send( ch, "You are already protecting %s.\n\r", ch->protecting );
        return;
    }
    if ( victim == ch ) {
        send( ch, "You are a sick, funny little paladin.\n\r" );
        return;
    }
    if ( victim->protected_by != NULL ) {
        send( ch, "That person is being protected by %s.\n\r", victim->protected_by );
        return;
    }
    //if ( (victim->leader != ch->leader) ) {
    //    send( ch, "You are not grouped with that person.\n\r" );
    //    return;
    //}
    if ( ch->get_skill(SKILL_PROTECT) == 0 ) {
        send( ch, "You don't know how to protect someone.\n\r" );
        return;
    }
    send( ch, "You dedicate yourself to protecting %s's life.\n\r" ,victim );
    send( victim, "%s has dedicated %sself to protecting your life.\n\r" ,ch, ch->Him_Her( ) );
    victim->protected_by = ch;
    ch->protecting = victim;

    return;

}

void stop_protect( char_data* ch )
{
    ch->protecting->protected_by = NULL;
    ch->protecting = NULL;
    return;
}






