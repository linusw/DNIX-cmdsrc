#include             <stdio.h>
#include             "define.h"

extern int           trows     ;
extern int           tcols     ;
extern int           row       ;
extern int           col       ;
extern char *        row_buf   ;
extern int           tot_len   ;
extern int           eoln      ;
extern struct row_p  *prev_row ;
extern char          *end_wdw  ;
extern int           language  ;
extern int           wdw_size  ;
extern char *        space     ;

static char          *lcur     ;
static char          *clr      ;
static FLAG          f_clr     ;
static char          *homestr  ;
static FLAG          f_home    ;
static char          *poscur   ;
static char          *vbell    ;
static char          *clrend   ;
static FLAG          f_clrend  ;
static char          *ins      ;
static FLAG          f_ins     ;
static FLAG          vt100     ;

/*eject*/
prepterm()
{
    register char *termbp ;
    register      err ;
    char          *tmp ;
    char          *malloc() ;
    char          *getenv() ;
    char          *tgetstr() ;
    char          *tgoto() ;
    char          *skipdig() ;

    /* Check which language                               */
    if( ( tmp = getenv( "LANGUAGE" ) ) == NULL ) {
        language = ENGLISH ;
    }
    else if( strcmp( "swedish", tmp ) == 0 ) {
        language = SWEDISH ;
    }
    else language = ENGLISH ;

    /* Check which terminal-type                          */
    if( (tmp = getenv("TERM")) == NULL )
        return(NOTERM) ;
    if( (lcur = malloc(100)) == NULL ||
        (termbp = malloc(1024)) == NULL ) return(NOMEM) ;    
    if( ( err = tgetent( termbp, tmp ) ) != 1 )
         return( err ? NOFILE : NOENTRY ) ;

    /* Special case for vt100 compatible terminals        */
    if( strcmp( tmp, "vt100" ) == 0   ||
        strcmp( tmp, "x35p" )  == 0   ||
        strcmp( tmp, "x35l" )  == 0   ||
        strcmp( tmp, "dt80" )  == 0     )
        vt100 = TRUE ;
    else
        vt100 = FALSE ;

    if( (tcols = tgetnum("co")) == -1 )  tcols = 80 ;
    if( (trows = tgetnum("li")) == -1 )  trows = 24 ;
    tmp = lcur ;

    /* Get backspace character                                 */
    if( tgetflag("bs") ) {   
        *tmp++ = '\b' ;
        *tmp++ = '\0' ;
    }
    else
        if( tgetstr("bc", &tmp) == NULL )
            return(NOTSUPPORTED) ;

    /* Get clear screen sequence                               */
    if( f_clr = (clr = tgetstr("cl", &tmp)) != NULL )
        clr = skipdig( clr ) ;

    /* Get position cursor sequence                            */
    if( (poscur = tgetstr("cm", &tmp)) != NULL )
        poscur = skipdig( poscur ) ;
    else
        return(NOTSUPPORTED) ;

    /* Get clear screen to end sequence                        */
    if( f_clrend = (clrend = tgetstr("cd", &tmp)) != NULL )
        clrend = skipdig( clrend ) ;

    /* Get home sequence                                       */
    if( f_home = (homestr = tgetstr("ho", &tmp)) != NULL )
        homestr = skipdig( homestr ) ;

    /* Get insert line sequence                                */
    if( f_ins = (ins = tgetstr("al", &tmp)) != NULL )
        ins = skipdig( ins ) ;

    /* Get bell character                                      */
    vbell = tgetstr("vb", &tmp) ;

    free(termbp) ;
    return(OK) ;
}

char * skipdig( p )
register char * p ;
{
    while( *p >= '0' && *p <= '9' )
        ++p ;
    return( p ) ;
}

/*eject*/
/***********************************************************
Home() : Send cursor to home position.
***********************************************************/

home()
{
    register char *p ;

    if( f_home )
        for( p = homestr ; *p ; ++p )
            putchar( *p ) ;
    else
        cursor( 0, 0 );
}

/*eject*/
/***********************************************************
Curleft() : Backsteps the cursor if it's not at the left
margin.
***********************************************************/

curleft()
{
    register char *p ;

    for( p = lcur ; *p ; p++)
        putchar( *p );
}
/*eject*/
/***********************************************************
Clrscr() : Clears the screen and "homes" the cursor.
***********************************************************/

clrscr()
{
    register i;
    register char *p ;

    if( f_clr )
        for( p = clr ; *p ; p++ )
            putchar( *p );
    else {
        cursor( trows - 1, tcols - 1);
        for( i = trows ; i ; --i )
            putchar( '\n' );
    }
    home();
}
/*eject*/
/***********************************************************
Cclrrest( r, c ) :
Clearrest() :
Clrr( r, c ) : Clears the screen after the cursor.
***********************************************************/

clearrest()
{
        clrr( row, col ) ;
}

cclrrest( r, c )
int r, c ;
{
        cursor( r, c );
        clrr( r, c );
}

clrr( r, c )
register r, c ;
{
    register i;
    register char *p ;

    if( f_clrend )
        for( p = clrend ; *p ; ++p )
            putchar( *p );
    else {
        /* the last col on the last row are not cleared, to avoid scroll */
        for( i = tcols - c - 1 + (trows - r - 1) * tcols - 1 ; i ; --i )
            putchar( ' ' );
    }
    cursor( r, c );
}
/*eject*/
/***********************************************************
Cursor() : Positions the cursor somewhere on the screen i.e.
0--(trows-1) and 0--(tcols-1) .
***********************************************************/

