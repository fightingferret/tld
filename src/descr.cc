#include "ctype.h"
#include "sys/types.h"
#include "stdio.h"
#include "stdlib.h"
#include "define.h"
#include "struct.h"


void  send_descr        ( char_data*, const char* );
void  extract           ( extra_data*, wizard_data*, char );

bool will_fit_desc( char_data* ch, obj_data* obj, int i );
const char*   obj_wear_loc_desc( char_data* ch, obj_data* obj );
const char* loc_desc_lobj[ ] =
{ 
    "0a",
    "1a",
    "2a",
    "3a",
    "4a",
    "5a",
    "6a",
    "7a",
    "8a", 
    "9a",
    "10a",
    "11a",
    "12a",
    "13a",
    "14a",
    "15a",
    "16a",
    "17a",
    "18a",
    "19a",
    "20a",
    "21a",
    "22a",
    "23a", 
    "24a",
    "25a",
};

/*
 *   READ/WRITE EXTRAS
 */


void read_extra( FILE* fp, extra_array& list )
{
    extra_data*   extra;
    char         letter;

    for( ; ; ) {
        if( ( letter = fread_letter( fp ) ) != 'E' )
            break;
        extra           = new extra_data;
        extra->keyword  = fread_string( fp, MEM_EXTRA );
        extra->text     = fread_string( fp, MEM_EXTRA );
        list += extra;
    }

    if( letter != '!' )
        ungetc( letter, fp );
}


void write_extras( FILE* fp, extra_array& list )
{
    for( int i = 0; i < list; i++ ) {
        fprintf( fp, "E\n" );
        fprintf( fp, "%s~\n", list[i]->keyword );
        fprintf( fp, "%s~\n", list[i]->text );
    }
}


/* 
 *   SUPPORT ROUTINES
 */


void show_extras( char_data* ch, extra_array& list )
{
    char tmp [ 3*MAX_STRING_LENGTH ];

    if( is_empty( list ) )
        return;

    page( ch, "\n\rExtra Descriptions:\n\r" );

    for( int i = 0; i < list; i++ ) {
        sprintf( tmp, "[ %s ]\n\r%s", list[i]->keyword, list[i]->text );
        page( ch, tmp );
    }

    return;
}


void page_descr( char_data* ch, const char* text )
{
    char tmp  [ 3*MAX_STRING_LENGTH ];

    convert_to_ansi( ch, text, tmp );
    page( ch, tmp );

    return;
}


/*
 *   SUBROUTINES
 */


const char* Extra_Data :: Keywords( char_data* )
{
    return keyword;
}


void Obj_Data :: Look_At( char_data* ch )
{
    page_descr( ch, is_set( extra_flags, OFLAG_IDENTIFIED ) ? after_descr( pIndexData ) : before_descr( pIndexData ) ); 

    if( pIndexData->item_type == ITEM_ARMOR ) {
        page( ch, "\n\r      Condition: %s\n\r", condition_name( ch, TRUE ) );
        page( ch, "  Wear Location: %s\n\r", obj_wear_loc_desc( ch, this ) );
    }
    else if( pIndexData->item_type == ITEM_WEAPON ) {
        page( ch, "\n\r    Condition: %s\n\r Weapon Class: %s\n\r",
        condition_name( ch, TRUE ),
        weapon_class( pIndexData ) );
    }
    else if( pIndexData->item_type == ITEM_DECK_CARDS ) { 
        look_cards( ch, this );
    }
    else if( pIndexData->item_type == ITEM_KEYRING ) {
        look_in( ch, this );
    }
    else if( pIndexData->item_type == ITEM_DRINK_CON ) {
        page( ch, "\n\rVolume: %.2f liters\n\r",
        float( value[0]/100. ) );
    } 
    else if( pIndexData->item_type == ITEM_CONTAINER ) {
        if( !is_set( &value[1], CONT_CLOSED ) ) {
            page( ch, "\n\rCapacity: %.2f lbs\n\r",
            float( pIndexData->value[0] ) );
            page( ch, "Contains: %.2f lbs\n\r", contents.weight/100. );
        }
    }
}


const char* before_descr( obj_clss_data* obj )
{
    extra_data* ed;

    if( ( obj = get_obj_index( obj->fakes ) ) == NULL )
        return "Item is bugged - Fakes non-existent object.\n\r"; 

    for( int i = 0; i < obj->extra_descr; i++ ) {
        ed = obj->extra_descr[i]; 
        if( !strcmp( ed->keyword, "either" ) || !strcmp( ed->keyword, "before" ) )
            return ed->text;
    }

    return "Item is bugged - Missing non-identified description.\n\r";
}


