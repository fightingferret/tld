#include "ctype.h"
#include "sys/types.h"
#include "stdio.h"
#include "stdlib.h"
#include "define.h"
#include "struct.h"


bool        in_bank            ( char_data* );
obj_data*   obj_from_locker    ( obj_data*, char_data*, int );


/*
 *   BANK FUNCTIONS
 */


bool in_bank( char_data* ch )
{
    room_data* room;

    if( is_mob( ch ) )
        return FALSE;

    if( ( room = Room( ch->array->where ) ) == NULL || !is_set( &room->room_flags, RFLAG_BANK ) ) {
        send( ch, "You are not in a banking institution.\n\r" );
        return FALSE;
    }

    return TRUE;
}


void do_balance( char_data* ch, char* )
{
    thing_data*      thing;
    player_data*        pc;
    obj_data*          obj;

    if( !in_bank( ch ) )
        return;

    pc = player( ch );
    if( is_empty( pc->locker) ) {
        if( pc->bank == 0 ) 
            send( ch, "You have no items or coins stored with the bank.\n\r" );
        else  
            page( ch, "You have %d copper coin%s, but no items stored with the bank.\n\r", pc->bank, pc->bank == 1 ? "" : "s" );
        return;
    }
    page( ch, "Items in Storage                                         Weight\n\r" );
    select( pc->locker );
    rehash_weight( ch, pc->locker );

    for( int i = 0; i < pc->locker; i++ ) {
        thing = pc->locker[i];
        if( thing->shown > 0 ) {
            page( ch, "%-55s %7.2f\n\r", thing->Seen_Name( ch, thing->shown, TRUE ), .01*thing->temp );
        }
    }

    page_header( ch, "Total Weight: %.2f lbs\n\r\n\r", .01*pc->locker.weight );
    page_header( ch, "Copper Coins: %d\n\r", pc->bank );
//  send_color( ch, 27,  "*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*\n\r");
}

void do_locker( char_data* ch, char* argument )
{

    link_data*           link;
    pfile_data*          pfile;
    thing_data*          thing;
    player_data*         victim;
            
    if( *argument == '\0' ) {
       send( ch, "Review whos bank account?\n\r" );
       return;
    }
       	 
    link = new link_data;
    pfile = find_pfile( argument );
    
    if( pfile == NULL ) {
       send( ch, "Noone found by that name.\n\r" );
       return;
    }
    
    load_char( link, pfile->name, PLAYER_DIR );
    victim = link->player;
           
    if( is_empty( victim->locker ) ) {
       if( victim->bank == 0 )
          send( ch, "%s has no items or coins stored in their locker.\n\r", pfile->name );
       else
	  send( ch, "%s has %d copper coin%s, but no items stored in their locker.\n\r", pfile->name, victim->bank, victim->bank == 1 ? "" : "s" );
       return;
    }
        
    page_header( ch, "Total Weight: %.2f lbs\n\r\n\r", .01*victim->locker.weight );
    page_header( ch, "Copper Coins: %d\n\r", victim->bank );
     
    for( int i = 0; i < victim->locker; i++ ) {
        thing = victim->locker[i];
        if( thing->shown > 0 ) {
           page( ch, "%s \n\r", thing->Seen_Name( ch, thing->shown, TRUE ) );
        }   
    }   
       
    victim->Extract( );
    delete link;
    return;
}

/*
 *   DEPOSIT
 */


thing_data* no_room( thing_data* thing, char_data* ch, thing_data* )
{ 
    player_data*  pc  = (player_data*) ch;
    obj_data*    obj  = (obj_data*) thing;
    int            n  = obj->Weight( );
    int            m  = 40000-pc->locker.weight;

    if( obj->pIndexData->item_type == ITEM_MONEY )
        return obj; 

    if( m >= n || n <= 0 )
        return obj;
  
    if( ( m = obj->selected*m/n ) <= 0 )
        return NULL;

    content_array* where = thing->array;

    thing = thing->From( m );
    thing->To( where );

    return thing;
}


