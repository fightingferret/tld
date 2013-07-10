#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include "define.h"
#include "struct.h"

void do_watch( char_data* ch, room_data* room )
{
    char_data*      victim;
    link_data*        link;
    player_data*        pc;
    species_data*  species;
    
    if( is_mob( ch ) )
        return;

    pc = player( ch );
        
    if( room == NULL ) {
        code_bug( "WATCH: Non-existent room." );
        return;
    }
        
    if( pc->switched != NULL ) {
        send( ch, "You cannot watch the arena if you are switched!\n\r" );
        return;
    }
    if( (species = get_species( 2397 ) ) == NULL ) {
        code_bug( "WATCH: Load species failed, non existant. 2397" );
        return;
    }
    
    victim = create_mobile( species );
    victim->reset  = NULL;
   
    if( victim->pcdata != NULL ) {
        code_bug( "WATCH: Watch victim is player?!" );
        return;
    }

    if( victim->link != NULL ) {
        code_bug( "WATCH: Watch target is in use?!" );
        return;
    }

    victim->To( room );
    link = ch->link;

    link->character  = victim;
    pc->switched     = victim;
    pc->link         = NULL;
    victim->link     = link;
    victim->pcdata   = pc->pcdata;
    victim->timer    = current_time;

    send( victim, "You begin watching the fight.\n\r" );

    return;
}
