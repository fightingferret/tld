#include <sys/types.h>
#include <stdio.h>
#include "define.h"
#include "struct.h"

thing_data* steal( thing_data* obj, char_data* receiver, thing_data* giver )
{
    obj = obj->From( obj->selected );
    set_owner( (obj_data*) obj, receiver, (char_data*) giver );
    obj->To( receiver );

    return obj;
}

thing_data* badpos( thing_data* object, char_data* thief, thing_data* victim )
{
  
    char_data*  rch = (char_data*) victim;
    obj_data*   obj = (obj_data*) object;

    if( obj == rch->Wearing( WEAR_BODY ) ||
        obj == rch->Wearing( WEAR_HEAD ) ||
        obj == rch->Wearing( WEAR_LEGS ) ||
        obj == rch->Wearing( WEAR_FEET ) ||
        obj == rch->Wearing( WEAR_ARMS ))
        return NULL; 
    return obj; 
}

void remove_poison( obj_data* reagent, char_data* ch )
{
    if( reagent->value[1] <= 1 ) {
        reagent->Extract( 1 );
        return;
    }

    if( reagent->number > 1 ) {
        reagent->Extract( 1 );
        reagent = duplicate( reagent, 1 );
        reagent->value[1]--;
        reagent->To( ch );
    }
    else {
        reagent->value[1]--;
    }
}

void do_coat( char_data* ch, char* argument )
{
    obj_data      *obj;
    obj_data      *reagent;
    int           level;
    affect_data   affect;
    char          arg[MAX_INPUT_LENGTH];

    if( !two_argument( argument, "with", arg ) ) {
        send( ch, "Syntax: Coat <object> [with] <object>\n\r" );
        return;
    }

    if( ( obj = one_object( ch, arg, "coat", &ch->contents ) ) == NULL )
        return;

    if( ( reagent = one_object( ch, argument, "coat with", &ch->contents ) ) == NULL )
        return;
  
    if( ( level = ch->get_skill( SKILL_POISON ) ) == 0 ) {
        send( ch, "You really shouldn't be playing with poison should you?\n\r");
        return;
    }
  
    if( obj->pIndexData->item_type != ITEM_WEAPON ) {
        send( ch, "It wouldn't do any good to coat %s.\n\r", obj );
        return;
    }

    if( reagent->pIndexData->item_type != ITEM_POISON ) {
        send( ch, "%s can not be used to coat your weapon.\n\r", reagent );
        return;
    }

    if( obj->affected.size != 0 )
        switch( obj->affected[0]->type ) {
            case AFF_BURNING:
            case AFF_FLAMING:
                send( ch, "Are you nuts?!  %s is on fire!\n\r", obj );
                return;
            default:
                send( ch, "%s is already coated with poison.\n\r", obj );
                return;
        }  
  
    remove_poison( reagent, ch );
  
    affect.type      = FIRST_POISON + reagent->value[0];
    affect.duration  = level*3;
    affect.level     = level;
    affect.leech     = NULL;

    add_affect( obj, &affect );

    ch->improve_skill( SKILL_POISON );
 
    return; 

}

/* 
 *   BACKSTAB FUNCTIONS
 */


