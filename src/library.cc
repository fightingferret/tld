#include "ctype.h"
#include "sys/types.h"
#include "stdio.h"
#include "stdlib.h"
#include "define.h"
#include "struct.h"


const char* arg_type_name[] = { "none", "any", "string", "integer",
  "character", "object", "room", "direction", "clan", "skill", "rflag",
  "stat", "class", "religion", "race", "thing" };


/*
 *   FUNCTION TABLE
 */


#define nn NONE
#define st STRING
#define ch CHARACTER
#define ob OBJECT
#define in INTEGER
#define rm ROOM
#define sk SKILL
#define sa STAT
#define di DIRECTION
#define rf RFLAG
#define na NATION
#define cl CLASS
#define re RELIGION
#define rc RACE
#define th THING

const class cfunc_type cfunc_list [] =
{
  { "acode",           &code_acode,         nn, { rm, in, nn, nn } },
  { "act_area",        &code_act_area,      nn, { st, ch, ob, ch } },
  { "act_neither",     &code_act_neither,   nn, { st, ch, ob, ch } },
  { "act_notchar",     &code_act_notchar,   nn, { st, ch, ob, ch } },
  { "act_notvict",     &code_act_notchar,   nn, { st, ch, ob, ch } },
  { "act_tochar",      &code_act_tochar,    nn, { st, ch, ob, ch } },
  { "act_room",        &code_act_room,      nn, { st, ch, ob, ch } },
  { "add",             &code_add,           in, { in, in, nn, nn } },
  { "arena_watch",     &code_arena_watch ,  nn, { ch, rm, nn, nn } },
  { "assign_quest",    &code_assign_quest,  nn, { ch, in, nn, nn } },
  { "attack",          &code_attack,        in, { ch, ch, in, st } },
  { "attack_acid",     &code_attack_acid,   in, { ch, ch, in, st } },
  { "attack_cold",     &code_attack_cold,   in, { ch, ch, in, st } },
  { "attack_fire",     &code_attack_fire,   in, { ch, ch, in, st } },
  { "attack_room",     &code_attack_room,   nn, { ch, in, st, nn } },
  { "attack_shock",    &code_attack_shock,  in, { ch, ch, in, st } },
  { "attack_weapon",   &code_attack_weapon, in, { ch, ch, in, st } },
  { "can_fly",         &code_can_fly,       in, { ch, nn, nn, nn } },
  { "can_see",         &code_can_see,       in, { ch, ch, nn, nn } },
  { "cast_spell",      &code_cast_spell,    nn, { sk, ch, ch, nn } },
  { "char_in_room",    &code_char_in_room,  in, { ch, rm, nn, nn } },
  { "cflag",           &code_cflag,         in, { in, ch, nn, nn } },
  { "class",           &code_class,         cl, { ch, nn, nn, nn } },
  { "close",           &code_close,         nn, { rm, di, nn, nn } },
  { "coin_value",      &code_coin_value,    in, { ob, nn, nn, nn } },
  { "dam_element",     &code_dam_element,   nn, { ch, in, st, in } },
  { "dam_message",     &code_dam_message,   nn, { ch, in, st, in } },
  { "deinvis",         &code_deinvis,       nn, { ch, nn, nn, nn } },
  { "dice",            &code_dice,          in, { in, in, in, nn } },
  { "disarm",          &code_disarm,        in, { ch, in, nn, nn } },
  { "do_spell",        &code_do_spell,      nn, { sk, ch, ch, nn } },
  { "doing_quest",     &code_doing_quest,   in, { ch, in, nn, nn } },
  { "done_quest",      &code_done_quest,    in, { ch, in, nn, nn } },
  { "drain_exp",       &code_drain_exp,     nn, { ch, in, nn, nn } },
  { "drain_stat",      &code_drain_stat,    nn, { ch, sa, in, in } },
  { "find_room",       &code_find_room,     rm, { in, nn, nn, nn } },
  { "find_skill",      &code_find_skill,    in, { ch, sk, nn, nn } },
  { "find_stat",       &code_find_stat,     in, { ch, sa, nn, nn } },
  { "get_pos",         &code_get_pos,       in, { ch, nn, nn, nn } },
  { "get_room",        &code_get_room,      rm, { ch, nn, nn, nn } },
  { "get_skeleton",    &code_get_skeleton,  in, { ch, nn, nn, nn } },
  { "get_undead",      &code_get_undead,    in, { ch, nn, nn, nn } },
  { "junk_mob",        &code_junk_mob,      nn, { ch, nn, nn, nn } },
  { "junk_obj",        &code_junk_obj,      nn, { ob, in, nn, nn } },
  { "has_obj",         &code_has_obj,       ob, { in, ch, nn, nn } },
  { "has_obj_inv",     &code_has_obj_inv,   ob, { in, ch, nn, nn } },
  { "has_obj_worn",    &code_has_obj_worn,  ob, { in, ch, nn, nn } },
  { "has_quest",       &code_has_quest,     in, { ch, in, nn, nn } },
  { "has_status",      &code_has_status,    in, { ch, in, nn, nn } },
  { "heal",            &code_heal,          nn, { ch, in, nn, nn } },
  { "inflict",         &code_inflict,       in, { ch, ch, in, st } },
  { "inflict_acid",    &code_inflict_acid,  in, { ch, ch, in, st } },
  { "inflict_cold",    &code_inflict_cold,  in, { ch, ch, in, st } },
  { "inflict_fire",    &code_inflict_fire,  in, { ch, ch, in, st } },
  { "inflict_shock",   &code_inflict_shock, in, { ch, ch, in, st } },
  { "interpret",       &code_interpret,     nn, { ch, st, ch, nn } },
  { "is_affected",     &code_is_affected,   in, { ch, in, nn, nn } },
  { "is_invis",        &code_is_invis,      in, { ch, nn, nn, nn } },
  { "is_camouflaged",  &code_is_camouflaged,in, { ch, nn, nn, nn } },
  { "is_exhausted",    &code_is_exhausted,  in, { ch, in, nn, nn } },
  { "is_fighting",     &code_is_fighting,   in, { ch, nn, nn, nn } },
  { "is_follower",     &code_is_follower,   in, { ch, nn, nn, nn } },
  { "is_following",    &code_is_following,  in, { ch, ch, nn, nn } },
  { "is_hidden",       &code_is_hidden,     in, { ch, nn, nn, nn } },
  { "is_locked",       &code_is_locked,     in, { rm, di, nn, nn } },
// Sadis -- is_mob( mob, vnum )
  { "is_mob",          &code_is_mob,        nn, { ch, in, nn, nn } },
  { "is_mounted",      &code_is_mounted,    in, { ch, nn, nn, nn } },
  { "is_name",         &code_is_name,       in, { st, st, nn, nn } },
  { "is_open",         &code_is_open,       in, { rm, di, nn, nn } },
  { "is_player",       &code_is_player,     in, { ch, nn, nn, nn } },
  { "is_resting",      &code_is_resting,    in, { ch, nn, nn, nn } },
  { "is_searching",    &code_is_searching,  in, { ch, nn, nn, nn } },
  { "is_silenced",     &code_is_silenced,   in, { ch, nn, nn, nn } },
  { "lock",            &code_lock,          nn, { rm, di, nn, nn } },
  { "num_in_room",     &code_num_in_room,   in, { rm, nn, nn, nn } },
  { "num_mob",         &code_num_mob,       in, { in, rm, nn, nn } },
  { "mload",           &code_mload,         ch, { in, rm, nn, nn } },
  { "mob_in_room",     &code_mob_in_room,   ch, { in, rm, nn, nn } },
  { "modify_mana",     &code_modify_mana,   in, { ch, in, nn, nn } },
  { "modify_move",     &code_modify_move,   in, { ch, in, nn, nn } },
  { "modify_rep",      &code_modify_rep,    in, { ch, na, in, nn } },
  { "mpcode",          &code_mpcode,        nn, { ch, in, nn, nn } },
  { "mult",            &code_mult,          in, { in, in, nn, nn } },
  { "obj_in_room",     &code_obj_in_room,   ob, { in, rm, nn, nn } },
  { "obj_name",        &code_obj_name,      st, { ob, nn, nn, nn } },
  { "obj_to_char",     &code_obj_to_char,   nn, { ob, ch, nn, nn } },
  { "obj_to_room",     &code_obj_to_room,   nn, { ob, rm, nn, nn } },
  { "obj_type",        &code_obj_type,      in, { ob, nn, nn, nn } },
  { "obj_value",       &code_obj_value,     in, { ob, in, nn, nn } },
  { "oload",           &code_oload,         ob, { in, nn, nn, nn } },
  { "opcode",          &code_opcode,        nn, { ob, in, nn, nn } },
  { "open",            &code_open,          nn, { rm, di, nn, nn } },
  { "plague",          &code_plague,        nn, { ch, nn, nn, nn } },
  { "players_area",    &code_players_area,  in, { rm, nn, nn, nn } },
  { "players_room",    &code_players_room,  in, { rm, nn, nn, nn } },
  { "poison",          &code_poison,        nn, { ch, nn, nn, nn } },
  { "race",            &code_race,          rc, { ch, nn, nn, nn } },
  { "random",          &code_random,        in, { in, in, nn, nn } },
  { "rand_char",       &code_rand_char,     ch, { rm, nn, nn, nn } },
  { "rand_player",     &code_rand_player,   ch, { rm, nn, nn, nn } },
  { "rem",             &code_remark,        nn, { st, nn, nn, nn } },
  { "remove_aff",      &code_remove_aff,    nn, { ch, in, nn, nn } },
  { "rem_nonself_aff", &code_rem_not_aff,   nn, { ch, nn, nn, nn } },
  { "remove_cflag",    &code_remove_cflag,  nn, { in, ch, nn, nn } },
  { "remove_rflag",    &code_remove_rflag,  nn, { rf, rm, nn, nn } },
  { "reputation",      &code_reputation,    in, { ch, na, nn, nn } },
  { "rflag",           &code_rflag,         in, { rf, rm, nn, nn } },
  { "sees_invis",      &code_sees_invis,    in, { ch, nn, nn, nn } },
  { "sees_hidden",     &code_sees_hidden,   in, { ch, nn, nn, nn } },
  { "send_to_area",    &code_send_to_area,  nn, { st, rm, nn, nn } },
  { "send_to_char",    &code_send_to_char,  nn, { st, ch, nn, nn } },
  { "send_to_room",    &code_send_to_room,  nn, { st, rm, nn, nn } },
  { "set_align",       &code_set_align,     nn, { ch, in, nn, nn } },
  { "set_cflag",       &code_set_cflag,     nn, { in, ch, nn, nn } },
  { "set_delay",       &code_set_delay,     nn, { ch, in, nn, nn } },
  { "set_religion",    &code_set_religion,  nn, { ch, re, nn, nn } },
  { "set_rflag",       &code_set_rflag,     nn, { rf, rm, nn, nn } },
  { "set_pos",         &code_set_pos,       nn, { ch, in, nn, nn } },
  { "show",            &code_show,          nn, { ch, rm, in, nn } },
  { "show_room",       &code_show_room,     nn, { ch, rm, nn, nn } },
  { "size",            &code_size,          in, { ch, nn, nn, nn } }, 
  { "start_fight",     &code_start_fight,   nn, { ch, ch, nn, nn } },
  { "subt",            &code_subt,          in, { in, in, nn, nn } },
  { "summon",          &code_summon,        nn, { ch, ch, nn, nn } },
  { "tell",            &code_tell,          nn, { ch, ch, st, nn } },
  { "transfer",        &code_transfer,      nn, { ch, rm, nn, nn } },
  { "transfer_all",    &code_transfer_all,  nn, { rm, rm, nn, nn } },
  { "uncamouflage",    &code_uncamouflage,  nn, { ch, nn, nn, nn } },
  { "unhide",          &code_unhide,        nn, { ch, nn, nn, nn } },
  { "unlock",          &code_unlock,        nn, { rm, di, nn, nn } },
  { "update_quest",    &code_update_quest,  nn, { ch, in, nn, nn } },
  { "use_move",        &code_use_move,      nn, { ch, in, nn, nn } },
  { "wait",            &code_wait,          nn, { in, nn, nn, nn } },
  { "weight",          &code_weight,        in, { th, nn, nn, nn } },
  { "",                NULL,                nn, { nn, nn, nn, nn } }
};

