#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "define.h"
#include "struct.h"


bool   remove           ( char_data*, obj_data*& );
void   wear_obj         ( char_data*, obj_data*, int );
void   wear_trigger     ( char_data*, obj_data* ); 
void   wield_obj        ( char_data*, obj_data* ); 
bool   can_remove       ( char_data*, obj_data*, bool = FALSE );

const char* wear_name [ MAX_ITEM_WEAR ] = { "take", "finger", "neck", "body", "head",
    "legs", "feet", "hands", "arms", "floating", "unused0", "waist", "wrist",
    "right.hand", "left.hand", "unused1", "unused2", "unused3", "unused4",
    "horse_body", "horse_back", "horse_foot" };

const int wear_index [ MAX_WEAR ] = { ITEM_WEAR_FLOATING, ITEM_WEAR_FINGER,
    ITEM_WEAR_FINGER, ITEM_WEAR_NECK, ITEM_WEAR_NECK, ITEM_WEAR_BODY,
    ITEM_WEAR_HEAD, ITEM_WEAR_LEGS, ITEM_WEAR_FEET, ITEM_WEAR_HANDS,
    ITEM_WEAR_ARMS, ITEM_TAKE, ITEM_TAKE, ITEM_WEAR_WAIST,
    ITEM_WEAR_WRIST, ITEM_WEAR_WRIST, ITEM_HELD_R, ITEM_HELD_L,
    ITEM_TAKE, ITEM_TAKE, ITEM_TAKE, ITEM_TAKE,
    ITEM_WEAR_HORSE_BODY, ITEM_WEAR_HORSE_BACK, ITEM_WEAR_HORSE_FEET };
  
const char** wear_part_name = &reset_wear_name[2];

const char* wear_abbrev[ MAX_WEAR + 2 ] = { "Grnd", "Inv.", "Lght", "rFgr", "lFgr",
    "Neck", "??  ", "Body", "Head", "Legs", "Feet", "Hand", "Arms", "??  ",
    "??  ", "Wst.", "rWrs", "lWrs", "rHnd", "lHnd", "??  ", "??  ", "??  ",
    "??  ", "??  ", "??  ", "??  " };

const char* reset_wear_name[ MAX_WEAR + 2 ] = { "ground", "inventory",
    "floating", "right.finger", "left.finger",
    "neck", "unused1", "body", "head", "legs", "feet", "hands", "arms",
    "unused1", "unused2", "about.waist", "right.wrist", "left.wrist",
    "right.hand", "left.hand", "unused3", "unused4", "unused5", "unused6",
    "horse.body", "horse.back", "horse.foot" };


const char* wear_verb [ MAX_WEAR ] =
{ 
    "toss into the air",
    "place on your left hand",
    "place on your right hand",
    "place around your neck",
    "place around your neck",
    "wear",
    "place on your head",
    "cover your legs with", 
    "slip your feet into",
    "put your hands into",
    "cover your arms with",
    "??2",
    "??3",
    "place around your waist",
    "put on your right wrist",
    "put on your left wrist",
    "hold in your right hand",
    "hold in your left hand",
    "??4",
    "??5",
    "??6",
    "??7",
    "covering body", 
    "placed on back",
    "attached to feet",
};

 
const char* where_name [ MAX_WEAR ] =
{
    "floating nearby   ",
    "right hand finger ",
    "left hand finger  ",
    "worn around neck  ",
    "worn around neck  ",
    "worn on body      ",
    "worn on head      ",
    "worn on legs      ",
    "worn on feet      ",
    "worn on hands     ",
    "worn on arms      ",
    "??9               ",
    "??10              ",
    "worn about waist  ",
    "right wrist       ",
    "left wrist        ",
    "right hand        ",
    "left hand         ",
    "??11              ",
    "??12              ", 
    "??13              ",
    "??14              ",
    "covering body     ", 
    "placed on back    ",
    "attached to feet  ",
};


obj_data *get_eq_char (char_data *victim, int position, int layer)
{
    obj_data * obj;
  
    for (int a = 0; a < victim->wearing.size; ++ a){
        obj = object (victim->wearing [a]);
        if (obj->position == position && obj->layer == layer)
            return obj;
    }
    return NULL;
}

