#include "sys/types.h"
#include <ctype.h>
#include "stdlib.h"
#include "stdio.h"
#include "define.h"
#include "struct.h"


auction_array  auction_list;


void  transfer_buyer   ( auction_data* ); 
void  transfer_file    ( pfile_data*, thing_array*, int );
void  return_seller    ( auction_data* );
bool  no_auction       ( obj_data* );
bool  stolen_auction   ( char_data*, obj_data* );
void  display_auction  ( player_data* );
bool  can_auction      ( char_data*, obj_array* );


const char* undo_msg = "A daemon stamps up to you and hands you %s.\n\r
    He mutters something about making up your mind while tapping his forehead\n\r
    and storms off.";

const char* corpse_msg = "An auction daemon runs up to you.  You hand him\n\r
    %s and a silver coin.  He looks at the corpse, looks at you, rips the corpse\n\r
    apart, eats it, smiles happily and disappears with your silver coin.";

const char* no_auction_msg = "An auction daemon runs up to you.  You hand\n\r
    him a silver coin and attempt to hand him %s, but he quickly refuses with\n\r
    some mumble about items banned by the gods.  He then disappears in cloud of\n\r
    smoke.  Only afterwards do you realize your silver coin went with him.";

const char* distinct_msg = "Due to idiosyncrasies of the accountant daemons\n\r
    you may only auction lots of items which contain at most 5 distinct types of\n\r
    items and distinct is unfortunately defined by them and not by what\n\r
    you see.";

  
void do_auction( char_data* ch, char* argument )
{
    char                 arg  [ MAX_INPUT_LENGTH ];
    char                 tmp  [ FOUR_LINES ];
    auction_data*    auction;
    player_data*          pc;
    thing_array*       array;
    obj_data*            obj;
    obj_data*           obj2;
    int                  bid;
    int                 slot;
    int                count; 
    int                flags  = 0;
    int                 j, i;

    if( is_mob( ch ) )
        return;

    pc = player( ch );

    if( has_permission( ch, PERM_PLAYERS ) && !get_flags( ch, argument, &flags, "c", "Auction" ) )
        return;
 
    if( !strcasecmp( argument, "on" ) || !strcasecmp( argument, "off" ) ) {
        send( ch, "See help iflag for turning on and off auction.\n\r" );
        return;
    }

    if( is_set( &flags, 0 ) ) {
        if( auction_list == NULL ) {
            send( ch, "The auction block is empty.\n\r" );
            return;
        }

        sprintf( tmp, "++ Auction Block cleared by %s. ++", ch->real_name( ) );
        info( tmp, LEVEL_BUILDER, tmp, 1, IFLAG_AUCTION, ch );
        send( ch, "-- Auction block cleared --\n\r" );

        for( int i = 0; i < auction_list; i++ ) 
            return_seller( auction_list[i] );
        delete_list( auction_list );

        return;
    }

    if( *argument == '\0' ) {
        display_auction( pc );
        return;
    }

    if( ch->position == POS_MEDITATING ) {
       send( ch, "A mail daemon notices your deep concentration, pushes you over, and runs away laughing hysterically.\n\r" );
       ch->position = POS_RESTING;
       }

    if( ch->position == POS_SLEEPING ) {
      send( ch, "You don't want to invite a mail daemon into your dreams.\n\r" );
      return;
      }

    if( ch->position <= POS_SLEEPING ) {
      send( ch, "You're in no condition to do that." );
      return;
      }

    argument = one_argument( argument, arg );

    if(!strcmp( arg, "undo" ) ) {
        for( i = 0; i < auction_list; i++ ) {
            auction = auction_list[i];
            if( auction->seller == ch->pcdata->pfile && ( *argument == '\0' || atoi( argument ) == auction->slot ) ) {
                if( auction->time < 25 && ( auction->buyer != NULL || auction->deleted ) ) {
                    send( ch, "That item has been on the auction block too long to remove it.\n\r" );
                    return;
	            }
         
                array = &auction->contents;
                fsend( ch, undo_msg, list_name( ch, array, FALSE ) );
                sprintf( tmp, "%s has removed item #%d, %s from the auction block.", ch->real_name( ), auction->slot,
                list_name( ch, array, FALSE ) );
                info( tmp, LEVEL_BUILDER, tmp, IFLAG_AUCTION, 2, ch );

                for( j = 0; j < array->size; j++ ) {
                    obj = (obj_data*) array->list[j];
                    obj = (obj_data*) obj->From( obj->selected );
                    obj->To( ch );
                }
 
                remove( auction_list, i );
                delete auction; 
                return;
            }
        }
        send( ch, "You have no items on the auction block.\n\r" );
        return;
    }

  count = 0;
  for( int i = 0; i < auction_list; i++ ) 
    if( auction_list[i]->seller == ch->pcdata->pfile && ++count == 3 ) {
      send( ch,
        "You may only have three items on the auction block at once.\n\r" );
      return;
      }


    if( ( array = several_things( ch, arg, "auction", &ch->contents ) ) == NULL )
        return;

    if( array->size > 1 ) {
        send( ch, "You may only auction one item at a time.\n\r" ); 
        return; 
    }

    if( *argument == '\0' ) {
        send( ch, "What do you want the minimum bid to be?\n\r" );
        return;
    }

    if( ( bid = atoi( argument ) ) < 1 ) {
        send( ch, "Minimum bid must be at least 1 cp.\n\r" );
        return;
    }

    if( bid > 100000 ) {
        send( ch, "The minimum bid cannot be greater than 100 pp.\n\r" );
        return;
    }

    if( !can_auction( ch, (obj_array*) array ) )
        return;

    if( !remove_silver( ch ) ) {
        send( ch, "To auction you need a silver coin to bribe the delivery daemon.\n\r" );
        return;
    }

  /* FIND FIRST OPEN SLOT */

    slot = 1;
    for( int i = 0; ; ) {
        if( i == auction_list )
            break;
        if( auction_list[i]->slot == slot ) {
            slot++;
            i = 0;
        }
        else
            i++;
    }

  /* AUCTION ITEM */

    fsend( ch, "A mail daemon in disguise runs up to you.  You hand him %s and a silver coin and he sprints off to the auction house.", list_name( ch, array, FALSE ) );

    sprintf( tmp, "%s has placed %s on the auction block.", ch->real_name( ), list_name( ch, array, FALSE ) );
    info( tmp, LEVEL_BUILDER, tmp, IFLAG_AUCTION, 1, ch );

    auction          = new auction_data;
    auction->seller  = ch->pcdata->pfile;
    auction->buyer   = NULL;
    auction->bid     = bid;
    auction->time    = 30;
    auction->slot    = slot;

    for( int i = 0; i < array->size; i++ ) {
        obj = (obj_data*) array->list[i];
        obj = (obj_data*) obj->From( obj->selected );
        obj->To( auction );
    }

    auction_list += auction;
}