#undef nn
#undef st
#undef ch
#undef ob
#undef in
#undef rm
#undef sk
#undef st
#undef di
#undef rf
#undef na
#undef re
#undef th

void* code_set_delay( void** argument )
{
  char_data*  ch = (char_data*) argument[0];
  int          i = (int)        argument[1];

  if( ch == NULL ) {
    code_bug( "Code_set_delay: NULL character" );
    return;
    }

  if( i < 0 || i > 100 ) {
    code_bug( "Code_set_delay: Invalid delay" );
    return;
    }

  add_queue( &ch->active, i );
  return;
}

/*
 *   SIMPLE MATH ROUTINES
 */

void* code_add( void** argument )
{
  int        aint  = (int)        argument[0];
  int        bint  = (int)        argument[1];

  if( ( aint == NULL ) || ( bint == NULL ) ) {
    code_bug( "Code_add: Invalid argument" );
    return NULL;
  }

  return (void*) ( aint + bint );
}

void* code_subt( void** argument )
{
  int        aint  = (int)        argument[0];
  int        bint  = (int)        argument[1];

  if( ( aint == NULL ) || ( bint == NULL ) ) {
    code_bug( "Code_subt: Invalid argument" );
    return NULL;
  }

  return (void*) ( aint - bint );
}

void* code_mult( void** argument )
{
  long        aint  = (long)        argument[0];
  long        bint  = (long)        argument[1];

  if( ( aint == NULL ) || ( bint == NULL ) ) {
    code_bug( "Code_mult: Invalid argument" );
    return NULL;
  }

  return (void*) ( aint * bint );
}