/*
 *   SUPPORT ROUTINES
 */ 

const char* wear_loc( obj_data* obj )
{
    if( obj->position == WEAR_HELD_R || obj->position == WEAR_HELD_L ) {
        if( obj->pIndexData->item_type == ITEM_WEAPON )
            return "you are wielding";
        return "you are holding";
    }

    if( obj->position == WEAR_FLOATING )
        return "floating nearby";

    return "you are carrying";
}
  

obj_data* char_data :: Wearing( int slot, int layer )
{
    obj_data* obj;

    for( int j = 0; j < wearing; j++ ) {
        obj = (obj_data*) wearing[j];
        if( obj->position == slot && ( layer == -1 || obj->layer == layer ) ) 
            return obj;
    }

    return NULL;
}


/*
 *   CAN_USE ROUTINES
 */


bool will_fit( char_data* ch, obj_data* obj, int i )
{
    if( !is_set( &obj->pIndexData->wear_flags, wear_index[i] ) ) 
       return FALSE;
    
  /*  CUSTOM FIT  */
// Sadis is a real bastard and put custom fit information here
// someone else might want to fiddle around and do something else for
// size and custom fit messages
    
    if( is_set( &obj->pIndexData->size_flags, SFLAG_CUSTOM ) && ( ch->pcdata == NULL || ( ch->pcdata->pfile->ident != obj->size_flags ) ) ) 
       return FALSE;

    if( is_set( &obj->pIndexData->size_flags, SFLAG_RACE ) && ch->shdata->race < MAX_PLYR_RACE && !is_set( &obj->pIndexData->size_flags, SFLAG_HUMAN+ch->shdata->race ) ) 
       return FALSE;
/*
    if( is_set( &obj->pIndexData->size_flags, SFLAG_SIZE ) || ( obj != NULL && is_set( &obj->pIndexData->size_flags, SFLAG_RANDOM ) ) ) 
       if( !is_set( obj == NULL ? &obj->pIndexData->size_flags : &obj->size_flags, wear_size( ch ) ) ) 
          return FALSE;
*/

    if( is_set( &obj->pIndexData->size_flags, SFLAG_SIZE ) )
       if( !is_set( &obj->pIndexData->size_flags, wear_size( ch ) ) )
           return FALSE;

    if( ch->species == NULL )
       return( wear_index[i] != ITEM_TAKE && i < MAX_WEAR_HUMANOID );

    return is_set( &ch->species->wear_part, i );
}


bool can_use( char_data* ch, obj_clss_data* obj_clss, obj_data* obj, bool custom )
{
    if( ch->species != NULL && !is_set( &ch->species->act_flags, ACT_HUMANOID ) ) 
       return FALSE;

    /*  CUSTOM FIT  */

    if( is_set( &obj_clss->size_flags, SFLAG_CUSTOM ) && ( ch->pcdata == NULL || ( !custom && ch->pcdata->pfile->ident != obj->size_flags ) ) ) 
       return FALSE;

    /*  SIZE */

    if( is_set( &obj_clss->size_flags, SFLAG_RACE ) && ch->shdata->race < MAX_PLYR_RACE && !is_set( &obj_clss->size_flags, SFLAG_HUMAN+ch->shdata->race ) ) 
       return FALSE;
/*
    if( is_set( &obj_clss->size_flags, SFLAG_SIZE ) || ( obj != NULL && is_set( &obj_clss->size_flags, SFLAG_RANDOM ) ) ) 
       if( !is_set( obj == NULL ? &obj_clss->size_flags : &obj->size_flags, wear_size( ch ) ) ) 
          return FALSE;
*/
    if( is_set( &obj_clss->size_flags, SFLAG_SIZE ) )
       if( !is_set( &obj_clss->size_flags, wear_size( ch ) ) )
          return FALSE;
   
    /*  CLASS RESTRICTIONS */

    if( ch->shdata->level < obj_clss->level ) 
       return FALSE;

    if( ch->shdata->level < LEVEL_BUILDER && ( ch->shdata->race < MAX_PLYR_RACE && is_set( &obj_clss->anti_flags, ANTI_HUMAN+ch->shdata->race ) ) || ( ch->species == NULL && is_set( &obj_clss->anti_flags, ANTI_MAGE+ch->pcdata->clss ) ) || is_set( &obj_clss->anti_flags, ANTI_GOOD+ch->shdata->alignment%3 ) || is_set( &obj_clss->anti_flags, ANTI_LAWFUL+ch->shdata->alignment/3 ) ) 
       return FALSE; 
    
    if( ch->pcdata != NULL && ch->pcdata->clss == CLSS_CLERIC && is_set( &obj_clss->restrictions, RESTR_BLADED ) )
       return FALSE;
       
    return TRUE;
}


