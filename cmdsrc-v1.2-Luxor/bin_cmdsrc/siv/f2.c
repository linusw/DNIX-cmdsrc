#include             <signal.h>
#include             <stdio.h>
#include             <sgtty.h>
#include             "define.h"
#include             "struct.h"
#include             "term.c"



extern int           tcols    ;
extern int           trows    ;
extern int           col      ;
extern int           row      ;
extern int           wdw_size ;
extern struct row_p  first_r  ;
extern struct row_p  kill_buf ;
extern struct free_p first_f  ;



/***********************************************************
Initterm() checks if the terminal type in TERM has an entry
in the termcap file and sets the program to support that
terminal. The program exits if error occures.
***********************************************************/

initterm()
{
    int n ;
    
    switch(n = prepterm()) {

        case OK: set_term() ;
                 return ;

        default:  error(n) ;
                  exit(1) ;
    }
}



static char *err_msg[] = {
    " " ,
    "out of memory" ,
    "TERM not set" ,
    "can't find termcap file" ,
    "no entry in termcap file" ,
    "terminaltype is not supported"
} ;

static error(n)
{
    outstr("terminal error: ") ;
    outstr(err_msg[n]) ;
    outstr("\n\nexiting\n") ;
}

/***********************************************************
Set_term() calls set_mode() which sets the terminal in
CBREAK mode and turns ECHO off. The call to set_break()
changes the interrupt character to ctrl-C.
***********************************************************/

static int brk ;         /* Tmp for old interrupt char.   */
static int q ;           /* Tmp for old quit char.        */
static int start_o ;     /* Tmp for old start_output char */
static int stop_o ;      /* Tmp for old stop_output char. */

set_term()
{
    set_mode() ;
    set_brk() ;
}

static set_mode() 
{
    static struct sgttyb terms ;

    ioctl(0,TIOCGETP,&terms) ;
    terms.sg_flags ^= 10 ;      /* invert CBREAK & ECHO   */
    ioctl(0,TIOCSETP,&terms) ;
}

static set_brk() 
{
    static struct tchars tmp ;

    ioctl(0,TIOCGETC,&tmp) ;
    brk = tmp.t_intrc ;        /* save old values         */
    q = tmp.t_quitc ;
    start_o = tmp.t_startc ;
    stop_o = tmp.t_stopc ;
    tmp.t_intrc = CTRL_C ;     /* intrpt char = ctrl-C    */
    tmp.t_quitc = tmp.t_startc = tmp.t_stopc = -1 ;
    ioctl(0,TIOCSETC,&tmp) ;
}

/***********************************************************
Reset_term() calls reset_mode() which quits CBREAK mode and
turn echo on again. Thereafter reset_brk() restores the old
interrupt character.
***********************************************************/

reset_term() 
{
    reset_mode() ;
    reset_brk() ;
}

static reset_mode() 
{
    set_mode() ;
}

static reset_brk() 
{
    static struct tchars tmp ;

    ioctl(0,TIOCGETC,&tmp) ;
    tmp.t_intrc = brk ;         /* reset values          */
    tmp.t_quitc = q ;
    tmp.t_startc = start_o ;
    tmp.t_stopc = stop_o ;
    ioctl(0,TIOCSETC,&tmp) ;
    
}

/***********************************************************
Set_trap() sets trap() to be executed whenever the interrupt
character (ctrl-C) is detected .
***********************************************************/

set_trap() 
{
    int quit() ;

    signal( SIGINT, quit ) ;
}


quit() 
{
    static FLAG here = FALSE ;

    set_trap() ;
    if ( here )
        return ;
    here = TRUE ;
    clr_msg() ;
    pr_msg( XIT_ED, NULL ) ;
    loop :
    switch( getchar() ) {

        case 'n' :
        case 'N' : here = FALSE ;
                   return ;

        case 'y' :
        case 'Y' : if( language == SWEDISH ) goto loop ;
                   outstr( "\nBye Bye!!!\n" ) ;
                   reset_term() ;
                   exit(0) ;
                   
        case 'j' :
        case 'J' : if( language == ENGLISH ) goto loop ;
                   outstr( "\nBye Bye!!!\n" ) ;
                   reset_term() ;
                   exit(0) ; 

         default : goto loop ;

    }
}