void do_backstab( char_data* ch, char* argument )
{
    char_data*  victim;
    obj_data*      obj;
    int          skill  = ch->get_skill( SKILL_BACKSTAB );

    if( ch->mount != NULL ) {
        send( ch, "Backstabbing while mounted is beyond your skill.\n\r" );
        return;
    }

    if( *argument == '\0' ) {
        send( ch, "Backstab whom?\n\r" );
        return;
    }

    if( ( victim = one_character( ch, argument, "backstab", ch->array ) ) == NULL ) 
        return;

    if( opponent( ch ) != NULL ) {
        send( "You are already fighting someone.\n\r", ch );
        return;
    }

    if( victim == ch ) {
        send( "How can you sneak up on yourself?\n\r", ch );
        return;
    }

    if( skill == 0 ) {
        send( "Backstabbing is not part of your repertoire.\n\r", ch );
        return;
    }

    if( ( obj = ch->Wearing( WEAR_HELD_R ) ) == NULL ) {
        send( "You need to be wielding a weapon to backstab.\n\r", ch );
        return;
    }

    if( !is_set( obj->pIndexData->extra_flags, OFLAG_BACKSTAB ) ) {
        send( ch, "It isn't possible to use %s to backstab.\n\r", obj );
        return;
    }

    if( opponent( ch ) != NULL ) {
        send( ch, "You are unable to backstab while fighting someone.\n\r" );
        return;
    }

    if( ch->Seen( victim ) || includes( victim->aggressive, ch ) ) {
        send( ch, "%s is too wary of you for backstab to succeed.\n\r", victim );
        return;
    }

    if( !can_kill( ch, victim ) )
        return;

    check_killer( ch, victim );

    remove_bit( &ch->status, STAT_WIMPY );
    remove_bit( &ch->status, STAT_LEAPING );

    leave_shadows( ch );
    attack( ch, victim, "backstab", obj, -1, 0 );
    update_pos( victim );
    if( ( victim->position > POS_DEAD ) && ( victim->in_room == ch->in_room ) ) {
        ch->fighting = victim;
        victim->aggressive += ch;
        react_attack( ch, victim );
        add_queue( &ch->active, 20 );
    }
    return;
}


/*
 *   STEAL ROUTINES
 */


void do_steal( char_data* ch, char* argument )
{
    char           buf  [ MAX_INPUT_LENGTH ];
    char           arg  [ MAX_INPUT_LENGTH ];
    char_data*  victim;
    thing_array*   array;

    if( is_confused_pet( ch ) )
        return;
 
    if( is_mob( ch ) )
        return;
 
    if( is_set( ch->pcdata->pfile->flags, PLR_PARRY ) ) {
        send( ch, "You can not steal with parry on.\n\r" );
        return;
    }

    argument = one_argument( argument, arg );

    for( ; ; ) {
        argument = one_argument( argument, buf );
        if( buf[ 0 ] == '\0' || !strcasecmp( buf, "from" ) )
            break;
        sprintf( arg+strlen( arg ), " %s", buf );
    }

    if( arg[0] == '\0' || argument[0] == '\0' ) {
        send( "Syntax: steal <object> from <character>\n\r", ch );
        return;
    }

    if( ( victim = one_character( ch, argument, "steal", ch->array ) ) == NULL ) 
        return;

    if( victim == ch ) {
        send( "That's pointless.\n\r", ch );
        return;
    }

    if( victim->pcdata != NULL ) {
        send( "You can't steal from players.\n\r", ch );
        return;
    }  

    if( !can_kill( ch, victim ) ) {
        send(ch, "You can't steal from them.\n\r" );
        return; 
    } 

    if( ch->shdata->skill[ SKILL_STEAL] == UNLEARNT) {
        send(ch, "You are untrained in the art of stealing.\n\r");
        return;
    }
   
    remove_bit( ch->pcdata->pfile->flags, PLR_PARRY );

    if(ch->Dexterity() / 4 + ch->shdata->skill [ SKILL_STEAL ] + 
        ( ( ch->shdata->level-victim->shdata->level )/4) < number_range( 1, 20 ) ) {
        leave_shadows( ch );
        if(!player(victim) ) {
            ch->fighting = victim;
            react_attack( ch, victim );
            add_queue( &ch->active, 20 );
            remove_bit( &ch->status, STAT_LEAPING );
            remove_bit( &ch->status, STAT_WIMPY );
        } 
        send( victim, "%s tried to steal from you.\n\r", ch );
        send( *ch->array, "%s tried to steal from %s.\n\r", ch, victim );
        modify_reputation( ch, victim, REP_STOLE_FROM );
        if( victim->pShop != NULL ) {
            sprintf( buf, "Guards! %s is a thief.", ch->Name( victim ) );
            do_yell( victim, buf );
            summon_help( victim, ch );
        }
        return;
    }

    if( ( array = several_things( ch, arg, "steal", &victim->contents ) ) == NULL )
        return;

    if(array->size > 1) {
        send(ch, "You may only steal one item at a time.");
        return;
    } 
    thing_array   subset  [ 4 ];
    thing_func*     func  [ 4 ]  = { cursed, heavy, many, steal };

    sort_objects( ch, *array, victim, 4, subset, func );

    page_priv( ch, NULL, empty_string );
    page_priv( ch, &subset[0], "can't let go of", victim );
    page_priv( ch, &subset[1], "can't lift" );
    page_priv( ch, &subset[2], "can't handle" );
    page_priv( ch, &subset[3], "steal", victim, "from" );

    ch->improve_skill( SKILL_STEAL );

    return;
}

