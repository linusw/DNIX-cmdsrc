/***********************************************************
This file contains the META- or ESCAPE-commands, i.e. 
commands created by pressing the esc-key and thereafter
another key. It also contains the search-commands, i.e.
CTRL_S and _R which are related to the query replace.
***********************************************************/

#include             <stdio.h>
#include             "define.h"
#include             "struct.h"

extern int           col       ;
extern int           row       ;
extern int           tcols     ;
extern int           wdw_size  ;
extern int           trows     ;
extern int           tot_len   ;
extern int           eoln      ;
extern long          mark      ;
extern char          *row_buf  ;
extern char          *end_wdw  ;
extern char          f_name[]  ;
extern char          search[]  ;
extern char          replce[]  ; 
extern struct row_p  *prev_row ;
extern struct row_p  first_r   ;

/***********************************************************
Escape() : Just reads a character and calls the routines
that perform the command.
***********************************************************/

escape()
{
    char c ;

    switch( c = upper( getchar() ) ) {

        case TOP_F :             /* To top of file        */
             top_file() ;
             break ;

        case END_F :             /* To end of file        */
             end_file() ;
             break ;

        case BCK_W :             /* Previous screen       */
             bck_wdw() ;
             break ;

        case QUERY :             /* Query replace         */
             q_rplce() ;
             break ;

        case '[' :               /* vt100 cursor arrwos   */
             vt100_cur() ;
             break ;

        default    :
             not_imp() ;
             break ;

    }
}

upper( c )
char c ;
{
    return( c >= 'a' AND c <= 'z' ? c - 'a' + 'A' : c ) ;
}

/***********************************************************
Vt100_cur() : Case out vt100 cursor arrows.
***********************************************************/

vt100_cur()
{
        char c ;

        switch( c = upper( getchar() ) ) {

            case 'A' :          /* arrow up             */
                ctrl_P() ;
                break ;

            case 'B' :          /* arrow down           */
                ctrl_N() ;
                break ;

            case 'C' :          /* arrow right          */
                ctrl_F() ;
                break ;

            case 'D' :          /* arrow left           */
                ctrl_B() ;
                break ;

            default :
                break ;
        }
}

/***********************************************************
Top_file() : moves the cursor to the first row in the 
buffer.
***********************************************************/

top_file()
{
    struct row_p *ins_text() ;
    char *get_row() ;
    
    col = 0 ;
    row = 0 ;

    /* Check if already at top row                        */
    if( prev_row->previous == NULL ) return( TRUE ) ;
    
    /* Save the current row                               */
    insert( row_buf, prev_row, tot_len, eoln ) ;

    /* Set the first row to current                       */
    prev_row = &first_r ;
    tot_len = prev_row->next->length ;
    eoln = prev_row->next->eoln ;
    cpystr( get_row( prev_row->next ) ,
            tot_len ,
            row_buf ) ;

    /* Rewrite the screen                                 */
    ctrl_L() ;
}

/***********************************************************
Bck_wdw() : moves the cursor to the previous window unless
it was at the top of the file.
***********************************************************/

bck_wdw()
{
    struct row_p *rp ;
    char *get_row() ;
    int i ;

    /* At the top of the file ???                        */
    if( firstrow() ) {
        pr_msg( TOP_FL, NULL ) ;
        return ;
    }

    /* Save current row                                  */
    insert( row_buf, prev_row, tot_len, eoln ) ;

    /* Find the new current row                          */
    for( i = row + trows - 4 ; i > 0 ; i-- ) {

        /* Don't go beyond the top of the file           */
        if( firstrow() ) break ;
        prev_row = prev_row->previous ;
    }
    tot_len = prev_row->next->length ;
    eoln = prev_row->next->eoln ;
    cpystr( get_row( prev_row->next ) ,
            tot_len ,
            row_buf ) ;
    rewrite() ;
    col = 0 ;
}

/***********************************************************
End_file() : moves the cursor to the last row of the file.
***********************************************************/