/*
 *   WEAR ROUTINES
 */ 


thing_data* cantwear( thing_data* t1, char_data* ch, thing_data* )
{
    obj_data* obj = (obj_data*) t1;

    for( int i = 0; !is_set( &obj->pIndexData->layer_flags, i ); )
        if( ++i == MAX_LAYER )
            return NULL; 
 
    for( int i = 0; i < MAX_WEAR; i++ ) 
        if( will_fit( ch, obj, i ) )
            return obj;

    return NULL;
}

thing_data* notcustom( thing_data* t1, char_data* ch, thing_data* )
{
  obj_data* obj = (obj_data*) t1;
  /*  CUSTOM FIT  */
  if( is_set( &obj->pIndexData->size_flags, SFLAG_CUSTOM ) && ( ch->pcdata == NULL || ( ch->pcdata->pfile->ident != obj->size_flags ) ) ) 
    return FALSE;

  return obj;
}

thing_data* notrace( thing_data* t1, char_data* ch, thing_data* )
{
  obj_data* obj = (obj_data*) t1;
 /* RACE RESTRICTIONS */
    if( is_set( &obj->pIndexData->size_flags, SFLAG_RACE ) && ch->shdata->race < MAX_PLYR_RACE && !is_set( &obj->pIndexData->size_flags, SFLAG_HUMAN+ch->shdata->race ) ) 
        return FALSE;
  return obj;
}

thing_data* notsize( thing_data* t1, char_data* ch, thing_data* )
{
  obj_data* obj = (obj_data*) t1;
/* SIZE RESTRICTIONS */
  if( is_set( &obj->pIndexData->size_flags, SFLAG_SIZE ) ) 
    if( !is_set( &obj->pIndexData->size_flags, wear_size( ch ) ) ) 
      return FALSE;

  return obj;
}
thing_data* bladed( thing_data* t1, char_data* ch, thing_data* )
{
    obj_data* obj = (obj_data*) t1;
    if( ch->pcdata != NULL && ch->pcdata->clss == CLSS_CLERIC && is_set( &obj->pIndexData->restrictions, RESTR_BLADED ) ) 
        return NULL;

    return obj;
}


thing_data* dishonorable( thing_data* t1, char_data* ch, thing_data* )
{
    obj_data* obj = (obj_data*) t1;

    if( ch->pcdata != NULL && ch->pcdata->clss == CLSS_PALADIN && is_set( &obj->pIndexData->restrictions, RESTR_DISHONORABLE ) ) 
        return NULL;

    return obj;
}


thing_data* levellimit( thing_data* t1, char_data* ch, thing_data* )
{
    obj_data* obj  = (obj_data*) t1;

    if( ch->shdata->level < obj->pIndexData->level ) 
        return NULL;

    return obj;
}


thing_data* antiobj( thing_data* t1, char_data* ch, thing_data* )
{
    obj_data*            obj  = (obj_data*) t1;
    obj_clss_data*  obj_clss  = obj->pIndexData;

    if( ch->shdata->level < LEVEL_BUILDER && ( ch->shdata->race < MAX_PLYR_RACE && is_set( &obj_clss->anti_flags, ANTI_HUMAN+ch->shdata->race ) ) || ( ch->species == NULL && is_set( &obj_clss->anti_flags, ANTI_MAGE+ch->pcdata->clss ) ) || is_set( &obj_clss->anti_flags, ANTI_GOOD+ch->shdata->alignment%3 ) || is_set( &obj_clss->anti_flags, ANTI_LAWFUL+ch->shdata->alignment/3 ) ) 
        return NULL; 

    return obj;
}