/*
 *   CAN AUCTION
 */


bool can_auction( char_data* ch, obj_array* array )
{
    obj_data* obj;

    if( *array > 5 ) {
        fsend( ch, distinct_msg );
        return FALSE;
    }

    for( int i = 0; i < *array; i++ ) {
        obj = object( array->list[i] );
 
        if( no_auction( obj ) ) {
          if( !remove_silver( ch ) ) {
            send( ch, "To auction you need a silver coin to bribe the delivery  daemon.\n\r" );
            return FALSE;
            }
            fsend( ch, no_auction_msg, obj );
            return FALSE;
        }
    
        if( is_set( obj->extra_flags, OFLAG_NODROP ) ) {
            send( ch, "You can't auction items which you can't let go of.\n\r" );
            return FALSE;
        }
    
        if( obj->pIndexData->item_type == ITEM_MONEY ) {
            send( ch, "You can't auction money!\n\r" );
            return FALSE;
        }
     
        if(  obj->pIndexData->item_type == ITEM_CORPSE ) {
            fsend( ch, corpse_msg, obj );
            obj->Extract( obj->selected );
            return FALSE;
        }
    }

    return TRUE;
}


bool no_auction( obj_data* obj )
{
    obj_data* content;

    if( is_set( obj->pIndexData->extra_flags, OFLAG_NO_AUCTION ) ) 
        return TRUE;

    for( int i = 0; i < obj->contents; i++ )
        if( ( content = object( obj->contents[i] ) ) == NULL || no_auction( content ) )
            return TRUE;

    return FALSE;
}