const char* after_descr( obj_clss_data* obj )
{
    extra_data*  ed;

    for( int i = 0; i < obj->extra_descr; i++ ) {
        ed = obj->extra_descr[i];
        if( !strcmp( ed->keyword, "either" ) || !strcmp( ed->keyword, "after" ) )
            return ed->text;
    }

    return "Item is bugged - Missing identified description.\n\r";
}


/*
  for( ed = obj->extra_descr; ; ed = ed->next ) {
    if( ed == NULL ) {
      if( !first || ( ed = obj->pIndexData->extra_descr ) == NULL )
        break;
      first = FALSE;
      }
    if( is_name( argument, ed->keyword ) 
      && strcmp( ed->keyword, "before" )
      && strcmp( ed->keyword, "after" )
      && strcmp( ed->keyword, "either" ) ) {  
      send_descr( ch, ed->text, obj, FALSE );
      return TRUE;
      }
    }

  return FALSE;
}
*/


/*
 *   EDITING ROUTINE
 */


void edit_extra( extra_array& array, wizard_data* wizard, int offset, char* argument, char* text )
{
    char            arg  [ MAX_INPUT_LENGTH ];
    extra_data*      ed;
    extra_data**   edit;
    int               i;

    edit = (extra_data**) ( int( wizard )+offset );

    if( *argument == '\0' ) {
        if( is_empty( array ) ) 
            send( wizard, "No extras found.\n\r" );
        else 
            for( i = 0; i < array; i++ ) {
                page( wizard, "[%2d]  %s\n\r", i+1, array[i]->keyword );
                page( wizard, array[i]->text );
            }
        return;
    }

    if( exact_match( argument, "delete" ) ) {
        if( ( i = atoi( argument )-1 ) < 0 || i >= array )   
            send( wizard, "Extra not found to remove.\n\r" );
        else {
            send( wizard, "Extra %d, '%s' removed.\n\r", i+1, array.list[i]->keyword );
            remove( array, i );
            *edit = array.list[i]; 
            extract( wizard, offset, text );
        }
        return;
    }

    if( exact_match( argument, "new" ) ) {
        ed          = new extra_data;
        ed->keyword = alloc_string( argument, MEM_EXTRA );
        ed->text    = alloc_string( "No description.\n\r", MEM_EXTRA );
        *edit       = ed; 
        array += ed;
        send( wizard, "Extra %s created.\n\r", ed->keyword );
        return;
    }

    argument = one_argument( argument, arg );

    if( ( i = atoi( arg )-1 ) < 0 || i >= array ) {
        for( i = 0; ; i++ ) {
            if( i == array ) {
                send( wizard, "Extra not found.\n\r" );
                return;
            }
            if( is_name( arg, array[i]->keyword ) ) 
            break;
        }
    }

    ed = array[i]; 

  if( *argument != '\0' ) {
    free_string( ed->keyword, MEM_OBJ_CLSS );
    ed->keyword = alloc_string( argument, MEM_OBJ_CLSS );
    send( wizard, "Extra name set to: %s\n\r", ed->keyword );
    return;
    }

    *edit = ed;

    send( wizard, "%s now operates on extra %d, '%s'.\n\r", text, i+1, ed->keyword );
}