thing_data* needremove( thing_data* t1, char_data* ch, thing_data* t2 )
{

    obj_data*       obj  = (obj_data*) t1;
    thing_array*  array  = (thing_array*) t2;
    obj_data*      worn;
    int               j;
    obj_data* obj2;

    if( is_set( obj->extra_flags, OFLAG_NO_SHIELD ) && ch->Wearing( WEAR_HELD_L, LAYER_BASE ) )
        return NULL;

    if( ( obj2 = ch->Wearing( WEAR_HELD_R, LAYER_BASE ) ) != NULL)
        if( ( is_set(obj2->extra_flags, OFLAG_NO_SHIELD ) ) && ( will_fit( ch, obj, WEAR_HELD_L ) ) )
            return NULL;

    for( int i = 0; i < MAX_WEAR; i++ ) 
        if( will_fit( ch, obj, i ) ) {
            for( j = 0; j < MAX_LAYER; j++ )  
                if( is_set( &obj->pIndexData->layer_flags, j ) && !ch->Wearing( i, j ) ) {
                    worn = (obj_data*) obj->From( 1 );
                    worn->position = i;
                    obj->position  = i;
                    worn->layer    = j;
                    worn->To( &ch->wearing );
                    if( obj->selected == 1 ) 
                        return obj;
                    obj->selected--;
                    if( array != NULL )
                        *array += worn;
                }
        }
    
    return NULL;
    
}


void do_wear( char_data* ch, char* argument )
{
    thing_array* array;
    obj_data* obj;
    char            arg  [ MAX_INPUT_LENGTH ];

    if( is_confused_pet( ch ) )
        return; 


    if( ( array = several_things( ch, argument, "wear", &ch->contents ) ) == NULL )
    return;
/*
    thing_array   subset  [ 8 ];
    thing_func*     func  [ 8 ]  = { cantwear, stolen, bladed,
                                   dishonorable, levellimit,
                                   antiobj, needremove, NULL };
*/
    thing_array   subset [ 11 ];
    thing_func*     func [ 11 ] = { notcustom, notrace, notsize,
                                    cantwear, stolen, bladed,
                                    dishonorable, levellimit,
                                    antiobj, needremove, NULL };


    sort_objects( ch, *array, (thing_data*) &subset[10], 11, subset, func );

    msg_type = MSG_EQUIP;

    page_priv( ch, NULL, empty_string );
    page_priv( ch, &subset[0], NULL, NULL,  "was constructed for someone else", "were constructed for someone else" );
    page_priv( ch, &subset[1], NULL, NULL, "isn't designed for your race", "aren't designed for your race" );
    page_priv( ch, &subset[2], NULL, NULL, "doesn't fit", "don't fit" );
    page_priv( ch, &subset[3], "can't wear" );
    page_priv( ch, &subset[4], NULL, NULL, "is stolen", "are stolen" );
    page_priv( ch, &subset[5], NULL, NULL, "is a bladed weapon and forbidden to you", "are bladed weapons and forbidden to you" ); 
    page_priv( ch, &subset[6], "you consider", NULL, "dishonorable" );
    page_priv( ch, &subset[7], "can't cope with" );
    page_priv( ch, &subset[8], "sense adversion from" );
    page_priv( ch, &subset[9], "need to remove something to equip" );
    list_wear( ch, &subset[10] );

    delete array;
}