bool stolen_auction( char_data* ch, obj_data* obj )
{
    player_data* player;

    if( obj->Belongs( ch ) ) 
        return FALSE;

    fsend( ch, "A mail daemon in disguise runs up to you.  You hand him\n\r
        %s and a silver coin.  He stops and looks at it closely and then declares\n\r
        it stolen property from %s and disappears with a mutter about returning it\n\r
        to the true owner.", obj, obj->owner->name );

    obj = (obj_data*) obj->From( obj->selected );

    if( ( player = find_player( obj->owner ) ) != NULL ) {
        fsend( player, "A mail daemon in disguise runs up to you and hands you %s\n\r
            He bows deeply and then blinks out of existence.", obj );
        obj->To( player->array );
    }
    else { 
   /* transfer_file( obj->owner, obj, 0 ); */
    }

    return TRUE;
}


/*
 *   DISPLAY
 */


void display_auction( player_data* pc )
{
    char               tmp  [ TWO_LINES ];
    char         condition  [ 50 ];
    char             buyer  [ 20 ];
    char               age  [ 20 ];
    auction_data*  auction;
    obj_data*          obj;
    bool             first;

    if( is_empty( auction_list ) ) {
        send( pc, "There are no items being auctioned.\n\r" );
        return;
    }

    page( pc, "Bank Account: %d cp\n\r\n\r", pc->bank );
    page_centered( pc, "+++ The Auction Block +++" );
    page( pc, "\n\r" );
    sprintf( tmp, "%4s %3s %-42s %4s %4s %s %s %7s\n\r", "Slot", "Tme", "Item", "Buyr", "Use?", "Age", "Cnd", "Min Bid" );
    page_underlined( pc, tmp );
 
    for( int i = 0; i < auction_list; i++ ) {
        auction = auction_list[i];
        rehash( pc, auction->contents );
        first = TRUE;
        for( int j = 0; j < auction->contents; j++ ) {
            obj = object( auction->contents[j] );
            if( obj->shown > 0 ) {
                condition_abbrev( condition, obj, pc );
                age_abbrev( age, obj, pc );

                if( first ) {
                    first = FALSE;
                    memcpy( buyer, auction->buyer == NULL ? " -- " : auction->buyer->name, 4 );
                    buyer[4] = '\0';
                    sprintf( tmp, "%-4d %-3d %-42s %4s %4s %s %s %7d\n\r", auction->slot, auction->time, truncate( obj->Seen_Name( NULL, obj->shown, TRUE ), 42 ), buyer,  can_use( pc, obj->pIndexData, obj ) ? "yes" : "no", age, condition, auction->minimum_bid( ) );
                }
                else {
                    sprintf( tmp, "         %-42s      %4s %s %s\n\r",
                    truncate( obj->Seen_Name( NULL, obj->shown, TRUE ), 42 ),
                    can_use( pc, obj->pIndexData, obj ) ? "yes" : "no", age, condition );
                }
                page( pc, tmp );
            }
        }
    }
}


/*
 *   BID
 */


void do_bid( char_data* ch, char* argument )
{

    char               arg  [ MAX_INPUT_LENGTH ];
    char               tmp  [ TWO_LINES ];
    player_data*        pc;
    auction_data*  auction  = NULL;
    int                bid;
    int            min_bid;
    int               slot;
    int              proxy;
    bool             found;
    
    found = FALSE;
    
    if( is_mob( ch ) )
        return;

    pc = player( ch );

    if( *argument == '\0' ) {
        send( "Syntax: Bid <slot> <price>\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );
    slot     = atoi( arg );
   
        
    for( int i = 0; i < auction_list; i++ ) {
        if( ( auction = auction_list[i] )->slot == slot ) {
            found = TRUE;
            break;        
        }
    }
    
    if( found == FALSE ) {
    	send( ch, "There are no items in slot %d on the auction block.\n\r", slot );
        return;
    }
    
    if( auction->seller == ch->pcdata->pfile ) {
        send( ch, "You can't bid on your own item!\n\r" );
        return;
    } 

    argument = one_argument( argument, arg );

    bid     = atoi( arg );
    proxy   = atoi( argument );
    min_bid = auction->minimum_bid( );

    bid = max( bid, min( min_bid, proxy ) );

    if( bid < min_bid ) {
        fsend( ch, "The minimum bid for %s is %d.\n\r", list_name( ch, &auction->contents, TRUE ), min_bid );
        return;
    }

    if( bid > 3*min_bid && bid > 500 ) {
        send( ch, "To protect you from yourself you may not bid more than the greater\n\rof 3 times the current minimum bid and 500 cp.\n\r" );
        return;
    }

    if( max( bid, proxy ) > free_balance( pc, auction ) ) {
        send( ch, "The bid is not accepted due to insufficent funds in your bank account.\n\r" );
        return;
    }

    if( *argument != '\0' && proxy <= bid ) {
        send( ch, "A proxy only makes sense if greater than the bid.\n\r" );
        return;
    }

    if( auction->buyer == ch->pcdata->pfile ) {
        bid = max( bid, proxy );
        if( auction->proxy > 0 ) 
            fsend( ch, "You change the proxy on %s from %d to %d cp.", list_name( ch, &auction->contents, TRUE ), auction->proxy, bid );
        else
            fsend( ch, "You add a proxy on %s of %d cp.", list_name( ch, &auction->contents, TRUE ), bid );
        auction->proxy = bid;
        return;
    }

    bid = max( bid, min( auction->proxy, proxy ) );

    if( bid < auction->proxy  ) {
        sprintf( tmp, "You bid %d cp for %s, but it is immediately matched by %s.\n\r", bid, list_name( ch, &auction->contents, TRUE), auction->buyer->name );
        fsend( ch, tmp );
        sprintf( tmp, "%s bids %d cp on item #%d, %s.", ch->real_name( ), bid, auction->slot, list_name( ch, &auction->contents, TRUE) );
        info( tmp, LEVEL_BUILDER, tmp, IFLAG_AUCTION, 3, ch );
        sprintf( tmp, "The bid is matched by %s.", auction->buyer->name );
        info( tmp, LEVEL_BUILDER, tmp, IFLAG_AUCTION, 3, ch );
        auction->bid = bid;
        add_time( auction );
        return;
    } 

    if( proxy > bid ) 
        fsend( ch, "You bid %d cp for %s and will automatically match bids on it up to %d cp.\n\r", bid, list_name(ch, &auction->contents, TRUE) , proxy );
    else
        send( ch, "You bid %d cp for %s.\n\r", bid, list_name(ch, &auction->contents, TRUE) );

    sprintf( tmp, "%s bids %d cp on item #%d, %s.", ch->real_name( ), bid, auction->slot, list_name(ch, &auction->contents, TRUE) );
    info( tmp, LEVEL_BUILDER, tmp, IFLAG_AUCTION, 3, ch );

    auction->bid     = bid;
    auction->buyer   = ch->pcdata->pfile;
    auction->deleted = FALSE;
    auction->proxy   = proxy;

    add_time( auction );
}


const char* delivery_msg = "A daemon runs up and hands you %s.  He mumbles\n\r
    something about %d cp and raiding your bank account and sprints off.";

const char* return_msg = "A daemon runs up and hands you %s.\n\r
    He then marches off without a word.";

const char* floor_msg = "A daemon runs up to you and attempts to hand %s\n\r
    to you but realizes you are unable to carry %s.  He snickers rudely and drops\n\r
    the delivery on the floor and sprints off.";


void auction_update( )
{
    auction_data*  auction;
    int i;

    for( i=0; i < auction_list; i++ ) {
        auction = auction_list[i];
        if( --auction->time == 0 ) {
            if( auction->buyer == NULL && !auction->deleted ) 
                return_seller( auction );
        else
            transfer_buyer( auction );
        remove( auction_list, i );
        delete auction;
        }
    }
}


int free_balance( player_data* player, auction_data* replace )
{
    int              credit  = player->bank;
    auction_data*   auction;

    for( int i = 0; i < auction_list; i++ ) {
        auction = auction_list[i];
        if( auction->buyer == player->pcdata->pfile && auction != replace ) 
            credit -= max( auction->bid, auction->proxy );
    }

    return credit;
}


void auction_message( char_data* ch )
{
  /*
  char   tmp   [ TWO_LINES ];
  int      i;

  if( ( i = count( auction_list ) ) != 0 ) {
    sprintf( tmp, "There %s %s item%s on the auction block.",
      i == 1 ? "is" : "are", number_word( i ),
      i == 1 ? "" : "s" ); 
    send_centered( ch, tmp );
    }
  */
}


/*
 *   TRANSFERING OF OBJECT/MONEY
 */


void transfer_file( pfile_data *pfile, thing_array *array,int amount )
{
    link_data*       link;
    player_data*   player;
    obj_data*         obj;
    int           connect;
    int                 i;

    for( i = 0; i < player_list; i++ ) {
        player = player_list[i];
        if( player != NULL && player->pcdata->pfile == pfile ) {
            for( int i = 0; i < *array; i++ ) {
                obj = (obj_data*) array->list[i];
                obj = (obj_data*) obj->From( obj->selected );
                set_owner( obj, pfile );
                obj->To( player );
            }
            player->bank   += amount;
            return;
        }
    }

    link = new link_data;

    if( !load_char( link, pfile->name, PLAYER_DIR ) ) {
        bug( "Transfer_File: Non-existent player file." ); 
        for( int i = 0; i < *array; i++ ) {
            obj = (obj_data*) array->list[i];
            if( obj != NULL )
                obj->Extract( );
        }
        return;
    }          

    player           = link->player;
    link->connected  = CON_PLAYING;
    player->bank    += amount;

    if(array != NULL)
    for( int i = 0; i < *array; i++ ) { 
       obj = (obj_data*) array->list[0];
       obj = (obj_data*) obj->From( obj->selected );
       set_owner( obj, pfile );
       obj->To( player );
   } 

    write( player );
    player->Extract( );

    delete link;
}


void return_seller( auction_data* auction )
{
    char             tmp  [ TWO_LINES ];
    player_data*      pc;
    obj_data*        obj;
    thing_array*   array  = &auction->contents;

    if( auction->seller == NULL ) {
        sprintf( tmp, "Lot #%d, %s returned to the estate of a deceased character.", auction->slot, list_name( NULL, &auction->contents ) );
        info( tmp, LEVEL_BUILDER, tmp, 3, IFLAG_AUCTION );
        extract( auction->contents );
        return;
    }

    pc = find_player( auction->seller );
 
    sprintf( tmp, "Lot #%d, %s, returned to %s.", auction->slot, list_name( pc, &auction->contents ),
    auction->seller->name );
    info( tmp, LEVEL_BUILDER, tmp, IFLAG_AUCTION, 3, pc );
 
    if( pc != NULL ) {
        for( int i = 0; i < array->size; i++ ) {
            obj = (obj_data*) array->list[i];
            obj = (obj_data*) obj->From( obj->selected );
            fsend( pc, "A daemon runs up and returns your auction items.  He then marches off without a word." );
            obj->To(pc);
        }
        return;
    }

    transfer_file( auction->seller, array, 0 );
}


void transfer_buyer( Auction_Data* auction ) 
{
    char              tmp  [ TWO_LINES ];
    player_data*   player;
    thing_array*     array = &auction->contents;
    obj_data*      obj;

    if( auction->seller != NULL ) {
        if( ( player = find_player( auction->seller ) ) != NULL ) 
            player->bank += auction->bid;
        else
            transfer_file( auction->seller, NULL, auction->bid ); 
    }

    if( auction->buyer == NULL ) {
        sprintf( tmp, "Item #%d, %s, sold to the estate of a deceased character.", auction->slot, list_name( player, array, TRUE ) );
        info( tmp, LEVEL_BUILDER, tmp, 2, IFLAG_AUCTION );
        return;
    }

    player = find_player( auction->buyer );

    sprintf( tmp, "Item #%d, %s, sold to %s for %d cp.", auction->slot, list_name( player, array, TRUE ), auction->buyer->name, auction->bid );
    info( tmp, LEVEL_BUILDER, tmp, IFLAG_AUCTION, 2, player );

    if( player == NULL ) {
        transfer_file( auction->buyer, array, -auction->bid ); 
        return;
    }

    player->bank -= auction->bid;

    for( int i = 0; i < array->size; i++ ) {
        obj = (obj_data*) array->list[i];
        obj = (obj_data*) obj->From( obj->selected );
        set_owner( obj, auction->buyer );

        if( can_carry( player, obj, FALSE ) ) {
            fsend( player, delivery_msg, obj, auction->bid );
            obj->To(player);
        }
        else {
            fsend( player, floor_msg, obj, obj->number > 1 ? "them" : "it" );
            obj->To(player->in_room);
        }
    }
}


void clear_auction( pfile_data* pfile )
{
  /*
    auction_data* auction;

    for( auction = auction_list; auction != NULL; auction = auction->next ) {
        if( auction->seller == pfile )
            auction->seller = NULL;
        if( auction->buyer == pfile ) {
            auction->buyer   = NULL;
            auction->deleted = TRUE;
        }
    }

    return;
  */
}