end_file()
{
    struct row_p *ins_text(), *rp ;
    char *get_row() ;

    /* Already at last row ???                            */
    if( last_row() ) {
        col = tot_len ;
        return ;
    }
    
    /* Save current row                                   */
    insert( row_buf, prev_row, tot_len, eoln ) ;
    prev_row = prev_row->next ;

    /* Find the last row                                  */
    while( NOT last_row() ) {
        prev_row = prev_row->next ;
        row++ ;
    }
    tot_len = col = prev_row->length ;
    eoln = prev_row->eoln ;
    prev_row = prev_row->previous ;
    cpystr( get_row( prev_row->next ) ,
            tot_len ,
            row_buf ) ;

    /* The last row was on a later window ???             */
    if( row >= wdw_size ) rewrite() ;
}


/***********************************************************
Ctrl_S() : does a forward search, i.e. tries to find the
the string stored in search[].
***********************************************************/

ctrl_S()
{
    FLAG str_legal() ;
    char str[MAXLEN], chr ;

    /* Input the string to search for                     */
    clr_msg() ;
    pr_msg( SEARCH, NULL ) ;

    /* Two consecutive ctrl-s's searches the same string  */
    /* as last time                                       */
    if( ( chr = getchar() ) != CTRL_AA ) {
        if( input( chr, str, str_legal ) == ABORTED ) {
            clr_msg() ;
            pr_msg( ABORTD, NULL ) ;
            return ;
        }
        strcpy( search, str ) ;
    }
    else printf("%s", search ) ;

    if( FAILED f_search() ) {

        /* String was not found                               */
        clr_msg() ;
        pr_msg( NO_FND, NULL ) ;
    }
}

/***********************************************************
F_search() : searches in the textbuffer for a sequence of
characters equal to the one in the string search[]. If an
occurrence is found, that position becomes the current
position.
***********************************************************/

f_search()
{
    struct row_p *ins_text(), *rp ;
    int tmp_row, n ;
    
    insert( row_buf, prev_row, tot_len, eoln ) ;
    rp = prev_row ;
    tmp_row = row ;
    prev_row = prev_row->next ;
    n = col + 1 ;

    /* Search until found or no more rows to search       */
    while( prev_row != NULL ) {
        if( n >= tot_len ) {

            /* Try in next row                            */
            prev_row = prev_row->next ;
            if( prev_row == NULL ) break ;  /* NOT FOUND! */
            tot_len = prev_row->length ;
            n = 0 ;
            row++ ;
        }
        else {
            if( compare( prev_row, tot_len, n ) == EQUAL ) {

                /* FOUND!!!                               */
                tot_len  = prev_row->length ;
                eoln     = prev_row->eoln ;
                prev_row = prev_row->previous ;

                /* Reprint window if necessary            */
                reprint() ;
                cpystr( get_row( prev_row->next ) ,
                        tot_len ,
                        row_buf ) ;
                col = n ;
                return( TRUE ) ;
            }

            /* Step one character                         */
            else n++ ;
        }
    }

    /* String was not found                               */
    prev_row = rp ;
    row = tmp_row ;
    tot_len = rp->next->length ;
    eoln    = rp->next->eoln ;
    cpystr( get_row( rp->next ) ,
            tot_len ,
            row_buf ) ;
    return( FALSE ) ;
}

/***********************************************************
Compare() : compares 'search[] ' with the sequence of
characters starting at rp->row + n . Len is used to see if 
the string continues in the next row.
***********************************************************/

compare( rp, len, n )
struct row_p *rp ;
int len, n ;
{
    int i ;

    i = 0 ;
    while( TRUE ) {

        /* A correct sequence hsa been found             */
        if( *(search + i) == '\0' ) return( EQUAL ) ;
        if( n == len ) {

            /* At the end of a row or line               */
            /* Not found if no more rows                 */
            if( rp->next == NULL ) return( NOTEQ ) ;

            /* Not found if end of line                  */
            if( rp->eoln ) return( NOTEQ ) ;

            /* Continue in next row                      */
            rp = rp->next ;
            n = 0 ;
        }

        /* Compare characters                            */
        if( *(search + i) != *(rp->row + n) ) return( NOTEQ ) ;

        /* Equal so far. Compare next                    */
        i++ ;
        n++ ;
    }
}