/*
 *   POSITION ROUTINES
 */

   
void* code_is_mounted( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  return (void*) ( ch->mount != NULL );
}

 
void* code_is_resting( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Is_Resting: NULL character." );
    return FALSE;
    }

  return (void*) ( ch->position == POS_RESTING );
}


void* code_is_fighting( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Is_Fighting: NULL character." );
    return (void*) FALSE;
    }

  if( ch->fighting != NULL )
    return (void*) TRUE;

  return (void*) FALSE;
}

// Sadis -- is_mob( mob, vnum )
void* code_is_mob( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  int      vnum = (int) argument[1];

  if( ch == NULL ) {
    code_bug( "Is_mob: NULL target" );
    return (void*) FALSE;
    }

  if( !not_player( ch ) ) {
    code_bug( "Is_mob: Target is not a mobile" );
    return (void*) FALSE;
    }

  if( ch->species->vnum != vnum )
    return (void*) FALSE;

  return (void*) TRUE;
}
 
void* code_is_player( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Is_Player: NULL character." );
    return (void*) FALSE;
    }

  return (void*) ( ch->species == NULL );
}


/*
 *   ROOM ROUTINES
 */


void* code_rflag( void** argument )
{
  int         flag  = (int)        argument[0];
  room_data*  room  = (room_data*) argument[1]; 

  if( room == NULL ) {
    code_bug( "Code_rflag: NULL room" );
    return (void*) FALSE;
    }

  return (void*) is_set( &room->room_flags, flag );
}


void* code_set_rflag( void** argument )
{
  int         flag  = (int)        argument[0];
  room_data*  room  = (room_data*) argument[1]; 

  if( room == NULL ) {
    code_bug( "Code_rflag: NULL room" );
    return NULL;
    }

  set_bit( &room->room_flags, flag );

  return NULL;
}


void* code_remove_rflag( void** argument )
{
  int         flag  = (int)        argument[0];
  room_data*  room  = (room_data*) argument[1]; 

  if( room == NULL ) {
    code_bug( "Code_rflag: NULL room" );
    return NULL;
    }

  remove_bit( &room->room_flags, flag );

  return NULL;
}


void* code_get_room( void** argument )
{
  char_data* ch = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Get_room: NULL character" );
    return NULL;
    }

  return ch->in_room;
}


/*
 *   CHARACTER ROUTINES
 */

void* code_unhide( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  if( ch==NULL ) {
    code_bug( "Unhide: Null argument" );
    return NULL;
  }

  remove_bit( &ch->status, STAT_HIDING );
  return NULL;
}

void* code_uncamouflage( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  if( ch==NULL ) {
    code_bug( "Uncamouflage: Null argument" );
    return NULL;
  }
  remove_bit( &ch->status, STAT_CAMOUFLAGED );
  return NULL;
}

void* code_deinvis( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  if( ch==NULL ) {
    code_bug( "Deinvis: Null argument" );
    return NULL;
  }
  remove_bit( ch->affected_by, AFF_INVISIBLE );
  return NULL;
}

void* code_is_affected( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  int	num_aff = (int) argument[1];

  if( ch==NULL || !num_aff ) {
    code_bug( "Is_affected: Null argument" );
    return NULL;
  }

  if( num_aff > 67 || num_aff < 0 ) {
    code_bug( "Is_affected: Affect number out of range" );
    return NULL;
  }

  return (void*) is_set( ch->affected_by, num_aff );
 }

void* code_has_status( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  int   num_status = (int) argument[1];

  if( ch==NULL || !num_status ) {
    code_bug( "Has_status: Null argument" );
    return NULL;
  }

  if( num_status > 22 || num_status < 0 ) {
    code_bug( "Is_affected: Affect number out of range" );
    return NULL;
  }

  return (void*) is_set( &ch->status, num_status );
 }

void* code_remove_aff( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  int   num_aff = (int) argument[1];

  if( ch == NULL ) {
    code_bug( "Is_hidden: Null argument" );
    return NULL;
    }

  if( num_aff < 0 || num_aff > 67 ) {
    code_bug( "Remove_aff: Affect out of range" );
    return NULL;
    }

  if( is_set( ch->affected_by, num_aff ) ) {
    remove_bit( ch->affected_by, num_aff );
    strip_affect( ch, num_aff );
    }
}

void* code_remove_status( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  int   num_aff = (int) argument[1];

  if( ch==NULL ) {
    code_bug( "Remove_status: Null argument for ch" );
    return NULL;
  }

  if( !num_aff ) {
    code_bug( "Remove_status: Null argument for int" );
    return NULL;
  }

  if( num_aff < 0 || num_aff > 22 ) {
    code_bug( "Remove_status: Affect out of range" );
    return NULL;
  }

  if( is_set( &ch->status, num_aff ) ) {
    remove_bit( &ch->status, num_aff );
  }
}

void* code_is_hidden( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  if( ch==NULL ) {
    code_bug( "Is_hidden: Null argument" );
    return NULL;
  }

  return (void*) is_set( &ch->status, STAT_HIDING );
 }

void* code_is_invis( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  if( ch==NULL ) {
    code_bug( "Is_invis: Null argument" );
    return NULL;
  }
  return (void*) is_set( ch->affected_by, AFF_INVISIBLE );
 }

void* code_is_camouflaged( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  if( ch==NULL ) {
    code_bug( "Is_camouflaged: Null argument" );
    return NULL;
  }
  return (void*) is_set( &ch->status, STAT_CAMOUFLAGED );
 }

void* code_sees_invis( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  int	can_see_it;

  if( ch->Sees_Invis( ) )
    return (void*) (can_see_it = 1);
  else
    return (void*) (can_see_it = 0);
}

void* code_sees_hidden( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  int   can_see_it;

  if( ch->Sees_Hidden( ) )
    return (void*) (can_see_it = 1);
  else
    return (void*) (can_see_it = 0);
}

