#include "ctype.h"
#include "sys/types.h"
#include "stdio.h"
#include "stdlib.h"
#include "define.h"
#include "struct.h"


/*
 *   FIGHT ROUTINES
 */
void* code_start_fight( void** argument )
{
  char_data*      ch  = (char_data*) argument[0];
  char_data*  victim  = (char_data*) argument[1];

  if( ch == NULL || victim == NULL ) {
//    code_bug( "Code_start_fight: Null ch or victim." );
    return (void*) FALSE;
    }

//  check_killer( ch, victim );
//  ch->fighting = victim;
  init_attack( ch, victim );
  //react_attack( ch, victim );

  //fight_round( ch );
}

  


void* code_attack_weapon( void** argument )
{
  char_data*      ch  = (char_data*) argument[0];
  char_data*  victim  = (char_data*) argument[1];
  int            dam  = (int)        argument[2];
  char*       string  = (char*)      argument[3];
  obj_data*    wield;
  int              i;

  if( string == NULL ) {
    code_bug( "Code_Attack_Weapon: NULL attack string." );
    return NULL;
    }

  if( ch == NULL || victim == NULL 
    || ch->in_room != victim->in_room
    || ch->position < POS_FIGHTING ) 
    return NULL;

  wield = ch->Wearing( WEAR_HELD_R );

  push( );

  i = attack( ch, victim, wield == NULL ? string
    : weapon_attack[ wield->value[3] ], wield,
    wield == NULL ? dam : -1, 0 );

    if( ch != NULL && victim != NULL 
       && ch->in_room == victim->in_room ) {
      //ch->fighting = victim;
      victim->aggressive += ch;
      react_attack( victim, ch );
      react_attack( ch, victim );
//      ch->fighting = victim;
//      if( victim->fighting == NULL )
//        victim->fighting = ch;
      }

  pop( );

  return (void*) i;
}


void* code_attack_room( void** argument )
{
  char_data*        ch  = (char_data*) argument[0];
  int              dam  = (int)        argument[1];
  char*         string  = (char*)      argument[2];
  char_data*       rch;

  if( string == NULL ) {
    code_bug( "Code_Attack_Room: NULL attack string." );
    return NULL;
    }

  if( ch == NULL || ch->position < POS_SLEEPING ) 
    return NULL;

  for( int i = *ch->array-1; i >= 0; i-- ) {
    if( ( rch = character( ch->array->list[i] ) ) != NULL
      && rch != ch && ( rch->pcdata != NULL
      || is_set( &rch->status, STAT_PET ) ) ) {
      damage_physical( rch, ch, dam, string );
      }
    }

  return NULL;
}


void* code_attack( void** argument )
{
    char_data*      ch  = (char_data*) argument[0];
    char_data*  victim  = (char_data*) argument[1];
    int            dam  = (int)        argument[2];
    char*       string  = (char*)      argument[3];
    int              i;

    if( string == NULL ) {
        code_bug( "Code_Attack: NULL attack string." );
        return NULL;
    }

    if( ch == NULL || victim == NULL || ch->in_room != victim->in_room || ch->position < POS_FIGHTING ) 
        return NULL;

    push( );

    i = attack( ch, victim, string, NULL, dam, 0 );

    if( ch != NULL && victim != NULL
       && ch->in_room == victim->in_room ) {
      //ch->fighting = victim;
      victim->aggressive += ch;
      react_attack( victim, ch );
      react_attack( ch, victim );
//      ch->fighting = victim;
//      if( victim->fighting == NULL )
//        victim->fighting = ch;
      }

    pop( );

    return (void*) i;
}


/*
 *   ELEMENTAL ATTACKS
 */


void* element_attack( void** argument, int type )
{
  char_data*      ch  = (char_data*) argument[0];
  char_data*  victim  = (char_data*) argument[1];
  int            dam  = (int)        argument[2];
  char*       string  = (char*)      argument[3];
  int              i;

  if( string == NULL ) {
    code_bug( "Attack: Null string." );
    return NULL;
    }

  if( ch == NULL || victim == NULL
    || ch->in_room != victim->in_room
    || ch->position < POS_FIGHTING ) 
    return NULL;

  push( );

  i = attack( ch, victim, string, NULL, dam, 0, type );

  pop( );

  return (void*) i;
}


#define ea( i )  element_attack( arg, i )

void* code_attack_acid  ( void** arg ) { return ea( ATT_ACID ); }
void* code_attack_cold  ( void** arg ) { return ea( ATT_COLD ); }
void* code_attack_shock ( void** arg ) { return ea( ATT_SHOCK ); }
void* code_attack_fire  ( void** arg ) { return ea( ATT_FIRE ); }

#undef ea( i )


/*
 *   DAMAGE ROUTINES
 */ 


void* code_inflict( void** argument )
{
  char_data* victim  = (char_data*) argument[0];
  char_data*     ch  = (char_data*) argument[1];
  int           dam  = (int)        argument[2];
  char*      string  = (char*)      argument[3];
  
  if( victim != NULL )
    inflict( victim, ch, dam, string );

  return NULL;
}


void* element_inflict( void** argument, int type )
{
  char_data*  victim  = (char_data*) argument[0];
  char_data*      ch  = (char_data*) argument[1];
  int            dam  = (int)        argument[2];
  char*       string  = (char*)      argument[3];

  if( string == NULL ) {
    code_bug( "Inflict: Null string." );
    return NULL;
    }

  if( victim != NULL )
// SADIS - softcode inflict by element
    elemental_inflict( victim, ch, dam, string, type );

  return NULL;
}


#define ei( i )  element_inflict( arg, i )

void* code_inflict_acid  ( void** arg ) { return ei( ATT_ACID ); }
void* code_inflict_cold  ( void** arg ) { return ei( ATT_COLD ); }
void* code_inflict_shock ( void** arg ) { return ei( ATT_SHOCK ); }
void* code_inflict_fire  ( void** arg ) { return ei( ATT_FIRE ); }

#undef ei( i )



void* code_dam_message( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];
  int        dam  = (int)        argument[1];
  char*   string  = (char*)      argument[2];
  
  if( ch == NULL )
    return NULL;

  dam_message( ch, NULL, dam, string, lookup( physical_index, dam ) );

  return NULL;
}

// BEGIN BY SADIS -- damage messages for elemental inflict
void* code_dam_element( void** argument )
{
  char_data*    ch = (char_data*) argument[0];
  int           dam = (int)       argument[1];
  char*         string = (char*)  argument[2];
  int           damtype = (int)   argument[3];

  if( ch == NULL )
    return NULL;
  switch( damtype ) {
  case 1:
    dam *= 100-ch->Save_Fire( );
    dam /= 100;
    dam_message( ch, NULL, dam, string, lookup( fire_index, dam ) );
    break;
  case 2:
    dam *= 100-ch->Save_Cold( );
    dam /= 100;
    dam_message( ch, NULL, dam, string, lookup( cold_index, dam ) );
    break;
  case 3:
    dam *= 100-ch->Save_Acid( );
    dam /= 100;
    dam_message( ch, NULL, dam, string, lookup( acid_index, dam ) );
    break;
  case 4:
    dam *= 100-ch->Save_Shock( );
    dam /= 100;
    dam_message( ch, NULL, dam, string, lookup( electric_index, dam ) );
    break;
  }

  return NULL;
}

