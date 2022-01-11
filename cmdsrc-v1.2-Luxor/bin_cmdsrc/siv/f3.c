#include             <stdio.h>
#include             "define.h"
#include             "struct.h"

extern int           col       ;
extern int           row       ;
extern int           tcols     ;
extern int           trows     ;
extern int           wdw_size  ;
extern int           tot_len   ;
extern FLAG          eoln      ;
extern char          prev_cmnd ;
extern char          cmnd      ;
extern char          *row_buf  ;
extern char          *tmp_buf  ;
extern char          *end_wdw  ;
extern char          *space    ;
extern struct row_p  *prev_row ;

/***********************************************************
Edit() reads one character from the standard input and checks
if it's ctrl-U.
***********************************************************/

edit( file )
char * file ;
{

    init() ;
    if( file )
        newread( file ) ;
    while( TRUE ) {
        prev_cmnd = cmnd ;
/*        scr_var() ; */
        if( col == tot_len ) {
            cursor( row, col ) ;
            cmnd = getchar() ;
        }
        else if( ( cmnd = *(row_buf + col)) >= ' ' ) {
            cursor( row, col ) ;
            cmnd = getchar() ;      
        }
        else {
            /* print the ctrl-char in the message-area    */

            clr_msg() ;
            cursor( trows - 1, 0 ) ;
            printf( "CTRL-%c", cmnd + '@' ) ;
            cursor( row, col ) ;
            cmnd = getchar() ;
            cursor( trows - 1, 0 ) ;
            clr_msg() ;
            cursor( row, col ) ;
        }

        switch( cmnd ) {

            case CTRL_U   :
                 ctrl_U() ;
                 break ;

            default    :            /* not ctrl-U          */
                 cmnd_scan( cmnd ) ;
                 break ;

        }
    }
} 

/***********************************************************
Init() : initializes most global variables             
***********************************************************/

static init()
{
    char *calloc() ;
    int i ;
    
    Buf_init() ;
    prev_cmnd = MARK ;
    row_buf = calloc( tcols , 1 ) ;
    tmp_buf = calloc( tcols , 1 ) ;
    end_wdw = calloc( tcols , 1 ) ;
    space   = calloc( tcols , 1 ) ;
    for( i = 0 ; i < tcols ; i++ ) {
        *(end_wdw + i) = '-' ;
        *(space + i) = ' ' ;
    }
    *(end_wdw + tcols - 1 ) = '\0' ;
    *(space + tcols - 1 ) = ' ' ;
    col = row = 0 ;
    wdw_size = trows - 2 ;
    eoln = TRUE ;
    clrscr() ;
    put_msg( wdw_size, end_wdw ) ;

}

/***********************************************************
Cmnd_scan(). Not ready yet...
***********************************************************/

cmnd_scan( c ) 
int c ;
{
    if( printable( c ) ) add_char( c ) ;
    else {
        switch( c ) {                /* ctrl-char.        */

            case DEL    :            /* Delete and        */
            case CTRL_H :            /* Backspace         */
                 delete() ;
                 break ;

            case CTRL_M :            /* Carriage ret      */
            case CTRL_J :            /* & Line Feed       */
                 new_line() ;
                 break ;

            case MARK   :
                 set_mark() ;
                 break ;

            case CTRL_A :            /* Start of line     */
                 ctrl_A() ;
                 break ;

            case CTRL_B :            /* Cursor backwards  */
                 ctrl_B() ;
                 break ;

            case CTRL_D :            /* Right delete      */
                 ctrl_D() ;
                 break ;

            case CTRL_E :
                 ctrl_E() ;
                 break ;

            case CTRL_F :            /* Cursor forward    */
                 ctrl_F() ;
                 break ;

            case CTRL_I :            /* Tab               */
                 tab() ;
                 break ;

            case CTRL_K :            /* Line delete       */
                 ctrl_K() ;
                 break ;

            case CTRL_L :            /* Rewrite window    */
                 ctrl_L() ;
                 break ;

            case CTRL_N :            /* Cursor down       */
                 ctrl_N() ;
                 break ;

            case CTRL_O :            /* Insert blank line */
                 ctrl_O() ;
                 break ;

            case CTRL_P :            /* Cursor up         */
                 ctrl_P() ;
                 break ;

            case CTRL_Q :            /* XON, skip it      */
                 break;

            case CTRL_OE :           /* Quote char        */
                 ctrl_Q() ;
                 break ;

            case CTRL_R :            /* Rewers search     */
                 ctrl_R() ;
                 break ;

            case CTRL_AA :           /* Forward search    */
                 ctrl_S() ;
                 break ;

            case CTRL_T :            /* Transpose chars   */
                 ctrl_T() ;
                 break ;

            case CTRL_V :            /* Next window       */
                 ctrl_V() ;
                 break ;

            case CTRL_W :
                 ctrl_W() ;
                 break ;

            case CTRL_X :
                 ctrl_X() ;
                 break ;

            case CTRL_Y :
                 ctrl_Y() ;
                 break ;

            case CTRL_Z :            /* Scroll up         */
                 ctrl_Z() ;
                 break ;

            case ESC    :            /* Escape / Meta     */
                 escape() ;
                 break ;

            default  :               /* Not implemented   */
                 not_imp() ;
                 break ;
        }
    }
}

