/***********************************************************
This file contains routines to delete characters. I have
tried to be as clear as possible, but they can probably cause
severe braindamage even to people with nerves of steel. I
think the best way to understand them is to read the code
and execute it simultaneously. DON'T TRY TO UNDERSTAND IT
IF YOU HAVEN'T READ THE NOTES IN THE FILE <global.h>!!!!!
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
extern char          *row_buf  ;
extern char          *end_wdw  ;
extern char          *space    ;
extern char          prev_cmnd ;
extern struct row_p  *prev_row ;
extern struct row_p  kill_buf  ;


/***********************************************************
Delete() : deletes the character to the left of the cursor 
and shifts the rightmost characters one step left. If the 
cursor is at the left margin and the previous row has an 
eoln, the lines are appended. 
***********************************************************/

delete()
{
    char *get_row() ;
    int i ;
    
    if( col == 0 ) {

        /* At the start of the file ???                   */
        if( firstrow() ) return( FALSE ) ;
        if( row == 0 ) scrl_dwn( trows/2 ) ;
        if( prev_row->length + prev_row->eoln == tcols ){

            /* Only a eoln left ???                       */
            if( tot_len == 0 ) del_row() ;
            else {

                /* Set eoln to FALSE for previous row     */
                cursor( row - 1, tcols - 1 ) ;
                putchar('!') ;
                prev_row->eoln = FALSE ;
            }
        }
        else {

            /* Delete the eoln in the previous row and    */
            /* append the current line to it. Finally     */
            /* make the previous row current.             */
            row-- ;
            insert( row_buf, prev_row, tot_len, eoln ) ;
            get_prev() ;
            col      = ( eoln ? tot_len : tot_len - 1 ) ;
            cursor( row, 0 ) ;
            shift( prev_row, tcols - 1 - col, row ) ;
            get_current() ;
        }
    }
    else if( eoln ) {
        if( col == tot_len ) {

            /* Only one character ???                     */
            if( ( col == 1 )        AND
                ( NOT firstrow() )  AND
                ( NOT prev_row->eoln ) ) del_row() ;
            else {

                /* The last character in a LINE           */
                tot_len-- ;
                col-- ;
                delchar() ;
            }
        }
        else {

            /* Delete a character IN a row. Shift the     */
            /* rightmost characters one step left.        */
            delchar() ;
            col-- ;
            tot_len-- ;
            for( i = col ; i < tot_len ; i++ ) {
                *(row_buf + i) = *(row_buf + i + 1) ;
            }
            pr_row( row_buf, col, tot_len ) ;
            putchar(' ') ;
        }
    }
    else {

        /* Delete character IN a row. Shift all chars     */
        /* between the cursor and next newline one step   */
        /* left.                                          */
        --col ;
        for( i = col ; i < tcols - 1 ; i++ ) {
            *(row_buf + i) = *(row_buf + i + 1) ;
        }
        cursor( row, 0 ) ;
        shift( prev_row, 1, row ) ;
        get_current() ;
    }
}

/***********************************************************
Ctrl_D() : deletes the character at the cursor and shifts
the rightmost characters one step left. If the character
is a newline the next line is appended.
***********************************************************/