/***********************************************************
Reprint() : reprints the screen if nessesary, i.e. if the
next occurence is outside the current window.
***********************************************************/

reprint()
{
    struct row_p *rp ;
    int i ;

    /* Check if outside the window                        */
    if( this_wdw() ) return ;
    clrscr() ;
    row = 0 ;
    rp = prev_row ;
    while( rp->previous != NULL ) {

        /* Look for the "home"-row                        */
        if( row == trows / 2 ) break ;
        row++ ;
        rp = rp->previous ;
    }
    rp = rp->next ;
    i = 0 ;

    /* Reprint, starting at the "home"-row                */
    while( rp != NULL ) {
        if( i == wdw_size ) break ;
        pr_row( rp->row, 0, rp->length ) ;
        if( rp->eoln ) putchar('\n') ;
        else putchar('!') ;
        rp = rp->next ;
        i++ ;
    }
    put_msg( wdw_size, end_wdw ) ;
}

/***********************************************************
This_wdw() : returns TRUE if 'row' is within the window. I.e.
0 <= row < wdw_size .
***********************************************************/

this_wdw()
{
    return( row < wdw_size AND row >= 0 ? TRUE : FALSE ) ;
}

/***********************************************************
Input() : inputs a string to search for or a filename. If it
is a filename, file_legal() is called, which diables spaces.
***********************************************************/

input( chr, str, legal )
char chr, *str ;
FLAG (*legal) () ;
{
    int i ;

    i = 0 ;
    if( chr == CTRL_G OR chr == '\n' ) return( ABORTED ) ;
    if( (*legal)(chr) ) {
        *str = chr ;
        putchar( chr ) ;
        i++ ;
    }
    while( ( chr = getchar() ) != '\n' ) {
        if( chr == CTRL_G ) return( ABORTED ) ;
        if( ( chr == DEL OR chr == CTRL_H ) AND i > 0 ) {
            i-- ;
            delchar() ;
        }
        else if( i < MAXLEN - 1 ) {
            if( (*legal)(chr) ) {
                *(str + i) = chr ;
                i++ ;
                putchar( chr ) ;
            }
        }
    }
    *(str + i) = '\0' ;
    return( CONT ) ;
}

/***********************************************************
File_legal() : checks that chr is a legal character in a
filename. I.e.  ascii 32 < chr < ascii 127.
***********************************************************/

file_legal( chr )
char chr ;
{
    if( chr == ' ' ) return( FALSE ) ;
    else return( printable( chr ) ) ;
}

/***********************************************************
Str_legal() : checks that chr is a legal character to search
for. I.e. ascii 31 < chr < ascii 127.
***********************************************************/

str_legal( chr ) 
char chr ;
{
    return( printable( chr ) ) ;
}

/***********************************************************
Ctrl_R() : makes a reverse search and is very similar to 
ctrl_S.
***********************************************************/

ctrl_R()
{
    int n, tmp_row ;
    FLAG str_legal() ;
    char str[MAXLEN], chr ;
    struct row_p *ins_text(), *rp ;

    /* Input the string to search for                     */
    clr_msg() ;
    pr_msg( SEARCH, NULL ) ;

    /* Two consecutive ctrl-r's searches the same string  */
    /* as last time                                       */
    if( ( chr = getchar() ) != CTRL_R ) {
        if( input( chr, str, str_legal ) == ABORTED ) {
            clr_msg() ;
            pr_msg( ABORTD, NULL ) ;
            return ;
        }
        strcpy( search, str ) ;
    }
    else printf("%s", search ) ;

    /* Copy current row to textbuffer and save some vars  */
    insert( row_buf, prev_row, tot_len, eoln ) ;
    rp = prev_row ;
    tmp_row = row ;
    prev_row = prev_row->next ;
    n = col - 1 ;

    /* Search until found or no more rows to search       */
    while( prev_row->previous != NULL ) {
        if( n < 0 ) {

            /* Try in previous row                            */
            prev_row = prev_row->previous ;
            tot_len = prev_row->length ;
            n = tot_len - 1 ;
            row-- ;
        }
        else {
            if( compare( prev_row, tot_len, n ) == EQUAL ) {

                /* FOUND!!!                               */
                tot_len  = prev_row->length ;
                eoln     = prev_row->eoln ;
                prev_row = prev_row->previous ;

                /* Reprint window if necessary            */
                reprint() ;
                cpystr( get_row( prev_row->next ) ,
                        tot_len ,
                        row_buf ) ;
                col = n ;
                return ;
            }

            /* Step one character                         */
            else n-- ;
        }
    }

    /* String was not found                               */
    clr_msg() ;
    pr_msg( NO_FND, NULL ) ;
    prev_row = rp ;
    row = tmp_row ;
    tot_len = rp->next->length ;
    eoln    = rp->next->eoln ;
    cpystr( get_row( rp->next ) ,
            tot_len ,
            row_buf ) ;
}