void equip( char_data* ch, obj_data* obj )
{
    obj_data*    worn;
    affect_data    af;

    if (obj->pIndexData->item_type == ITEM_LIGHT) {
        if (ch->in_room) ch->in_room->contents.light += obj->pIndexData->light;
            ch->wearing.light += obj->pIndexData->light;
    }

    if( !is_set( obj->pIndexData->extra_flags, OFLAG_ADDITIVE ) )
        for( int i = 0; i < ch->wearing; i++ )
            if( ( worn = object( ch->wearing[i] ) ) != NULL && worn->pIndexData == obj->pIndexData && worn != obj )
                return;

    for( int i = 0; i < MAX_ENTRY_AFF_CHAR; i++ )
        if( is_set( obj->pIndexData->affect_flags, i ) ) {
            af.type = i;
            modify_affect( ch, &af, TRUE );
        }

    for( int i = 0; i < obj->pIndexData->affected; i++ )
        modify_affect( ch, obj->pIndexData->affected[i], TRUE );
}


/*
 *   REMOVE ROUTINES
 */


thing_data* noremove( thing_data* thing, char_data*, thing_data* )
{
    obj_data* obj = (obj_data*) thing;

    return( is_set( obj->extra_flags, OFLAG_NOREMOVE ) ? NULL : obj );
}


thing_data* nocatch( thing_data* thing, char_data*, thing_data* )
{
    obj_data* obj = (obj_data*) thing;

    if (is_set (obj->extra_flags, OFLAG_NOREMOVE) && (obj->position == WEAR_FLOATING))
        return NULL;

    return obj;

}


thing_data* remove( thing_data* thing, char_data* ch, thing_data* )
{
    obj_data *obj = (obj_data *) thing;

    obj->From( 1 );
    obj->To( ch );

    return thing;
}


void do_remove( char_data* ch, char* argument )
{
    char            arg  [ MAX_INPUT_LENGTH ];
    thing_array*  array;
    char_data * victim;
    obj_data *item;

    if( is_confused_pet( ch ) ) 
        return;

    if( contains_word( argument, "from", arg ) ) {
        if( ( victim = one_character (ch, argument, "remove from", ch->array)) != NULL ) {   
            if ((item = one_object (victim, arg, "remove from", &victim->wearing)) == NULL) {
                send( ch, "%s isn't wearing anything like that.\n\r", victim );
            }
            else {
                if( victim->leader != ch || !is_set( &victim->status, STAT_PET ) ) {
                    send( ch, "%s isn't a pet of yours.\n\r", victim );
                    return;
                }
                send( ch, "You remove %s from %s.\n\r", item, victim );
                send_seen( ch, "%s removes %s from %s.\n\r", ch, item, victim );
                item->From ();
                item->To (&ch->contents);
            }  
        }
        return;
    }

    if( ( array = several_things( ch, argument, "remove", &ch->wearing ) ) == NULL )
        return;

    thing_array   subset  [ 3 ];
    thing_func*     func  [ 3 ]  = { nocatch, noremove, remove };

    sort_objects( ch, *array, NULL, 3, subset, func );

    msg_type = MSG_EQUIP;

    page_priv( ch, NULL, empty_string );
    page_priv( ch, &subset[0], "can't catch" );
    page_priv( ch, &subset[1], "can't remove" );
    page_publ( ch, &subset[2], "remove" );

    consolidate( subset[2] );

    delete array;
}


void unequip( char_data* ch, obj_data* obj )
{
    obj_data*    worn;
    affect_data    af;

    if (obj->pIndexData->item_type == ITEM_LIGHT){
        if (ch->in_room) 
            ch->in_room->contents.light -= obj->pIndexData->light;
        ch->wearing.light -= obj->pIndexData->light;
    }

    if( !is_set( obj->pIndexData->extra_flags, OFLAG_ADDITIVE ) )
        for( int i = 0; i < ch->wearing; i++ )
            if( ( worn = object( ch->wearing[i] ) ) != NULL && worn->pIndexData == obj->pIndexData && worn != obj )
                return;

    for( int i = 0; i < MAX_ENTRY_AFF_CHAR; i++ )
        if( is_set( obj->pIndexData->affect_flags, i ) ) {
            af.type = i;
            modify_affect( ch, &af, FALSE  );
        }

    for( int i = 0; i < obj->pIndexData->affected; i++ )
        modify_affect( ch, obj->pIndexData->affected[i], FALSE );
}