void do_heist( char_data* ch, char* argument )
{

    char           buf  [ MAX_INPUT_LENGTH ];
    char           arg  [ MAX_INPUT_LENGTH ];
    char_data*  victim;
    thing_array*   array;

    if( is_confused_pet( ch ) )
        return;
 
    if( is_mob( ch ) )
        return;
 
    if( is_set( ch->pcdata->pfile->flags, PLR_PARRY ) ) {
        send( ch, "You can not heist with parry on.\n\r" );
        return;
    }

    if( !is_set( &ch->status, STAT_HIDING ) && !is_set( &ch->status, STAT_CAMOUFLAGED ) ) {
        send( ch, "You can not heist while hidden.\n\r" );
        return;
    }

    argument = one_argument( argument, arg );

    for( ; ; ) {
        argument = one_argument( argument, buf );
        if( buf[ 0 ] == '\0' || !strcasecmp( buf, "from" ) )
            break;
        sprintf( arg+strlen( arg ), " %s", buf );
    }

    if( arg[0] == '\0' || argument[0] == '\0' ) {
        send( "Syntax: heist <object> from <character>\n\r", ch );
        return;
    }

    if( ( victim = one_character( ch, argument, "heist", ch->array ) ) == NULL ) 
        return;

    if( victim == ch ) {
        send( "That's pointless.\n\r", ch );
        return;
    }

    if( victim->pcdata != NULL ) {
        send( "You can't heist from players.\n\r", ch );
        return;
    }  

    if( !can_kill( ch, victim ) ) {
        send(ch, "You can't heist from them.\n\r" );
        return; 
    } 

    if( ch->shdata->skill[ SKILL_HEIST] == UNLEARNT) {
        send(ch, "You are untrained in the art of heisting.\n\r");
        return;
    }
   
    remove_bit( ch->pcdata->pfile->flags, PLR_PARRY );

    if(ch->Dexterity() / 4 + ch->shdata->skill [ SKILL_HEIST ] + 
        ( ( ch->shdata->level-victim->shdata->level )/4) < number_range( 1, 20 ) ) {
        leave_shadows( ch );
        if(!player(victim)) {
            ch->fighting = victim;
            react_attack( ch, victim );
            add_queue( &ch->active, 20 );
            remove_bit( &ch->status, STAT_LEAPING );
            remove_bit( &ch->status, STAT_WIMPY );
        } 
        send( victim, "%s tried to steal from you.\n\r", ch );
        send( *ch->array, "%s tried to steal from %s.\n\r", ch, victim );
        modify_reputation( ch, victim, REP_STOLE_FROM );
        if( victim->pShop != NULL ) {
            sprintf( buf, "Guards! %s is a thief.", ch->Name( victim ) );
            do_yell( victim, buf );
            summon_help( victim, ch );
        }
        return;
    }

    if( ( array = several_things( ch, arg, "heist", &victim->wearing ) ) == NULL )
        return;

    if(array->size > 1) {
        send(ch, "You may only heist one item at a time.");
        return;
    } 
    thing_array   subset  [ 5 ];
    thing_func*     func  [ 5 ]  = { cursed, heavy, many, badpos, steal };

    sort_objects( ch, *array, victim, 5, subset, func );

    page_priv( ch, NULL, empty_string );
    page_priv( ch, &subset[0], "can't let go of", victim );
    page_priv( ch, &subset[1], "can't lift" );
    page_priv( ch, &subset[2], "can't handle" );
    page_priv( ch, &subset[3], "can't heist");
    page_priv( ch, &subset[4], "steal", victim, "from" );

    ch->improve_skill( SKILL_STEAL );

    return;
}