/***********************************************************
Q_rplce() : searches for the occurrence of a string. When it
has been found, the user is asked if he wants to replace it.
Hitting the space-bar replaces the string, hitting  'N' does
not change the string but searches for the  next occurrence.
Finally Ctrl-G aborts the query replace.
***********************************************************/


q_rplce()
{
    char buf[MAXLEN], *int_to_str() ;
    struct row_p *ins_text(), *tmp_rp ;
    FLAG str_legal() ;
    int  n, i, tmp_row, tmp_col ;

    clr_msg() ;
    pr_msg( OLD_ST, NULL ) ;
    if( ( input( getchar(), buf, str_legal ) ) == ABORTED ) {
        clr_msg() ;
        pr_msg( ABORTD, NULL ) ;
        return ;
    }
    strcpy( search, buf ) ;
    clr_msg() ;
    pr_msg( NEW_ST, NULL ) ;
    if( ( input( getchar(), buf, str_legal ) ) == ABORTED ) {
        clr_msg() ;
        pr_msg( ABORTD, NULL ) ;
        return ;
    }
    strcpy( replce, buf ) ;
    tmp_rp  = prev_row ;
    tmp_col = col ;
    tmp_row = row ;
    i = 0 ;
    while( TRUE ) {

        /* Any more occurrences???                        */
        if( FAILED f_search() ) break ;
        n = 0 ;
again:  cursor( row, col ) ;
        switch( getchar() ) {

            case ' '    :
                 while( *(search + n) != '\0' ) {
                     cursor( row, col ) ;
                     ctrl_D() ;
                     n++ ;
                 }
                 i++ ;
                 n = 0 ;
                 while( *(replce + n) != '\0' ) {
                     cursor( row, col ) ;
                     add_char( *(replce + n ) ) ;
                     n++ ;
                 }
                 col-- ;
                 break ;

            case CTRL_G :
                 goto exit ;

            case 'n'    :
            case 'N'    :
                 break ;

            default     :
                 clr_msg() ;
                 cursor( trows - 1, 0 ) ;
                 pr_msg( Q_HELP, NULL ) ;
                 goto again ;
        }
    }
exit:
    insert( row_buf, prev_row, tot_len, eoln ) ;
    prev_row = tmp_rp  ;
    row      = tmp_row ;
    col      = tmp_col ;
    tot_len  = prev_row->next->length ;
    eoln     = prev_row->next->eoln ;
    cpystr( get_row( prev_row->next ) ,
            tot_len ,
            row_buf ) ;
    ctrl_L() ;
    pr_msg( REPLCE, int_to_str( i, 5, buf ) ) ;
}

char *int_to_str( i, n, str )
int i, n ;
char *str ;
{
    int tmp, j ;

    str  = str + MAXLEN - 1 ;
    *str = '\0' ;
    if( i == 0 ) {
        --str ;
        *str = '0' ;
    }
    else {
        for( tmp = 0 ; tmp < n ; tmp++ ) {
            if( i == 0 ) break ;
            j = i % 10 ;
            i = i / 10 ;
            --str ;
            *str = '0' + j ;
        }
    }
    return( str ) ;
}




        
    
    