void* code_can_see( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  char_data* victim = (char_data*) argument[1];
  int	can_see_it;

  if( ch==NULL ) {
    code_bug( "Can_see: Null argument" );
    return NULL;
  }

  if( ch->position <= POS_SLEEPING || is_set( ch->affected_by, AFF_BLIND ) )
    return (void*) (can_see_it = 0);

  if( victim == NULL ) {

    if( ch->pcdata->trust > 1 ) {
      return (void*) (can_see_it = 1); }
    else {
      if( ch->in_room->is_dark( ) )
        return (void*) (can_see_it = 0);
    }

    return (void*) (can_see_it = 1);
  }

  can_see_it = 1;

  if( is_set( &victim->status, STAT_HIDING ) ||
      is_set(&victim->status, STAT_CAMOUFLAGED )) {
    if( ch->Sees_Hidden( ) ) {
      can_see_it = 1;
    }
    else {
      can_see_it = 0;
    }
  }
  if( is_set( victim->affected_by, AFF_INVISIBLE ) ) {
    if( ch->Sees_Invis( ) ) {
      can_see_it = 1;
    }
    else {
      can_see_it = 0;
    }
  }

  return (void*) can_see_it;
 }

void* code_get_undead( void** argument )
{
  char_data* ch = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Get_undead: Null argument" );
    return NULL;
  }

  if( ch->species->zombie != 0 )
    return (void*) ch->species->zombie;

  return NULL;
}

void* code_get_skeleton( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  if( ch == NULL ) {
    code_bug( "Get_skeleton: Null argument" );
    return NULL;
  }
  if( ch->species->skeleton != 0 )
    return (void*) ch->species->skeleton;

  return NULL;
}

void* code_get_pos( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  if( ch == NULL ) {
    code_bug( "Get_pos: Null argument" );
    return NULL;
  }

  return (void *) ch->position;
}

void* code_set_pos( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  int	    pos = (int)	       argument[1];

    if( ch == NULL ) {
      code_bug( "Set_pos: Null argument" );
      return NULL;
    }

    if( pos < 0 ) {
      code_bug( "Set_pos: Attempted to set beyond dead" );
      return NULL;
    }

    ch->position = pos;
    return NULL;
}

void* code_size( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Size: Null argument" );
    return NULL;
    }

  return (void*) ch->Size( );
}


void* code_weight( void** argument )
{
  thing_data*  thing  = (char_data*) argument[0];

  if( thing == NULL ) {
    code_bug( "Weight: Null argument" );
    return NULL;
    }

  return (void*) thing->Weight( );
}


void* code_race( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Race: Null character." );
    return NULL;
    }

  return (void*) ch->shdata->race;
}


void* code_class( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Class: Null character." );
    return NULL;
    }

  if( ch->species != NULL ) {
    code_bug( "Class: Non-Player character." );
    return NULL;
    }
   
  return (void*) ch->pcdata->clss;
}


/*
 *   CFLAG ROUTINES
 */


void* code_cflag( void** argument )
{
  int         flag  = (int)        argument[0];
  char_data*    ch  = (char_data*) argument[1]; 

  if( ch == NULL ) {
    code_bug( "Code_Cflag: NULL character." );
    return (void*) FALSE;
    }

  if( flag < 0 || flag >= 32*MAX_CFLAG ) {
    code_bug( "Code_Cflag: flag out of range." );
    return (void*) FALSE;
    }

  if( ch->pcdata == NULL )
    return (void*) FALSE;

  return (void*) is_set( ch->pcdata->cflags, flag );
}


void* code_set_cflag( void** argument )
{
  int        flag  = (int)        argument[0];
  char_data*   ch  = (char_data*) argument[1]; 

  if( ch == NULL ) {
    code_bug( "Code_Set_Cflag: NULL character." );
    return NULL;
    }

  if( flag < 0 || flag >= 32*MAX_CFLAG ) {
    code_bug( "Code_Set_Cflag: flag out of range." );
    return NULL;
    }

  if( ch->pcdata != NULL )
    set_bit( ch->pcdata->cflags, flag );

  return NULL;
}


void* code_remove_cflag( void** argument )
{
  int flag      = (int)        argument[0];
  char_data* ch = (char_data*) argument[1]; 

  if( ch == NULL ) {
    code_bug( "Code_Remove_Cflag: NULL character." );
    return NULL;
    }

  if( flag < 0 || flag >= 32*MAX_CFLAG ) {
    code_bug( "Code_Remove_Cflag: flag out of range." );
    return NULL;
    }

  if( ch->pcdata != NULL )
    remove_bit( ch->pcdata->cflags, flag );

  return NULL;
}


/*
 *   CHARACTER STATUS ROUTINES
 */


void* code_can_fly( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Can_Fly: NULL Character." );
    return (void*) FALSE;
    }

  return (void*) ch->can_fly( );
}


void* code_is_silenced( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Is_Silenced: NULL Character." );
    return (void*) FALSE;
    }

  return (void*) is_set( ch->affected_by, AFF_SILENCE );
}


/*
 *   UNCLASSIFIED 
 */

void* code_remark( void** argument )
{
  return NULL;
}

void* code_interpret( void** argument )
{
  char_data* ch     = (char_data*) argument[0];
  char* string      = (char*)      argument[1];
  char_data* victim = (char_data*) argument[2];

  char tmp [ MAX_INPUT_LENGTH ];

  if( ch != NULL ) {
    if( victim == NULL ) 
      interpret( ch, string );
    else {
      sprintf( tmp, "%s %s", string, victim->descr->name );
      interpret( ch, tmp );
      }
    }

  return NULL;
}   


void* code_send_to_char( void** argument )
{
  char      *string = (char*)      argument[0];
  char_data *ch     = (char_data*) argument[1];

  send( string, ch );
 
  return NULL;
}


void* code_send_to_room( void** argument )
{
  char*      string = (char*)      argument[0];
  room_data*   room   = (room_data*) argument[1];

  if( room != NULL )
    act_room( room, string );
 
  return NULL;
}


void* code_send_to_area( void** argument )
{
  char      *string = (char*)      argument[0];
  room_data *room   = (room_data*) argument[1];

  if( room != NULL )
    send_to_area( string, room->area ); 
 
  return NULL;
}


void* code_act_room( void** argument )
{
  char*       string  = (char*)      argument[0];
  char_data*      ch  = (char_data*) argument[1];
  obj_data*      obj  = (obj_data*)  argument[2];
  char_data*  victim  = (char_data*) argument[3];  

  if( ch != NULL && ch->in_room != NULL )
    //act_room( ch->in_room, string );
    act_room( string, ch, victim, obj, ch->in_room );
  return NULL;
}


void* code_act_neither( void** argument )
{
  char*       string  = (char*)      argument[0];
  char_data*      ch  = (char_data*) argument[1];
  obj_data*      obj  = (obj_data*)  argument[2];
  char_data*  victim  = (char_data*) argument[3];  
  
  act_neither( string, ch, victim, obj );

  return NULL;
}