ctrl_D()
{
    int i ;

    /* Is the character a newline ???                     */
    if( col == tot_len ) {

        /* Do nothing if last row                         */
        if( last_row() ) return( FALSE ) ;

        /* At the right margin ???                        */
        if( tot_len == tcols - 1 ) {
            putchar('!') ;
            row++ ; col = 0 ;
            if( row >= wdw_size ) {
                scrl_up( trows/2 ) ;
            }
            insert( row_buf, prev_row, tot_len, FALSE ) ;
            get_nxt() ;
        }
        else {

            /* Append the next line                       */
            cursor( row, 0 ) ;
            shift( prev_row, tcols - 1 - tot_len, row ) ;
            get_current() ;
        }
    }
    else if( eoln ) {

        /* The character is not a newline. Does the       */
        /* previous row continue with one character in    */
        /* the current row ???                            */
        if( ( tot_len == 1 )   AND
            ( NOT firstrow() ) AND
            ( NOT prev_row->eoln ) ) del_row() ;
        else {

            /* Delete the char and shift the leftmost     */
            tot_len-- ;
            for( i = col ; i < tot_len ; i++ ) {
                *(row_buf + i) = *(row_buf + i + 1) ;
            }
            pr_row( row_buf, col, tot_len ) ;
            putchar(' ') ;
        }
    }
    else {

        /* Delete the char and shift the leftmost         */
        for( i = col ; i < tot_len ; i++ ) {
            *(row_buf + i) = *(row_buf + i + 1) ;
        }
        cursor( row, 0 ) ;
        shift( prev_row, 1, row ) ;
        get_current() ;
    }
}


/***********************************************************
Ctrl_K() : Works a little different depending on the situa-
tion. If the cursor is at the end of a line i.e. at the new-
line, the newline is deleted and the next line is appended
to the current row. However, if the cursor is not at the end
of the line, the rest of the characters in the line are del-
eted, leaving only the newline at the cursor-position. All
killed rows/lines are unlinked from the first_r-list and put
at the end of the kill-buffer.
***********************************************************/

ctrl_K()
{
    /* Empty the kill_buf if previous cmnd wasn't CTRL_K  */
    if( prev_cmnd != CTRL_K ) scratch( &kill_buf ) ;
    if( eoln ) {

        /* Only one row of the line after the cursor      */
        if( col == tot_len ) {

            /* Eoln at the cursor position                */
            if( last_row() ) return( FALSE ) ;

            /* Move newline to kill-buffer                */
            insert( row_buf, prev_row, 0, TRUE ) ;
            to_killbuf( prev_row ) ;

            /* Append next line                           */
            cursor( row, 0 ) ;
            shift( prev_row, tcols - 1 - col, row ) ;
            get_current() ;
            if( col == tcols - 1 ) {

                /* The next row becomes current           */
                if( row >= trows - 3 ) scrl_up( trows/2 ) ;
                row++ ;
                col = 0 ;
                insert( row_buf, prev_row, tot_len, eoln ) ;
                get_nxt() ;
                return( TRUE ) ;
            }
        }
        else {

            /* Delete rest of line/row                     */
            insert( row_buf + col, prev_row, tot_len - col, FALSE ) ;
            to_killbuf( prev_row ) ;
            pr_row( space, 0, tot_len + 1 - col ) ;
            tot_len = col ;
        }
    }
    else {

        /* The line contains several rows. Delete the     */
        /* rest of the line.                              */
        insert( row_buf + col, prev_row, tot_len - col, FALSE ) ;
        to_killbuf( prev_row ) ;
        pr_row( space, 0, tot_len + 1 - col ) ;
        tot_len = col ;
        eoln = TRUE ;
        while( prev_row->next != NULL ) {

            /* Move rest of line to kill-buffer           */
            if( prev_row->next->eoln ) {
                prev_row->next->eoln = FALSE ;
                to_killbuf( prev_row ) ;
                break ;
            }
            to_killbuf( prev_row ) ;
        }
        pr_rest( prev_row->next, row + 1 ) ;
    }
    if( ( col == 0 )       AND
        ( NOT firstrow() ) AND
        ( NOT prev_row->eoln ) ) {

            /* The previous row continued in the current. */
            /* Make the previous row current, set eoln =  */
            /* TRUE and reprint the rows after it.        */
            if( row < 1 ) scrl_dwn( trows/2 ) ;
            del_row() ;
    }
    return( TRUE ) ;
}

/***********************************************************
Del_row() : When the previous row continues in the current
row and the current row only contains a newline, the current
row must be removed, the previous row must become current
and it's eoln flag set to TRUE. Finally the rows after the
current row must be scrolled up.
***********************************************************/