void do_sneak( char_data* ch, char* argument )
{
    if( not_player( ch ) )
        return;

    if( ch->shdata->skill[ SKILL_SNEAK ] == 0 ) {
        send( ch, "Sneaking is not something you are adept at.\n\r" );
        return;
    }

    if( is_set( ch->pcdata->pfile->flags, PLR_SNEAK ) ) {
        remove_bit( ch->pcdata->pfile->flags, PLR_SNEAK );
        send( ch, "You stop sneaking.\n\r" );
        return;
    }
   
    if( ch->mount != NULL ) {
        send( ch, "But your mount can't sneak!\n\r");
        return;
    }

    if( ch->fighting != NULL ) {
        send( ch, "It would be a good idea to concentrate on the battle.\n\r" );
        return;
    }

    if( is_set( ch->affected_by, AFF_FIRE_SHIELD ) || is_set( ch->affected_by, AFF_FAERIE_FIRE ) ) {
        send( ch, "Your fiery glow rather spoils that.\n\r" );
        return;
    }

    if( water_logged( ch->in_room ) ) {
        send( ch, "It's hard to sneak in water.\n\r" );
        return;
    }
    
    set_bit( ch->pcdata->pfile->flags, PLR_SNEAK );

    send( ch, "You start sneaking.\n\r" );
    
}

void do_hide( char_data* ch, char* )
{
    char_data*               rch;
    char*            send_string;
    int           terrain_chance;

    if( not_player( ch ) )
        return;

    if( ch->shdata->skill[ SKILL_HIDE ] == 0 ) {
        send( ch, "You attempt to conceal yourself but fail.\n\r" );
        return;
    }

    if( leave_shadows( ch ) )
        return;

    if( ch->mount != NULL ) {
        send( ch, "You cannot hide while mounted.\n\r" );
        return;
    }
    
    if( ch->fighting != NULL ) {
        send( ch, "You are in battle!\n\r");
        return;
    }

    if( is_set( ch->affected_by, AFF_FIRE_SHIELD ) || is_set( ch->affected_by, AFF_FAERIE_FIRE ) ) {
        send( ch, "Your fiery glow rather spoils that.\n\r" );
        return;
    }

    switch( ch->in_room->sector_type ) {
        case SECT_SWAMP :
            terrain_chance = 10;
            send_string = "You crouch low, concealing yourself in the swampy fog.";
            break;
        case SECT_FOREST :
            terrain_chance = 10;
            send_string = "You conceal yourself in the trees.";
            break;
        case SECT_JUNGLE :
            terrain_chance = 10;
            send_string = "You hide in the lush foliage.";
            break;
        case SECT_CAVES :
            terrain_chance = 9;
            send_string = "You step into the shadows, concealing yourself.";
            break;
        case SECT_ARCTIC :
            terrain_chance = 9;
            send_string = "Ignoring the cold, you crouch low in the snow and conceal yourself.";
            break;
        case SECT_FIELD :
            terrain_chance = 8;
            send_string = "You hunker down in the grass.";
            break;
        case SECT_CITY :
            terrain_chance = 8;
            send_string = "You hide among the shadows of the city.";
            break;
        case SECT_HILLS :
            terrain_chance = 7;
            send_string = "You hunker down in the grass.";
            break;
        case SECT_MOUNTAIN :
            terrain_chance = 7;
            send_string = "You hide among the rocks and trees.";
            break;
        case SECT_ICE :
            terrain_chance = 4;
            send_string = "You crouch low, hugging the icy terrain.";
            break;
        case SECT_DESERT :
            terrain_chance = 3;
            send_string = "You manage to conceal yourself in the sandy terrain.";
            break;
        case SECT_INSIDE :
            terrain_chance = 1;
            send_string = "You hide in the shadows.";
            break;
        case SECT_ROAD :
            terrain_chance = 1;
            send_string = "You crouch down on the side of the road.";
            break;
        case SECT_SHALLOWS :
            terrain_chance = 1;
            send_string = "You conceal yourself in the marsh.";
            break;
        case SECT_BEACH :
            terrain_chance = 1;
            send_string = "You hunker down in the sand.";
            break;
        case SECT_STREAM :            
        case SECT_UNDERWATER :            
        case SECT_RIVER :            
        case SECT_WATER_SURFACE :            
        case SECT_AIR :
            send( ch, "You cannot hide here.\n\r" );
            return;
        default :
            code_bug( "HIDE : Invalid Terrain." );
            return;
        
    }
    if( number_range(1, 20) > ch->shdata->skill[ SKILL_HIDE ] + terrain_chance ) {
        send( ch, "You attempt to conceal yourself, but fail.\n\r" );
        return;
    }
        
    for( int i = 0; i < *ch->array; i++ ) 
        if( ( rch = character( ch->array->list[i] ) ) != NULL && rch != ch && ch->Seen( rch ) )
            ch->seen_by += rch;

    send( ch, "%s\n\r", send_string );
     
    set_bit( &ch->status, STAT_HIDING );

    ch->improve_skill( SKILL_HIDE );
    
    if ( ch->pcdata->clss == CLSS_THIEF )
        if ( ch->shdata->level > 59 && terrain_chance > number_range(1,10) ) {
            send( ch, "You manage to camouflage yourself into the surroundings.\n\r");
            set_bit( &ch->status, STAT_CAMOUFLAGED );
        }    
         
}