void* code_act_tochar( void** argument )
{
  char*      string  = (char*)      argument[0];
  char_data*     ch  = (char_data*) argument[1];
  obj_data*     obj  = (obj_data*)  argument[2];
  char_data* victim  = (char_data*) argument[3];  
  
  act( ch, string, ch, victim, obj );

  return NULL;
}


void* code_act_area( void** argument )
{
  char*       string  = (char*)      argument[0];
  char_data*      ch  = (char_data*) argument[1];
  obj_data*      obj  = (obj_data*)  argument[2];
  char_data*  victim  = (char_data*) argument[3];  
  
  if( ch == NULL ) {
    code_bug( "Act_Area: Character = null pointer." );
    return NULL;
    }

  act_area( string, ch, victim, obj );

  return NULL;
}


void* code_act_notchar( void** argument )
{
  char*      string  = (char*)      argument[0];
  char_data*     ch  = (char_data*) argument[1];
  obj_data*     obj  = (obj_data*)  argument[2];
  char_data* victim  = (char_data*) argument[3];  
  
  if( ch != NULL )
    act_notchar( string, ch, victim, obj );

  return NULL;
}


void* code_act_notvict( void** argument )
{
  char      *string = (char*)      argument[0];
  char_data *ch     = (char_data*) argument[1];
  obj_data  *obj    = (obj_data*)  argument[2];
  char_data *victim = (char_data*) argument[3];  
  
  act_notchar( string, ch, victim, obj );
  return NULL;
}


void* code_junk_mob( void** argument )
{
  char_data* ch = (char_data*) argument[0];

  if( ch == NULL )
    return NULL;
 
  if( !IS_NPC( ch ) ) {
    code_bug( "Junk_mob: character is a player??" );
    return NULL;
    } 
  
  ch->Extract( );

  return NULL;
}


void* code_drain_stat( void** argument )
{
  char_data* ch  = (char_data*) argument[0];
  int         i  = (int)        argument[1]; 
  int         j  = (int)        argument[2];
  int         m  = (int)        argument[3];

  int loc[] = { APPLY_STR, APPLY_INT, APPLY_WIS, APPLY_DEX, APPLY_CON }; 
  affect_data affect;
 
  if( ch == NULL || i < 0 || i > 4 || j <= 0 || m < -1 || m > 1 )
    return NULL;
 
  affect.type      = AFF_NONE;
  affect.location  = loc[i];
  affect.modifier  = m;
  affect.duration  = j;
  affect.level     = 5;
  affect.leech     = NULL;

  add_affect( ch, &affect );
  return NULL;
}  


void* code_find_stat( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];
  int          i  = (int)        argument[1]; 

  if( ch == NULL ) 
    return NULL;

  if( i < 0 || i > 17 ) {
    code_bug( "Find_Stat: Impossible field." );
    return NULL;
    } 

  if( ch->species != NULL ) {
    int value[] = { ch->Strength( ), ch->Intelligence( ),
      ch->Wisdom( ), ch->Dexterity( ), ch->Constitution( ),
      ch->shdata->level, 0, 0, ch->shdata->alignment,
      ch->max_hit, ch->hit, ch->max_mana, ch->mana,
      ch->max_move, ch->move,  ch->sex, 0, 0 };
    return (void*) value[i];
    }
  else { 
    int value[] = { ch->Strength( ), ch->Intelligence( ),
      ch->Wisdom( ), ch->Dexterity( ), ch->Constitution( ),
      ch->shdata->level, ch->pcdata->piety,
      ch->pcdata->clss, ch->shdata->alignment,
      ch->max_hit, ch->hit, ch->max_mana, ch->mana,
      ch->max_move, ch->move, ch->sex, ch->pcdata->condition[COND_FULL],
      ch->pcdata->condition[COND_THIRST] };
    return (void*) value[i];
    }
}


void* code_random( void** argument )
{
  int  n1  = (int) argument[0];
  int  n2  = (int) argument[1];

  return (void*) ( number_range( n1,n2 ) );
}


void* code_dice( void** argument )
{
  int  n1  = (int) argument[0];
  int  n2  = (int) argument[1];
  int  n3  = (int) argument[2];

  return (void*) ( roll_dice( n1,n2 )+n3 );
}


void* code_find_skill( void** argument )
{
  char_data*    ch  = (char_data*) argument[0];
  int        skill  = (int)        argument[1];

  if( ch == NULL ) {
    code_bug( "Find_Skill: NULL character." );
    return NULL;
    }

  return (void*) ch->get_skill( skill );
}


void* code_summon( void** argument )
{
  char_data*      ch  = (char_data*) argument[0];
  char_data*  victim  = (char_data*) argument[1];

  if( ch != NULL )
    summon_help( ch, victim );

  return NULL;
}


void* code_reputation( void** argument )
{
  char_data*          ch  = (char_data*) argument[0];
  int             nation  = (int)        argument[1];
  player_data*        pc  = (player_data*) ch;
  int          grievances = 0;
  
  if( ch == NULL ) {
    code_bug( "Reputation: Null character." );
    return NULL;
    }
  
  grievances = pc->reputation.nation[nation];
 
  return (void*) grievances;
}

void* code_modify_rep( void** argument )
{
  char_data*          ch  = (char_data*) argument[0];
  int             nation  = (int)        argument[1];
  int                mod  = (int)        argument[2];
  player_data*        pc  = (player_data*) ch;
  int          grievances = 0;
  
  if( ch == NULL ) {
    code_bug( "Reputation: Null character." );
    return NULL;
    }
  
  pc->reputation.nation[nation] += mod;
 
  return NULL;
}

void* code_find_room( void** argument )
{
  int         vnum   = (int) argument[0];
  room_data*  room;

  if( ( room = get_room_index( vnum, FALSE ) ) == NULL ) 
    code_bug( "Find_Room: Non-existent room." );
 
  return room;
}

/* Sadis -- rewrote code_cast_spell.. commented out old one
void* code_cast_spell( void** argument )
{
  int          spell  = (int)        argument[0];
  char_data*      ch  = (char_data*) argument[1];
  char_data*  victim  = (char_data*) argument[2];

  if( ch == NULL || spell < SPELL_FIRST || spell >= WEAPON_UNARMED ) 
    return NULL;

  switch( spell_table[ spell-SPELL_FIRST ].type ) {
   case STYPE_SELF_ONLY :
   case STYPE_PEACEFUL :
    if( victim == NULL )
      victim = ch;
    break;

   case STYPE_OFFENSIVE :
    if( victim == NULL )
      return NULL;
    break;

   default :
    return NULL;
    }
*/
  /*
  stop_active( ch );

  event = new cast_event( ch, victim );
  
  event->vo      = victim;
  event->spell   = *spell-SPELL_FIRST;
  event->prepare = FALSE;
  event->wait    = spell_table[*spell-SPELL_FIRST].prepare-1;
  ch->active     = event;

  has_reagents( ch, event );
  execute_cast( event );
  */