cursor(r, c)
int r ;
int c ;
{
    register char *p;
    char *tgoto();

    for( p = skipdig(tgoto(poscur, c, r)) ; *p ; ++p )
        putchar( *p );
}
/*eject*/
/***********************************************************
Blankrow() : Scrolls the screen after the cursor one row
down, i.e. inserts a row before the row that holds the
cursor. NOTE: The cursor should be in position "col=0".
***********************************************************/

blankrow( n )
int n ;
{
    register i, j ;
    register char *p ;
    register struct row_p * rp ;

    cursor( n, 0 ) ;
    if( vt100 ) {
        printf("\033[%d;%dr",n + 1, trows ) ;
        printf("\033[%dH\033M\033[r", n + 1 ) ;
    }
    else {
        if ( f_ins )
            for( p = ins ; *p ; p++ )
                putchar( *p ) ;
        else {
            i = n - row ;
            rp = prev_row->next ;
            while( i && rp )
                if( i > 0 ) {
                    rp = rp->next ;
                    --i ;
                } else {
                    rp = rp->previous ;
                    ++i ;
                }
            printf( space ) ;           /* clear */
            for( i = wdw_size - n ; --i > 0 && rp ; rp = rp->next ) {
                pr_row( rp->row, 0, rp->length ) ;
                if( rp->eoln )
                    for( j = tcols - rp->length ; j-- ; )
                        putchar( ' ' ) ;
                else
                    putchar( '!' ) ;
            }
        }
    }
}
/*eject*/
/***********************************************************
Beep() : Makes a beep.....
***********************************************************/

beep()
{
        register char *p ;

        if( vbell != NULL )
            for( p = vbell ; *p ; --p )
                putchar( *p );
}
/*eject*/
/***********************************************************
Delchar() : Deletes the character before the cursor.
***********************************************************/

delchar()
{
        curleft();
        putchar(' ');
        curleft();
}
/*eject*/
/***********************************************************
scrl() : Scrolls the whole screen one step up. NOTE: The
cursor must then be repositioned.
***********************************************************/

scrl()
{
    cursor( trows - 1, tcols - 1 ) ;
    putchar('\n') ;
}

/*eject*/        
/***********************************************************
Scrl_1up() : scrolls the window one step up and leaves an
empty row at the bottom.
***********************************************************/

scrl_1up()
{
    cclrrest( wdw_size, 0 ) ;
    scrl();
    row-- ;
    put_msg( wdw_size, end_wdw ) ;
}
/*eject*/

scrl_up( n )
int n ;
{
    struct row_p *rp ;

    cclrrest( wdw_size, 0 ) ;
    rp = prev_row->next ;
    while ( n > 0 ) {
        scrl() ;
        if( rp != NULL ) {
            cursor( trows - 3, 0 ) ;
            pr_row( rp->row, 0, rp->length ) ;
            if( rp->eoln ) putchar('\n') ;
            else putchar('!') ;
            rp = rp->next ;
        }
        n-- ;
        row-- ; 
    }
    put_msg( wdw_size, end_wdw ) ;
}
/*eject*/
/***********************************************************
Ins_blank() : inserts a row n rows after the one that held
the cursor. If row + n is outside the window a scroll up is 
performed. However, nothing is done if this means that row 
"falls off" the window ( i.e. becomes < 0 ).
***********************************************************/

ins_blank( n )
int n ;
{
    if( (row + n) > (trows - 3) ) {
        
        /* The cursor is outside the window, scroll up!!  */
        scrl_up( trows / 2 ) ;
    }
    /* scroll down the rest of the screen one row        */
    /* I.e. insert a blank row.                          */
    blankrow( row + n ) ;
    put_msg( wdw_size, end_wdw ) ;
    printf( space ) ;
}
/*eject*/
/***********************************************************
Scrl_dwn() : scrolls the screen max n rows down.
***********************************************************/

scrl_dwn( n )
register n ;
{
    register FLAG endofln ;
    register struct row_p  * rp ;

    rp = prev_row ;
    if( vt100 || f_ins )
        if( ( n > 0 ) && ( rp->previous != NULL ) ) {

            /* Scroll another row                            */
            row++ ;
            prev_row = prev_row->previous ;

            /* Make space for a row                          */
            blankrow( 0 ) ;
            scrl_dwn( n - 1 ) ;

            /* Print the row                                 */
            pr_row( rp->row, 0, rp->length ) ;
            if( rp->eoln )
                putchar('\n') ;
            else
                putchar('!') ;
            prev_row = rp ;
        } else {
            put_msg( wdw_size, end_wdw ) ;
            cclrrest( tcols -1, 0 ) ;
            home() ;
        }
    else {      /* scroll down on terminal without down-scroll */
            while( n-- && rp->previous->previous ) {
                rp = rp->previous ;
                row++ ;
            }
            row++ ;
            clrscr() ;
            for( n = 0 ; n < wdw_size && rp ; ++n ) {
                if( n == row ) {
                    pr_row( row_buf, 0, tot_len );
                    endofln = eoln ;
                } else {
                    pr_row( rp->row, 0, rp->length ) ;
                    endofln = rp->eoln ;
                    rp = rp->next ;
                }
                if( endofln )
                    putchar( '\n' ) ;
                else
                    putchar( '!' ) ;
            }
            put_msg( wdw_size, end_wdw ) ;
            cclrrest( trows - 1, 0 );
            home();
    }
}