thing_data* deposit( thing_data* thing, char_data*, thing_data* )
{ 
    return thing->From( thing->selected );
}


void do_deposit( char_data* ch, char* argument )
{
    thing_array*    array;
    player_data*       pc;
    obj_data*         obj;
    int            amount;

    if( !in_bank( ch ) )
        return;

    pc = player( ch );

    if( *argument == '\0' ) {
        send( ch, "What do you want to deposit?\n\r" );
        return;
    }

    if( is_number( argument ) ) {
        if( ( amount = atoi( argument ) ) <= 0 ) {
            send( ch, "You may only deposit positive amounts.\n\r" );
            return;
        }
        if( !remove_coins( ch, amount, "You deposit" ) ) {
            send( ch, "You don't have that much to deposit.\n\r" );
            return;
        }
        pc->bank += amount;
        send( ch, "You now have %d cp in your account.\n\r", pc->bank );
        return;
    }

    if( ( array = several_things( ch, argument, "deposit", &ch->contents ) ) == NULL )
        return;

    thing_array    subset  [ 5 ];
    thing_func*      func  [ 5 ]  = { cursed, stolen, corpse, no_room, deposit };

    sort_objects( ch, *array, NULL, 5, subset, func );

    msg_type = MSG_BANK;

    page_priv( ch, NULL, empty_string );
    page_priv( ch, &subset[0], "can't let go of" );
    page_priv( ch, &subset[1], "don't own" );
    page_priv( ch, &subset[2], NULL, NULL, "is refused", "are refused" );
    page_priv( ch, &subset[3], "don't have space for" );
    page_publ( ch, &subset[4], "deposit" );

    for( int i = 0; i < subset[4]; i++ ) {
        obj = (obj_data*) subset[4][i];
        if( obj->pIndexData->item_type != ITEM_MONEY ) {
            obj->To( &pc->locker );
            consolidate( obj );
        }
        else {
            pc->bank += monetary_value( obj );
            obj->Extract( );
        }
    }

    delete array;
}


/*
 *   WITHDRAW
 */


const char* auction_msg = "That would take your bank balance below what you have bid on the\n\rauction block and if you did that the delivery daemons would rip you apart\n\rand play catch with the pieces.\n\r";


thing_data* withdraw( thing_data* thing, char_data* ch, thing_data* )
{
    thing = thing->From( thing->selected );
    thing->To( ch );

    return thing;
}


void do_withdraw( char_data* ch, char* argument )
{
    thing_array*   array;
    player_data*      pc;
    int           amount;

    if( !in_bank( ch ) )
        return;

    pc = player( ch );

    if( *argument == '\0' ) {
        send( ch, "What do you want to withdraw?\n\r" );
        return;
    }

    if( is_number( argument ) ) {
        if( ( amount = atoi( argument ) ) <= 0 ) {
            send( ch, "You may only withdraw positive amounts.\n\r" );
        }
        else if( amount > pc->bank ) {
            send( ch, "That is more than you have in your account.\n\r" );
        }
        else if( amount > free_balance( pc ) ) {
            send( ch, auction_msg );
        }
        else {
            add_coins( ch, amount, "The bank teller hands you" );
            pc->bank -= amount;
        }
        return;
    }

    if( ( array = several_things( ch, argument, "withdraw", &pc->locker ) ) == NULL ) 
    return;

  /* HANDLE ITEM LIST */ 

    thing_array   subset  [ 3 ];
    thing_func*     func  [ 3 ]  = { heavy, many, withdraw };

    sort_objects( ch, *array, NULL, 3, subset, func );

    msg_type = MSG_BANK;

    page_priv( ch, NULL, empty_string );
    page_priv( ch, &subset[0], "can't lift" );
    page_priv( ch, &subset[1], "can't handle" );
    page_publ( ch, &subset[2], "withdraw" );

    consolidate( subset[2] );

    delete array;
}   