/*
  return NULL;  
}
*/
// Sadis -- new code_cast_spell()
void* code_cast_spell( void** argument )
{
  int          spell  = (int)        argument[0];
  char_data*      ch  = (char_data*) argument[1];
  char_data*  victim  = (char_data*) argument[2];
  cast_data*    cast;

  if( ch == NULL || spell < SPELL_FIRST || spell >= WEAPON_UNARMED ) {
    code_bug( "Cast_spell: Invalid spell" );
    return NULL;
    }

  if( ch == NULL ) {
    code_bug( "Cast_spell: Invalid caster" );
    return NULL;
    }

  if( victim == NULL ) {
    code_bug( "Cast_spell: Invalid target" );
    return NULL;
    }

  if( ch->in_room != victim->in_room ) {
    code_bug( "Cast_spell: caster and target not in same room" );
    return NULL;
    }

  switch( spell_table[ spell-SPELL_FIRST ].type ) {
   case STYPE_SELF_ONLY :
   case STYPE_PEACEFUL :
    if( victim == NULL )
      victim = ch;
    break;

   case STYPE_OFFENSIVE :
    if( victim == NULL )
      return NULL;
    break;

   default :
    return NULL;
    }

  spell = spell- SPELL_FIRST;

  if( is_confused_pet( ch ) || is_familiar( ch ) )
    return NULL;

  if( is_set( ch->affected_by, AFF_SILENCE ) ) {
    send( ch, "You are silenced and unable to cast spells!\n\r" );
    return NULL;
    }

  if( is_set( ch->affected_by, AFF_ENTANGLED ) ) {
    send( ch, "You are too entangled to cast spell!\n\r" );
    return NULL;
    }

  cast           = new cast_data;
  cast->spell    = spell;
  cast->prepare  = FALSE;
  cast->wait     = spell_table[spell].prepare-1;
  cast->mana     = 0;
  cast->target   = victim;

  if( !has_reagents( ch, cast ) ) {
    delete cast;
    return NULL;
    }

  send( ch, "You begin casting %s.\n\r", spell_table[spell].name );

  ch->cast    = cast;
  add_queue( &ch->active, ch->species != NULL ? 6
    : 10-ch->shdata->skill[ SPELL_FIRST+spell ]/2 );

  return NULL;
}
// Sadis -- done code_cast_spell()

void* code_do_spell( void** argument )
{
  int          spell  = (int)        argument[0];
  char_data*      ch  = (char_data*) argument[1];
  char_data*  victim  = (char_data*) argument[2];

  if( ch == NULL )
    return NULL;

  if( victim == NULL )
    victim = ch;

  if( spell >= SPELL_FIRST && spell < WEAPON_UNARMED ) 
    ( spell_table[spell-SPELL_FIRST].function )( ch, victim,
      NULL, 10, -1 );

  return NULL;
}


void* code_num_in_room( void** argument )
{
  room_data*  room  = (room_data*) argument[0];
  int          num  = 0;

  if( room != NULL )
    for( int i = 0; i < room->contents; i++ )
      num += ( character( room->contents[i] ) != NULL );

  return (void*) num;
}  


void* code_players_area( void** argument )
{
  room_data*  room  = (room_data*) argument[0];

  if( room == NULL ) {
    code_bug( "Players_Area: NULL room." );
    return NULL;
    }

  return (void*) room->area->nplayer;
}  


void* code_players_room( void** argument )
{
  room_data*  room  = (room_data*) argument[0];
  int          num  = 0;  

  if( room != NULL )
    for( int i = 0; i < room->contents; i++ )
      num += ( player( room->contents[i] ) != NULL );

  return (void*) num;
}  


void* code_num_mob( void** argument )
{
  int         vnum  = (int)        argument[0];
  room_data*  room  = (room_data*) argument[1];
  mob_data*    rch;
  int          num  = 0;  

  if( room != NULL )
    for( int i = 0; i < room->contents; i++ )
      if( ( rch = mob( room->contents[i] ) ) != NULL
        && rch->species->vnum == vnum )
        num++;

  return (void*) num;
}  

void* code_use_move( void** argument )
{
  char_data*    ch  = (char_data*) argument[0];
  int          num  = (char_data*) argument[1];

  if( ch == NULL ) {
    code_bug( "Code_use_move: NULL ch" );
    return (void*) FALSE;
    }

  if( num <= 0 ) {
    code_bug( "Code_use_move: Non-positive movement" );
    return (void*) FALSE;
    }

  if( ch->move >= num ) {
    ch->move -= num;
    return (void*) TRUE;
    }

  return (void*) FALSE;
}

void* code_transfer( void** argument )
{
  char_data*    ch  = (char_data*) argument[0];
  room_data*  room  = (room_data*) argument[1];

  if( room != NULL && room->vnum == 4 ) 
    room = get_room_index( ROOM_PRISON );

  if( room == NULL ) {
    send( "A script attempts to send you to a non-existent room??\n\r", ch );
    return NULL;
    }

  if( ch == NULL )
    return NULL;

  if( ch->rider != NULL )
    ch = ch->rider;

  ch->From( );
  ch->To( room );

  if( ch->mount != NULL ) {
    ch->mount->From( );
    ch->mount->To( room );
    }

  send( "\n\r", ch );
  do_look( ch, "" );

  return NULL;
}


void* code_transfer_all( void** argument )
{
  room_data*       from  = (room_data*) argument[0];
  room_data*         to  = (room_data*) argument[1];
  char_data*        rch;
  thing_array      list;

  if( to == NULL )
    return NULL;

  copy( list, from->contents );

  for( int i = 0; i < list; i++ )
    if( ( rch = character( list[i] ) ) != NULL ) {
      rch->From( );
      rch->To( to );
      }

  for( int i = 0; i < list; i++ )
    if( ( rch = character( list[i] ) ) != NULL ) {
      send( "\n\r", rch );
      show_room( rch, to, FALSE, FALSE );
      }

  return NULL;
}

void* code_show_room( void** argument)
{
   char_data*     ch = (char_data*) argument[0];	
   room_data*	  to = (room_data*) argument[1];
   
   if( to != NULL ) 
      show_room( ch, to, FALSE, FALSE );
   
   return NULL;
}
     	