del_row()
{
    row-- ;
    get_prev() ;
    col = tot_len ;
    eoln = TRUE ;
    cursor( row, tot_len ) ;
    putchar(' ') ;
    pr_rest( prev_row->next, row + 1 ) ;
}

/***********************************************************
Shift() : moves 'n' characters from rp->next->row to the end
of the row_buf. The row_buf is inserted and the end of rp->
next->row is moved to the start of row_buf. Recursive calls
are made until a row containing an eoln is reached. The rows
are printed if row is within the window.
***********************************************************/

shift( rp, n, row )
struct row_p *rp ;
int n, row ;
{
    int len, m ;
    char *get_row() ;

    m = tcols - 1 - n ;
    if( rp->next->eoln ) {

        /* Row contains eoln                              */
        if( ( len = rp->next->length ) <= n ) {
            cpystr( get_row( rp->next ) ,
                    len ,
                    row_buf + m ) ;
            insert( row_buf, rp, m + len, TRUE ) ;
            pr_rest( rp->next, row ) ;
            return ;
        }
        else {
            cpystr( rp->next->row ,
                    n ,
                    row_buf + m ) ;
            printr( row_buf, 0, tcols - 1, row, FALSE ) ;
            insert( row_buf, rp, tcols - 1, FALSE ) ;
            rp = rp->next ;
            cpystr( get_row( rp->next ) + n ,
                    len - n ,
                    row_buf ) ;
            insert( row_buf, rp, len - n, TRUE ) ;
            if( ++row < wdw_size ) {
                pr_row( row_buf, 0, len - n ) ;
                pr_row( space, len - n, tcols ) ;
            }
        }
    }
    else {
        cpystr( rp->next->row ,
                n ,
                row_buf + m ) ;
        insert( row_buf, rp, tcols - 1, FALSE ) ;
        printr( row_buf, 0, tcols - 1, row, FALSE ) ;
        rp =rp->next ;
        cpystr( get_row( rp->next ) + n ,
                m ,
                row_buf ) ;
        shift( rp, n, ++row ) ;
    }
}

/***********************************************************
Printr() : outputs a row on the CRT if row's within the win-
dow.
***********************************************************/

printr( str, fr, to, row, eoln )
char *str ;
int fr, to, row ;
FLAG eoln ;
{
    if( row >= wdw_size ) return ;
    pr_row( str, fr, to ) ;
    if( eoln ) putchar('\n') ;
    else putchar('!') ;
}


/***********************************************************
Pr_rest() : clears the screen after row + n and prints out
the rows pointed to by rp. Finally the dotted line is
printed after the window.
***********************************************************/

pr_rest( rp, row )
struct row_p *rp ;        
int          row   ;
{
    int i ;
    cclrrest( row, 0 ) ;
    putchar(' ') ; curleft() ;
    for( i = row ; i < wdw_size ; i++ ) {

        /* No more rows to print ???                      */
        if( rp == NULL ) break ;
        pr_row( rp->row, 0, rp->length ) ;
        if( rp->eoln ) putchar('\n') ;
        else putchar('!') ;
        rp = rp->next ;
    }
    put_msg( wdw_size, end_wdw ) ;
}

/***********************************************************
Get_nxt() : makes the next row current.
***********************************************************/

get_nxt()
{
    prev_row = prev_row->next ;
    get_current() ;
}

/***********************************************************
Get_prev() : makes the previous row current.
***********************************************************/

get_prev()
{
    prev_row = prev_row->previous ;
    get_current() ;
}

/***********************************************************
Get_current() : moves the row pointed to by prev_row->next
to the row_buf.
***********************************************************/

get_current()
{
    char *get_row() ;
    
    tot_len  = prev_row->next->length ;
    eoln     = prev_row->next->eoln ;
    cpystr(  get_row( prev_row->next ) ,
             tot_len ,
             row_buf ) ;
}    