bool leave_shadows( char_data* ch )
{
    if( !is_set( &ch->status, STAT_HIDING ) && !is_set( &ch->status, STAT_CAMOUFLAGED ) )
        return FALSE;
  
    remove_bit( &ch->status, STAT_CAMOUFLAGED );
    remove_bit( &ch->status, STAT_HIDING );
    
    clear( ch->seen_by );

    send( ch, "You stop hiding.\n\r" );
    send_seen( ch, "%s steps from the shadows.\n\r", ch );
    return TRUE;
}


/* 
 *   DIP ROUTINE
 */


void do_dip( char_data* ch, char* argument )
{
    char              arg  [ MAX_INPUT_LENGTH ];
    obj_data*   container;
    obj_data*         obj;
    int             value;
    affect_data    affect;
    int             spell;

    if( !two_argument( argument, "from", arg ) ) {
        send( ch, "Syntax: Dip <object> [into] <object>\n\r" );
        return;
    }

    if( ( obj = one_object( ch, arg, "dip", &ch->contents, ch->array ) ) == NULL ) 
        return;

    if( ( container = one_object( ch, argument, "dip into", &ch->contents, ch->array ) ) == NULL ) 
        return;

    if( container->pIndexData->item_type != ITEM_DRINK_CON && container->pIndexData->item_type != ITEM_FOUNTAIN ) {
        send( ch, "%s isn't something you can dip things into.\n\r", container );
        return;
    }

    if( container == obj ) {
        send( ch, "You can't dip %s into itself.\n\r", obj );
        return;
    }

    value               = container->value[1];
    container->value[1] = -2;

    if( value == 0 ) {
        send( ch, "%s is empty.\n\r", container );
        container->value[1] = value;
        return;
    }

    if( strip_affect( obj, AFF_BURNING ) ) {
        fsend( ch, "You extinguish %s by quickly dipping it into %s.", obj, container );
        fsend( *ch->array, "%s extinguishes %s by quickly dipping it into %s.", ch, obj, container );
        return;
    } 

    send( ch, "You dip %s into %s.\n\r", obj, container );
    send_seen( ch, "%s dips %s into %s.\n\r", ch, obj, container );
    container->value[1] = ( value == -1 ? -1 : max( 0, value-50 ) );

    if( ( spell = liquid_table[container->value[2]].spell ) == -1 ) 
        return;

    if( spell < SPELL_FIRST || spell >= WEAPON_FIRST ) {
        bug( "Do_dip: Liquid with non-spell skill." );
        return;
    }

    ( *spell_table[spell-SPELL_FIRST].function )( ch, NULL, obj, 10, -3 ); 

    return;
}  