void* code_mload( void** argument )
{
  int               vnum  = (int)        argument[0];
  room_data*        room  = (room_data*) argument[1];
  mob_data*          mob;
  species_data*  species;

  if( room == NULL || ( species = get_species( vnum ) ) == NULL ) {
    code_bug( "Mload: non-existent species or null room." );
    return NULL;
    }

  mob = create_mobile( species );
  mreset_mob( mob );
 
  mob->To( room );
  
  return mob;
}


void* code_rand_char( void** argument )
{
  room_data*  room  = (room_data*) argument[0];
  char_data*   rch;

  if( room == NULL )
    return NULL;

  rch = random_pers( room );
  
  return rch;
}


void* code_rand_player( void** argument )
{
  room_data*  room  = (room_data*) argument[0];
  char_data*   rch;

  if( room == NULL )
    return NULL;

  rch = rand_player( room );
  
  return rch;
}

   
void* code_obj_in_room( void** argument )
{
  int         vnum  = (int)       argument[0];
  room_data*  room  = (room_data*) argument[1];

  if( room == NULL ) {
    code_bug( "Obj_in_room: NULL room." );
    return NULL;
    }

  return find_vnum( room->contents, vnum );
}


void* code_mob_in_room( void** argument )
{
  int         vnum  = (int)        argument[0];
  room_data*  room  = (room_data*) argument[1];
  char_data*   rch;

  if( room == NULL ) {
    code_bug( "Mob_in_Room: Null Room??" );
    return NULL;
    }

  for( int i = 0; i < room->contents; i++ )
    if( ( rch = mob( room->contents[i] ) ) != NULL
      && rch->species->vnum == vnum )
      return rch; 
  
  return NULL;
}
// Sadis -- changed this function for that keyrings work with
//          has_obj()
void* code_has_obj( void** argument )
{
  int        vnum  = (int)        argument[0];
  char_data*   ch  = (char_data*) argument[1];
  obj_data*   obj;

  if( ch == NULL ) {
    code_bug( "Has_Obj: NULL character." );
    return NULL;
    }

  if( find_vnum( ch->contents, vnum ) ) return (void*) find_vnum( ch->contents, vnum );
  if( find_vnum( ch->wearing, vnum ) ) return (void*) find_vnum( ch->wearing, vnum );

  for( int a = 0 ; a < ch->contents ; a++ )
    if( ( obj = object( ch->contents[a] ) ) != NULL )
      if( obj->pIndexData->vnum == 56 )
        if( find_vnum( obj->contents, vnum ) ) return( void* ) find_vnum( obj->contents, vnum );

  for( int b = 0 ; b < ch->wearing ; b++ )
    if( ( obj = object( ch->wearing[b] ) ) != NULL )
      if( obj->pIndexData->vnum == 56 )
        if( find_vnum( obj->contents, vnum ) ) return( void* ) find_vnum( obj->contents, vnum );

  return NULL;
}

void* code_has_obj_inv( void** argument )
{
  int        vnum  = (int)        argument[0];
  char_data*   ch  = (char_data*) argument[1];
  bool     cont, wearing;

  if( ch == NULL ) {
    code_bug( "Has_Obj_Inv: NULL character." );
    return NULL;
    }

  return find_vnum( ch->contents, vnum );
}

void* code_has_obj_worn( void** argument )
{
  int        vnum  = (int)        argument[0];
  char_data*   ch  = (char_data*) argument[1];
  bool     cont, wearing;

  if( ch == NULL ) {
    code_bug( "Has_Obj_Worn: NULL character." );
    return NULL;
    }

  return find_vnum( ch->wearing, vnum );
}

void* code_obj_to_room( void** argument )
{
  obj_data*    obj  = (obj_data*)  argument[0];
  room_data*  room  = (room_data*) argument[1];

  if( obj == NULL || room == NULL )
    return NULL;

  obj->To( room );
  
  return NULL;
}


void* code_obj_to_char( void** argument )
{
  obj_data*   obj  = (obj_data*)  argument[0];
  char_data*   ch  = (char_data*) argument[1];

  if( obj == NULL || ch == NULL )
    return NULL;

// this should make objs with custom_fit set usable by
// the person who gets the obj

  obj->size_flags = ( ch->pcdata == NULL ? -1
    : ch->pcdata->pfile->ident );

  obj->To( ch );

  return obj;
}

   
void* code_coin_value( void** argument )
{
  obj_data*  obj  = (obj_data*) argument[0];
  int          i;

  if( obj == NULL || obj->pIndexData->item_type != ITEM_MONEY )  
    return NULL;

  for( i = 0; i < MAX_COIN; i++ ) 
    if( obj->pIndexData->vnum == coin_vnum[i] ) 
      return (void*) ( coin_value[i]*obj->number );

  return NULL;;
}


void* code_plague( void** argument )
{
  char_data*        ch  = (char_data*) argument[0];
  affect_data   affect;
 
  if( ch == NULL || ch->save_vs_poison( 5 ) )
    return NULL;

  if( var_mob != NULL && var_mob->species != NULL
    && !is_set( ch->affected_by, AFF_PLAGUE ) )
    var_mob->species->special += 20;

  affect.type      = AFF_PLAGUE;
  affect.duration  = 11;
  affect.level     = 0;
  affect.leech     = NULL;

  add_affect( ch, &affect );

  return NULL;
}


void* code_poison( void** argument )
{
  char_data*        ch  = (char_data*) argument[0];
  affect_data   affect;

  if( ch == NULL || ch->save_vs_poison( 5 ) )
    return NULL;

  if( var_mob != NULL && var_mob->species != NULL 
    && !is_set( ch->affected_by, AFF_POISON ) ) 
    var_mob->species->special += 10;

  affect.type      = AFF_POISON;
  affect.duration  = 10;
  affect.level     = 8;
  affect.leech     = NULL;

  add_affect( ch, &affect );

  return NULL;
}


void* code_heal( void** argument )
{
  char_data* ch = (char_data*) argument[0];
  int i         = (int)        argument[1];

  if( ch != NULL ) {
    ch->hit += i;
    ch->hit = min( ch->hit, ch->max_hit );
    }

  return NULL;
}


void* code_modify_mana( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];
  int          i  = (int)        argument[1];

  if( ch == NULL || ch->mana+i < 0 ) 
    return (void*) FALSE;

  ch->mana += i;
  ch->mana = min( ch->mana, ch->max_mana );

  return (void*) TRUE;
}


void* code_drain_exp( void** argument )
{
  char_data *ch = (char_data*) argument[0];
  int i         = (int)       argument[1];

  if( ch != NULL && ch->species == NULL ) 
    add_exp( ch, -i, "You lose %d exp!!\n\r" );

  return NULL;
}


/*
 *   FUNCTION CALLS
 */