const char* obj_wear_loc_desc( char_data* ch, obj_data* obj )
{
    int j;
    int i;
    
    for( i = 0; i < MAX_LAYER && !is_set( &obj->pIndexData->layer_flags, i ); i++ ){}
       
    for( j = 0; j < MAX_WEAR && !will_fit_desc( ch, obj, j ); j++ ){}
       
    obj->position = j;
    obj->layer = i;

    switch( obj->position ) {
        case WEAR_BODY:      
            switch( obj->layer ) {
                case LAYER_BOTTOM:
                    return "Worn under a shirt";
                case LAYER_UNDER:
                    return "Worn as a shirt";
                case LAYER_BASE:
                    return "Worn as a jacket";
                case LAYER_OVER:
                    return "Worn as a cloak";
                case LAYER_TOP:
                    return "Worn on back.";
                default:
                    return "BUG: Unkown body layer";
            }
        case WEAR_HEAD:
            switch( obj->layer ) {
                case LAYER_BOTTOM:
                    return "Head trinket";
                case LAYER_UNDER:
                    return "Worn as a cap";
                case LAYER_BASE:
                    return "Worn as a helm";
                case LAYER_OVER:
                    return "Worn as a cloak";
                case LAYER_TOP:
                    return "Head trinket";
                default:
                    return "BUG: Unkown head layer";
            }
        case WEAR_LEGS:
            switch( obj->layer ) {
                case LAYER_BOTTOM:
                    return "???";
                case LAYER_UNDER:
                    return "Worn under leg armor";
                case LAYER_BASE:
                    return "Worn as leg armor";
                case LAYER_OVER:
                    return "Worn as pants";
                case LAYER_TOP:
                    return "???";
                default:
                    return "BUG: Unkown leg layer";
            }
        case WEAR_FEET:
            switch( obj->layer ) {
                case LAYER_BOTTOM:
                    return "???";
                case LAYER_UNDER:
                    return "Worn as socks";
                case LAYER_BASE:
                    return "Worn as boots";
                case LAYER_OVER:
                    return "???";
                case LAYER_TOP:
                    return "???";
                default:
                    return "BUG: Unkown feet layer";
            }
        case WEAR_ARMS:
            switch( obj->layer ) {
                case LAYER_BOTTOM:
                    return "???";
                case LAYER_UNDER:
                    return "Worn as arm padding";
                case LAYER_BASE:
                    return "Worn as arm armor";
                case LAYER_OVER:
                    return "Hung around arms";
                case LAYER_TOP:
                    return "???";
                default:
                    return "BUG: Unkown arm layer";
            }
        case WEAR_HANDS:
            switch( obj->layer ) {
                case LAYER_BOTTOM:
                    return "???";
                case LAYER_UNDER:
                    return "???";
                case LAYER_BASE:
                    return "Worn as gauntlets";
                case LAYER_OVER:
                    return "???";
                case LAYER_TOP:
                    return "???";
                default:
                    return "BUG: Unkown hand layer";
            }
        case WEAR_WRIST_L:
            switch( obj->layer ) {
                case LAYER_BOTTOM:
                    return "Wrist jewelry";
                case LAYER_UNDER:
                    return "Wrist jewelry";
                case LAYER_BASE:
                    return "Wrist jewelry";
                case LAYER_OVER:
                    return "Wrist jewelry";
                case LAYER_TOP:
                    return "Wrist jewelry";
                default:
                    return "BUG: Unkown wrist layer";
            }
        case WEAR_FINGER_L:
            switch( obj->layer ) {
                case LAYER_BOTTOM:
                    return "Finger jewelry";
                case LAYER_UNDER:
                    return "Finger jewelry";
                case LAYER_BASE:
                    return "Finger jewelry";
                case LAYER_OVER:
                    return "Finger jewelry";
                case LAYER_TOP:
                    return "Finger jewelry";
                default:
                    return "BUG: Unkown finger layer";
            }
        case WEAR_NECK:
            switch( obj->layer ) {
                case LAYER_BOTTOM:
                    return "Neck Jewelry";
                case LAYER_UNDER:
                    return "Neck Jewelry";
                case LAYER_BASE:
                    return "Neck Jewelry";
                case LAYER_OVER:
                    return "Neck Jewelry";
                case LAYER_TOP:
                    return "Neck Jewelry";
                default:
                    return "BUG: Unkown neck layer";
            }
        case WEAR_FLOATING:
            switch( obj->layer ) {
                case LAYER_BOTTOM:
                    return "Light";
                case LAYER_UNDER:
                    return "Light";
                case LAYER_BASE:
                    return "Light";
                case LAYER_OVER:
                    return "Light";
                case LAYER_TOP:
                    return "Light";
                default:
                    return "BUG: Unkown float layer";
            }
        default:
            return "Item is bugged - Missing wear_loc description.\n\r";
            break;
    }
    
    return "Item is bugged - Missing wear_loc description.\n\r";

}

bool will_fit_desc( char_data* ch, obj_data* obj, int i )
{
    if( !is_set( &obj->pIndexData->wear_flags, wear_index[i] ) ) 
        return FALSE;

    if( ch->species == NULL )
        return( wear_index[i] != ITEM_TAKE && i < MAX_WEAR_HUMANOID );
//    return( i < MAX_WEAR_HUMANOID );

    return is_set( &ch->species->wear_part, i );
}