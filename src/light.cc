#include "math.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "define.h"
#include "struct.h"


/*
 *   SUNLIGHT
 */


const char* light_name( int i )
{
    const char* adj [] = { "Pitch Dark", "Extremely Dark", "Dark", "Dimly Lit", "Lit", "Well Lit", "Brightly Lit", "Blinding" };

    i = (int) log( (float) 2+i );
    i = min( i, 7 );

    return adj[i];
}


int sunlight( int time )
{
    int sunrise = 5*60;
    int sunset  = 20*60;

    if( time < sunrise || time > sunset ) 
        return 10;

    double x = 10+500*sin(M_PI*(time-sunrise)/(sunset-sunrise)); 

    return (int) x;
}


/*
 *   ILLUMINATION
 */


int Thing_Data  :: Light( int n )
{
    obj_data *obj = object (this);
    if (obj) 
        return obj->Light (n);

    player_data *plr = player (this);
    if (plr) 
        return plr->Light (n);

    mob_data *mobb = mob (this);
    if (mobb) 
        return mobb->Light (n);

    room_data *room = Room (this);
    if (room) 
        return room->Light ();

    return 0;
}
int Player_Data :: Light( int )  { return wearing.light; }
int Mob_Data    :: Light( int )  { return wearing.light+species->light; }


int Obj_Data :: Light( int n )
{
    if (pIndexData->item_type != ITEM_LIGHT) 
        return 0;

    int i  = pIndexData->light;

    i *= ( n == -1 ? number : n );

    return i; 
}   


int Room_Data :: Light( ) 
{
    int i = contents.light;

    if (is_set (&room_flags, RFLAG_LIT) || (isday( ) && (!is_set (&room_flags, RFLAG_INDOORS)))) ++ i;

    return i;
} 


/*
 *   SPELLS
 */


bool spell_create_light( char_data* ch, char_data*, void*, int level, int )
{
    obj_data*     light;
    event_data*   event;

    if( null_caster( ch, SPELL_CREATE_LIGHT ) )
        return TRUE;

    light = create( get_obj_index( OBJ_BALL_OF_LIGHT ) );

    light->value[2] = min (level, 7) * 4;

    if( ( ch->contents.number + 1 ) <= ch->can_carry_n( ) ) {
        light->To( ch );
        send( ch, "You manage to capture %s floating near you.\n\r", light );
        send( *ch->array, "%s appears floating near %s.\n\r", light, ch );
        }
    else {
        light->To( ch->in_room );
        send( ch, "You can't handle %s.\n\r", light );
        send( *ch->array, "%s drops %s on the floor.\n\r", ch, light );
    }

    return TRUE;    
}


bool spell_continual_light( char_data *ch, char_data *victim, void*, int level, int duration )
{
    spell_affect( ch, victim, level, duration, SPELL_CONTINUAL_LIGHT, AFF_CONTINUAL_LIGHT );

    return TRUE;
}