void* code_acode( void** argument )
{
  room_data *room = (room_data*) argument[0];
  int i           = (int)        argument[1];

  action_data* action; 
  int               j  = 1;

  if( room == NULL ) {
    code_bug( "Code_acode: NULL room." );
    return NULL;
    }

  for( action = room->action; action != NULL; action = action->next ) 
    if( j++ == i ) {
      push( );
      execute( action );
      pop( );
      break;
      }

  if( action == NULL ) 
    code_bug( "Code_acode: NULL action." );

  return NULL;
}


void* code_mpcode( void** argument )
{
  char_data* mob = (char_data*) argument[0];
  int i          = (int)       argument[1];

  mprog_data* mprog;
  int             j  = 1;

  if( mob == NULL || mob->species == NULL ) {
    code_bug( "Code_mpcode: NULL mob or mob is a player." );
    return NULL;
    }

  for( mprog = mob->species->mprog; mprog != NULL; mprog = mprog->next ) 
    if( j++ == i ) {
      push( );
      execute( mprog );
      pop( );
      break;
      }

  if( mprog == NULL ) 
    code_bug( "Code_mpcode: NULL mprog." );

  return NULL;
}


void* code_opcode( void** argument )
{
  obj_data *obj = (obj_data*) argument[0];
  int i         = (int)       argument[1];

  oprog_data* oprog;
  int             j  = 1;

  if( obj == NULL ) {
    code_bug( "Opcode: NULL obj." );
    return NULL;
    }
 
  for( oprog = obj->pIndexData->oprog; oprog != NULL; oprog = oprog->next ) 
    if( j++ == i ) {
      push( );
      execute( oprog );
      pop( );
      break;
      }

  if( oprog == NULL )
    code_bug( "Opcode: NULL oprog." );

  return NULL;
}


void* code_wait( void** argument )
{
  int i = (int) argument[0];

  queue_data* queue = new queue_data;

  end_prog   = TRUE;
  queue_prog = TRUE;
  
  queue->time = i;

  queue->room = var_room;
  queue->mob  = var_mob;
  queue->ch   = var_ch;
  queue->obj  = var_obj;
  queue->i    = var_i;

  queue->arg  = curr_arg;
  queue->next = queue_list;
  queue_list  = queue;

  return NULL;
}


void* code_is_searching( void** argument )
{
  char_data*   ch  = (char_data*) argument[0];

  return (void*) ( ch != NULL && ch->pcdata != NULL
    && is_set( ch->pcdata->pfile->flags, PLR_SEARCHING ) );
}


void* code_is_follower( void** argument )
{
  char_data*   ch  = (char_data*) argument[0];

  if( ch == NULL ) {
    code_bug( "Is_Follower: NULL character." );
    return FALSE;
    }
  if( ch->leader != NULL )
    return (void*) TRUE;

  return (void*) FALSE;
}

void* code_is_following( void** argument )
{
  char_data*    ch = (char_data*) argument[0];
  char_data* victim = (char_data*) argument[1];

  for( int i=0; i < victim->followers; i++ )
    if( ch == victim->followers[i] )
      return (void*) TRUE;

  return (void*) FALSE;

}

void* code_char_in_room( void** argument )
{
  char_data*    ch  = (char_data*) argument[0];
  room_data*  room  = (room_data*) argument[1]; 

  return (void*) ( ch != NULL && ch->in_room == room );
}


void* code_is_name( void** argument )
{
  char*       str  = (char*) argument[0];
  char*  namelist  = (char*) argument[1];

  if( str == NULL || namelist == NULL )
    return (void*) FALSE;

  return (void*) is_name( str, namelist );
}

void* code_set_align( void** argument )
{
  char_data*  ch = (char_data*) argument[0];
  int          i = (int)        argument[1];

  if( ch == NULL || ch->pcdata == NULL )
    return NULL;

  if( i < 0 || i >= MAX_ALIGNMENT ) {
    code_bug( "Code_set_align: alignment value out of range." );
    return NULL;
  }

  ch->shdata->alignment = i;

  return NULL;
}

void* code_set_religion( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];
  int          i  = (int)        argument[1];

  if( ch == NULL || ch->pcdata == NULL )
    return NULL;

  if( i < 0 || i >= MAX_RELIGION ) {
    code_bug( "Code_set_religion: religion value out of range." );
    return NULL;
    } 

  ch->pcdata->religion = i;

  return NULL;
}  


void* code_tell( void** argument )
{
  char_data*      ch  = (char_data*) argument[0];
  char_data*  victim  = (char_data*) argument[1];
  char*       string  = (char*)      argument[2];

  char      tmp  [ 3*MAX_STRING_LENGTH ];

  if( victim == NULL || ch == NULL || victim->pcdata == NULL )
    return NULL;

  act_print( tmp, string, victim, ch, NULL, NULL, NULL, NULL, victim );
  process_tell( ch, victim, tmp );

  return NULL;
}


void* code_obj_name( void** argument )
{
  obj_data* obj = (obj_data*) argument[0];

  if( obj == NULL )
    return NULL;

  return obj->singular;
} 


void* code_disarm( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];
  int          j  = (int)        argument[1];

  if( ch == NULL ) {
    code_bug( "Disarm: NULL character or level." );
    return NULL;
    }

  return (void*) ( ch->check_skill( SKILL_UNTRAP )
    && number_range( 0, 20 ) > j );
}


/*
 *   MOVEMENT ROUTINES
 */


void* code_modify_move( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];
  int          i  = (int)        argument[1];

  if( ch == NULL || ch->move+i < 0 ) 
    return (void*) FALSE;

  ch->move += i;
  ch->move  = min( ch->move, ch->max_move );
 
  return (void*) TRUE;
}


void* code_is_exhausted( void** argument )
{
  char_data*  ch  = (char_data*) argument[0];
  int          i  = (int)        argument[1];

  if( ch == NULL )
    return (void*) TRUE;
  
  if( ch->move-i < 0 ) {
    send( ch, "You are too exhausted.\n\r" );
    return (void*) TRUE;
    }

  ch->move -= i;
 
  return (void*) FALSE;
}

void* code_arena_watch( void** argument )
{
    char_data*    ch  = (char_data*) argument[0];
    room_data*  room  = (room_data*) argument[1];

    if( ch == NULL ) {
        code_bug( "Watch : NULL Character." );
        return (void*) FALSE;
    }
    do_watch( ch, room );

    return (void*) TRUE;
}

void* code_rem_not_aff( void** argument )
{
    char_data* ch = (char_data*) argument[0];

    if ( ch == NULL ) {
        code_bug( "aff_rem : NULL Character." );
        return (void*) FALSE;
    }
    //This is wierd, have to run it twice to remove element 0 of ch->affect
    rem_nonself_aff( ch );
    rem_nonself_aff( ch );

    return (void*) TRUE;

}
