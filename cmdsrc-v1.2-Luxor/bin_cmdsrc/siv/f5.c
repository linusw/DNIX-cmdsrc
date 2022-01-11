/***********************************************************
This file contains the actual commands, i.e. the actions
taken when the user presses one or a combination of control-
characters.
***********************************************************/

#include           <stdio.h>
#include           "define.h"
#include           "struct.h"

extern int          col       ;
extern int          row       ;
extern int          tcols     ;
extern int          wdw_size  ;
extern int          trows     ;
extern int          tot_len   ;
extern int          eoln      ;
extern long         mark      ;
extern char         *space    ;
extern char         prev_cmnd ;
extern char         cmnd      ;
extern char         *row_buf  ;
extern char         *end_wdw  ;
extern struct row_p kill_buf  ;
extern struct row_p *prev_row ;
extern struct row_p first_r   ;

/***********************************************************
Set_mark() : counts the number of characters before the
cursor.
***********************************************************/

set_mark()
{
    struct row_p *rp ;

    if( prev_row == &first_r ) {
        mark = col ;
    }
    else {
        mark = 0 ;
        rp = &first_r ;
        while( rp != prev_row ) {
            rp = rp->next ;
            mark = mark + rp->length + eoln ;
        }
        mark = mark + col ;
    }
}

/***********************************************************
Ctrl_A() : moves the cursor to the beginning of the line.
I.e. to the position after the latest newline. 
***********************************************************/

ctrl_A()
{
    struct row_p *ins_text() ;
    char *get_row() ;

    /* No need to search ???                              */    
    if( prev_row->eoln OR firstrow() ) col = 0 ;
    else {

        /* Search for the row                             */
        insert( row_buf, prev_row, tot_len, eoln ) ;
        while( NOT prev_row->eoln AND NOT firstrow() ) {
            row-- ;
            prev_row = prev_row->previous ;
        }

        /* Get current row                                */
        get_current() ;

        /* Outside the window ???                         */
        if( row < 0 ) rewrite() ;
        col = 0 ;
    }
}

/***********************************************************
Ctrl_B() moves the cursor one step backwards. If the cursor 
is at the left margin it moves to the previous row (if 
there is one) . If the cursor is in the upper leftmost
corner you also have to scroll the screen. 
***********************************************************/

ctrl_B()
{
    struct row_p *rp, *ins_text() ;
    char *get_row() ;

    
    if( col > 0 ) {

        /* Just move it left...                            */
        col-- ;
        curleft() ;
    }
    else {
        if( prev_row->previous != NULL ) {

            /* Not the first row                           */
            if( row == 0 ) scrl_dwn( trows / 2 ) ;

            /* Save the current row                        */
            insert( row_buf, prev_row, tot_len, eoln ) ;

            /* And make the previous row current           */
            get_prev() ;
            if( eoln ) col = tot_len ;
            else col = tot_len - 1 ;
            row-- ;
        }
        else {
            /* At the first row                            */
            pr_msg( TOP_FL, NULL ) ;
        }
    }
}

/***********************************************************
Ctrl_E() : moves the cursor to the end of the line i.e. to 
the next newline.
***********************************************************/
            
ctrl_E()
{
    struct row_p *ins_text() ;
    char *get_row() ;

    /* No need to search ???                              */
    if( eoln ) col = tot_len ;
    else {
        insert( row_buf, prev_row, tot_len, eoln ) ;

        /* Search for a newline                           */
        while( NOT prev_row->next->eoln ) {
            row++ ;
            prev_row = prev_row->next ;
        }

        /* Get current row                                */
        get_current() ;

        /* Out of window ???                              */ 
        if( row >= wdw_size ) rewrite() ;
        col = tot_len ;
    }
}

/***********************************************************
Ctrl_f() : moves the cursor right unless it reaches a eoln
which makes it change row. A up-scroll is performed if
it reaches the end of the last row in a window.
***********************************************************/

ctrl_F()
{
    struct row_p *ins_text() ;
    char *get_row() ;
    
    if( ( col == tot_len ) OR
        ( ( NOT eoln ) AND ( col + 1 == tot_len ) ) ) {

            /* The cursor is at the end of a row. Change  */
            /* to next row if not at eof                  */
            if( last_row() ) {
                pr_msg( END_FL, NULL ) ;
                return ;
            }
            else {
                if( row >= (trows - 3) ) scrl_up( trows / 2 ) ;

                /* Save the current row                   */
                insert( row_buf, prev_row, tot_len, eoln ) ;

                /* Make next row current                  */
                get_nxt() ;
                row++ ;
                col = 0 ;
            }
        }
        else col++ ;      /* Just move right              */
}

tab()
{
    int i ;

    for( i = col % 8 ; i < 8 ; i++ ) add_char(' ') ;
}


/***********************************************************
Ctrl_L() : rewrites the whole window.
***********************************************************/

ctrl_L()
{
    int i ;
    struct row_p *rp ;

    clrscr() ;    
    i = row - 1 ;
    rp = prev_row ;
    while( i > 0 ) {

        /* Look for the first row in the window           */
        i-- ;
        rp = rp->previous ;
    }
    if( i < 0 ) {

        /* The current row was the first row             */
        pr_row( row_buf, 0, tot_len ) ;
        if( eoln ) putchar('\n') ;
        else putchar('!') ;
        i = 1 ;
        rp = rp->next ;
    }
    for( ; i < wdw_size ; i++ ) {

        /* Print all the rows                             */
        if( rp == NULL ) break ;
        pr_row( rp->row, 0, rp->length ) ;
        if( rp->eoln ) putchar('\n') ;
        else putchar('!') ;
        if( rp == prev_row ) {

            /* Print the current row                      */
            pr_row( row_buf, 0, tot_len ) ;
            if( eoln ) putchar('\n') ;
            else putchar('!') ;
            i++ ;
        }
        rp = rp->next ;
    }
    put_msg( wdw_size, end_wdw ) ;
}


/***********************************************************
Ctrl_N() : moves the cursor down to the next row unless
it's the last row in the buffer. A scroll up is performed
if it reaches the last row in a window.
***********************************************************/

ctrl_N()
{
    struct row_p *ins_text() ;
    char *get_row() ;

    if( last_row() ) {
        pr_msg( END_FL, NULL ) ;
        return ;
    }
    else {
        if( row >= (trows - 3) ) scrl_up( trows / 2 ) ;

        /* Save the current row                           */
        insert( row_buf, prev_row, tot_len, eoln ) ;

        /* Make next row current                          */
        get_nxt() ;
        row++ ;
        if( tot_len == tcols - 1 ) {
            if( NOT eoln ) col = tot_len - 1 ;
        }
        else col = ( col > tot_len ? tot_len : col ) ;
    }
}

/***********************************************************
Ctrl_P() : same as ctrl_N but moves the cursor up.
***********************************************************/

ctrl_P()
{
    struct row_p *ins_text() ;
    char *get_row() ;

    if( prev_row->previous == NULL ) {
        pr_msg( TOP_FL, NULL ) ;
        return ;
    }
    else {
        if( row == 0 ) scrl_dwn( trows / 2 ) ;
        insert( row_buf, prev_row, tot_len, eoln ) ;
        get_prev() ;
        row-- ;
        if( tot_len == tcols - 1 ) {
            if( NOT eoln ) col = tot_len - 1 ;
        }
        else col = ( col > tot_len ? tot_len : col ) ;
    }
}
        
/***********************************************************
Ctrl_O() : acts like a new_line() but the cursor stays on
the same position.
***********************************************************/

ctrl_O()
{
    int tmp_col ;

    tmp_col = col ;
    if( ( col == 0 )       AND
        ( NOT firstrow() ) AND
        ( NOT prev_row->eoln ) ) tmp_col = tcols - 1 ;
    new_line() ;
    cursor( row, col ) ;
    ctrl_P() ;
    col = tmp_col ;
}


/***********************************************************
Ctrl_Q() : makes it possible to write a control character. A
'^' is written on the window but if you put the cursor at
the character you can see what character it is in the
message area.
***********************************************************/

FLAG ctrl_Q()
{
    char c ;

    c = getchar() ;
    if( c < ' ' ) add_char( c ) ;
}

/***********************************************************
Ctrl_T() : exchanges the two characters before the cursor, 
only characters on the same row can be exchanged .
***********************************************************/

FLAG ctrl_T()
{
    char c1, c2 ;

    if( col == 0 || col == 1 ) return( FALSE ) ;
    c1 = *(row_buf + col - 2 ) ;
    c2 = *(row_buf + col - 1 ) ;
    *(row_buf + col - 1) = c1 ;
    *(row_buf + col - 2) = c2 ;
    curleft() ; curleft() ;
    putchar( c2 ) ;
    putchar( c1 ) ;
    return( TRUE )  ;
}

/***********************************************************
Ctrl_U() : used to print arg characters or move the cursor
arg rows up or down.
***********************************************************/

ctrl_U()
{
    int arg, chr, i ;
    
    arg = 0 ;
    pr_msg( ARGMNT, NULL ) ;

    /* Get numeric argument, maxsize MAXARG               */
    while( isdigit( chr = getchar () ) ) {
        arg = chr - '0' + arg * 10 ;
        if( arg > MAXARG ) {
            pr_msg( BIGARG, NULL ) ;
            return ;
        }
        putchar( chr ) ;
    }
    if( arg > 0 ) {
        if( printable( chr ) ) {
            for( i = 0 ; i < arg ; i++ ) add_char( chr ) ;
        }
        else if( chr == CTRL_Q ) {
            if( ( chr = getchar() ) < 127 )
                  for( i = 0 ; i < arg ; i++ ) add_char( chr ) ;
        }
        else if( chr == CTRL_N ) {
            down( arg ) ;
        }
        else if( chr == CTRL_P ) {
            up( arg ) ;
        }
        else pr_msg( NO_IMP, NULL ) ;
    }
}

/***********************************************************
Ctrl_V()( : moves the cursor to the next window unless it's
at the last row.
***********************************************************/

ctrl_V()
{
    struct row_p *ins_text() ;
    char *get_row() ;

    /* At the last row ???                                */
    if( last_row() ) {
        pr_msg( END_FL, NULL ) ;
        return ;
    }
    clrscr() ;
    if( row != trows - 3 ) {

        /* Save the current row                           */
        insert( row_buf, prev_row, tot_len, eoln ) ;

        /* Find a new current                             */
        while( row < trows - 3 ) {
            if( last_row() ) break ;
            prev_row = prev_row->next ;
            row++ ;
        }
        get_prev() ;
    }

    /* Rewrite the screen                                 */
    rewrite() ;
    col = 0 ;
}





ctrl_X()
{
    register char c ;

    if(  (c = getchar()) == 'S' || c == 's' ) {
        c = CTRL_S ;
    }
    switch( c ) {

        case CTRL_V :          /* Read file               */
             newfile() ;
             break ;

        case CTRL_F :          /* Save file and exit      */
             wr_exit() ;
             break ;

        case CTRL_S :          /* Save file               */
             save() ;
             break ;

        case CTRL_I :          /* Insert file             */
             ins_file() ;
             break ;

        case CTRL_L :          /* file information        */
             info() ;
             break ;

        default     :          /* Not implemented         */
             not_imp() ;
             break ;

    }
}

/***********************************************************
Ctrl_Y() : moves the contents in the kill-buffer to the
position before the cursor. The cursor position after the
move becomes after the block.
***********************************************************/

ctrl_Y()
{
    struct row_p *kill_p, *rp ;
    int tmp_row ;

    /* Anything in the kill-buffer ???                     */
    if( kill_buf.next == NULL ) return ;

    /* Split the current row in two. Save 2nd half.        */
    insert( row_buf + col, prev_row, tot_len - col, eoln ) ;
    tot_len = col ;
    kill_p  = kill_buf.next ;
    rp      = prev_row ;
    tmp_row = row ;
    while( TRUE ) {

        /* Insert from kill-buffer                         */
        fill_row( kill_p, tot_len ) ;
        if( kill_p->next == NULL ) break ;
        kill_p = kill_p->next ;
    }
    col =tot_len ;
    if( NOT eoln ) {

        /* The last row in the kill-buffer didn't end with */
        /* a newline. I.e. add the second half of the      */
        /* split row above to it.                          */
        shift( prev_row, tcols - 1- tot_len, 99 ) ;
        if( col == tcols - 1 AND NOT prev_row->next->eoln ) {
            prev_row = prev_row->next ;
            row++ ;
            col = 0 ;
        }
    }
    /* Reprint screen ???                                  */
    if( row < wdw_size ) pr_rest( rp->next, tmp_row ) ;
    else reprint() ;
    get_current() ;
}

/***********************************************************
Ctrl_W() : Moves the rows between mark and cursor (region)
to the kill-buffer.
***********************************************************/

ctrl_W()
{
    struct row_p *rp ;

    /* Any mark set ???                                   */
    if( mark == NOTSET ) {
        clr_msg() ;
        put_msg( trows - 1, "No mark set" ) ;
        return ;
    }

    /* Scratch kill-buffer                                */
    scratch( &kill_buf ) ;
    insert( row_buf, prev_row, tot_len, eoln ) ;
    rp = &first_r ;
    while( TRUE ) {

        /* Look for mark                                  */
        if( rp == prev_row ) {

            /* Check if mark and cursor in same row       */
            same_row( prev_row ) ;
            break ;
        }
        else if( mark <= rp->next->length ) {

            /* Found mark. Move region.                   */
            found_mark( rp ) ;
            break ;
        }

        /* The search goes on....                         */
        mark = mark - rp->next->length - rp->next->eoln ;
        rp = rp->next ;
    }

    /* Reset mark                                         */
    mark = 0 ;
}

/***********************************************************
Ctrl_Z() : scrolls the screen on row up unless the cursor's
at the first row.
***********************************************************/

ctrl_Z()
{
    struct row_p *rp ;
    int          i ;

    if( row > 0 ) {
        rp = prev_row->next ;
        scrl_1up() ;
        for( i = row + 2 ; i < wdw_size ; i++ ) {
            if( rp == NULL ) return ;
            rp = rp->next ;
        }
        if( rp == NULL ) return ;
        cursor( wdw_size - 1, 0 ) ;
        pr_row( rp->row, 0, rp->length ) ;
        if( NOT rp->eoln ) putchar('!') ;
    }
    else return( FALSE ) ;
}

/***********************************************************
This one's real nasty ( and long ). There are several cases
to handle , but don't give up :

 1. If you're at the first position in a row and the 
    previous row didn't have an eoln, you must set the
    eoln flag in the previous row and overwrite it's '!'.
    
 2. If you're not at the last row of the window there are 
    two cases :
   
   2.1 The sentence ends on this row also has two cases :

     2.1.1 You're at the end of that row. Just insert a
           blank row after it.

     2.1.2 You're in the middle of the row. Insert a blank
           row and push down the characters to the right of
           the cursor.

   2.2 The line lasts over several rows. In this case
       you push down the characters to the right to the next
       row until you reach the "eoln"-row. If there's
       "overflow" in the last row you have to insert a new
       row and push the overflow into it.

 3. If you're at the last row of the window you must scroll
    up the screen and call newline recursively.

zzzzzzzzz.....

***********************************************************/

new_line()
{
    int tmp_len, i, n ;
    struct row_p *rp, *ins_text() ;

    if( ( col == 0 )       AND
        ( NOT firstrow() ) AND
        ( NOT prev_row->eoln ) ) {
        
        /* See 1. above                                   */
        prev_row->eoln = TRUE ;
        if( row > 0 ) {
            cursor( row - 1, tcols - 1 ) ;
            putchar(' ') ;
        }
    }
    else if( row < (trows - 3) ) {

        /* See 2 above                                   */
        if( eoln ) {
            /* See 2.1                                   */
            if( col == tot_len ) {
                /* See 2.1.1                             */
                cursor( ++row, col = 0 ) ;
                if( ! last_row() ) ins_blank( 0 ) ;
                insert( row_buf, prev_row, tot_len, eoln ) ;
                prev_row = prev_row->next ;
                tot_len = 0 ;
                return ;
            }
            else {
                /* See 2.1.2                             */
                tmp_len = tot_len ;
                tot_len = col ;
                insert( row_buf, prev_row, tot_len, eoln ) ;
                prev_row = prev_row->next ;
                tot_len = tmp_len - col ;
                for( i = 0 ; i < tot_len ; i++ ) {
                     *(row_buf + i) = *(row_buf + col + i) ;
                     putchar(' ') ;
                }
                cursor( ++row, col = 0 ) ;
                if( ! last_row() ) ins_blank( 0 ) ;
                cursor( row, col ) ;
                pr_row( row_buf, 0, tot_len ) ;
            }
        }
        else {

            /* See 2.2                                   */
            n = 0 ;
            tot_len = col ;
            eoln = TRUE ;
            insert( row_buf, prev_row, tot_len, eoln ) ;
            rp = prev_row = prev_row->next ;
            tmp_len = tcols -1 - col ;
            for( i = 0 ; i < tmp_len ; i++ ) {
                 *(row_buf + i) = *(row_buf + col + i) ;
                 putchar(' ') ;
            }
            putchar(' ') ;
            tot_len = tcols - 1 ;
            while( ! prev_row->next->eoln ) {
                cpystr( prev_row->next->row  ,
                        col ,
                        row_buf + tmp_len ) ;
                n++ ;
                if( row + n < wdw_size ) {
                    pr_row( row_buf, 0, tcols - 1 ) ;
                    putchar('!') ;
                }
                eoln = FALSE ;
                insert( row_buf, prev_row, tot_len, eoln ) ;
                prev_row = prev_row->next ;
                cpystr( get_row( prev_row->next ) + col ,
                        tmp_len ,
                        row_buf ) ;
            }
            if( col >= prev_row->next->length ) {
                tot_len = prev_row->next->length ;
                cpystr( get_row( prev_row->next ) ,
                        tot_len ,
                        row_buf + tmp_len ) ;
                n++ ;
                tot_len = tmp_len + tot_len ;
                if( row + n < wdw_size )
                    pr_row( row_buf, 0, tot_len ) ;
                eoln = TRUE ;
                insert( row_buf, prev_row, tot_len, eoln ) ;
            }
            else {
                cpystr( prev_row->next->row ,
                        col ,
                        row_buf + tmp_len ) ;
                n++ ;
                if( row + n < wdw_size ) {
                    pr_row( row_buf, 0 , tcols - 1 ) ;
                    putchar('!') ;
                }
                eoln = FALSE ;
                insert( row_buf, prev_row, tot_len, eoln ) ;
                prev_row = prev_row->next ;
                n++ ;
                tot_len = prev_row->next->length - col ;
                cpystr( get_row( prev_row->next ) + col ,
                        tot_len ,
                        row_buf ) ;
                eoln = TRUE ;
                if( row + n < wdw_size ) {
                    ins_blank( n ) ;                
                    cursor( row + n, 0 ) ;
                    pr_row( row_buf, 0 , tot_len ) ;
                }
                insert( row_buf, prev_row, tot_len, eoln ) ;
            }
            prev_row = rp ;
            tot_len = prev_row->next->length ;
            eoln = prev_row->next->eoln ;
            cpystr( get_row( prev_row->next ) ,
                    tot_len ,
                    row_buf ) ;
            cursor( ++row, col = 0 ) ;
        }
    }
    else {
        
        /* See 3.                                        */
        scrl_up( trows / 2 ) ;
        cursor( row, col ) ;
        new_line() ;
    }
}

/***********************************************************
To_killbuf() : unlinks the row pointed to by rp from the
row_p list and puts it at the end of the kill-buffer.
***********************************************************/

to_killbuf( rp )
struct row_p *rp ;
{
    struct row_p *kill_p ;
    
    kill_p = &kill_buf ;

    /* Find last position                                 */
    while( kill_p->next != NULL ) {
        kill_p = kill_p->next ;
    }
    kill_p->next = rp->next ;
    kill_p->next->previous = kill_p ;
    rp->next = kill_p->next->next ;
    kill_p->next->next = NULL ;
    if( rp->next != NULL ) rp->next->previous = rp ;
}

/***********************************************************
Fill_row() : tries to fill row_buf with the kill-buffer-row
pointed to by rp->row. 
***********************************************************/

fill_row( rp, n )
struct row_p *rp ;
int n ;
{
    int to_fill, len ;

    to_fill = tcols - 1 - n ;
    if( rp->length > to_fill ) {

        /* rp->row's too large. Copy to_fill chars        */
        cpystr( rp->row, to_fill, row_buf + n ) ;

        /* Save it after prev_row                         */
        insert( row_buf, prev_row, tcols - 1, FALSE ) ;
        row++ ;
        prev_row = prev_row->next ;
        tot_len  = rp->length - to_fill;
        eoln     = rp->eoln ;

        /* Copy rest of rp->row to row_buf                */
        cpystr( rp->row + to_fill ,
                tot_len ,
                row_buf ) ;
        if( eoln ) {

            /* Save it if newline-terminated              */
            insert( row_buf, prev_row, tot_len, TRUE ) ;
            row++ ;
            tot_len = 0 ;
            prev_row = prev_row->next ;
        }
    }
    else {

        /* Rp->row fits !!!                               */
        eoln    = rp->eoln ;
        cpystr( rp->row , rp->length, row_buf + n ) ;
        tot_len = n + rp->length ;
        if( eoln ) {

            /* Save if newline-terminated                 */
            insert( row_buf, prev_row, tot_len, eoln ) ;
            row++ ;
            tot_len = 0 ;
            prev_row = prev_row->next ;
        }
    }
}

/***********************************************************
Kill_blk() : moves the row_p's from p1->next to p2->previous
to the kill-buffer.
***********************************************************/


kill_blk( p1 TO p2 )
struct row_p *p1, *p2 ;
{
    kill_buf.next      = p1->next ;
    p1->next->previous = &kill_buf ;
    p2->previous->next = NULL ;
    p1->next           = p2 ;
    p2->previous       = p1 ;
}

/***********************************************************
Same_row() : if mark is before col ( the cursor ) that area
is moved to the kill_buffer.
***********************************************************/    

same_row( rp )
struct row_p *rp ;
{
    char *get_row() ;

    cpystr( get_row( rp->next ), tot_len, row_buf ) ;
    if( mark < col ) {

        /* Mark's before cursor. Move!!!                  */
        /* Insert are to move.                            */
        insert( row_buf + mark, rp, col - mark, FALSE ) ;

        /* Move rp->next to kill_buffer                   */
        to_killbuf( rp ) ;

        /* Shift right part of row_buf col-mark steps left */
        cpystr( row_buf + col, col - mark, row_buf + mark ) ;
        cursor( row, 0 ) ;
        tot_len = tot_len - col + mark ;
        pr_row( row_buf, 0, tot_len ) ;
        pr_row( space, 0, col - mark ) ;
        col = mark ;
    }
    else {

        /* Marks after cursor                             */
        clr_msg() ;
        put_msg( trows - 1, "No mark before the cursor" ) ;
    }
}

/***********************************************************
Found_mark() : Moves a block of several rows to the kill-
buffer.
***********************************************************/

found_mark( rp )
struct row_p *rp ;
{
    int len, i ;
    struct row_p *p ;
    char *get_row() ;

    p = rp ;

    /* Find current row.                                  */
    while( TRUE ) {
        if( p == prev_row ) break ;
        p = p->next ;
        row-- ;
    }
    if( row < 0 ) row = 0 ;
    if( mark == 0 AND  col == 0 ) {

        /* No need to split any rows, "just" move...     */
        prev_row = prev_row->next ;
        kill_blk( rp TO prev_row ) ;
        prev_row = rp ;
        tot_len = prev_row->next->length ;
        eoln    = prev_row->next->eoln ;
        cpystr( get_row( prev_row->next ), tot_len, row_buf ) ;
    }
    else {

        /* Split current row                              */
        get_current() ;
        insert( row_buf, prev_row, col, FALSE ) ;
        prev_row = prev_row->next ;
        insert( row_buf + col, prev_row, tot_len - col, eoln ) ;
        prev_row = prev_row->next ;
        if( mark == 0 ) {

            /* No need to split "mark-row".                */
            kill_blk( rp TO prev_row ) ;
            prev_row = rp ;
            get_current() ;
            if( NOT eoln ) {
                shift( prev_row, tcols - 1- tot_len, 99 ) ;
                get_current() ;
            }
        }
        else {

            /* Split "mark-row"                           */
            get_current() ;
            insert( row_buf + mark, rp, tot_len - mark, eoln ) ;
            kill_blk( rp TO prev_row ) ;
            prev_row = rp ;
            tot_len  = mark ;
            eoln = FALSE ;
            shift( prev_row, tcols - 1 - tot_len , 99 ) ;
            get_current() ;
        }
    }
    col = mark ;
    ctrl_L() ;
}

/***********************************************************
Down() : moves the cursor n rows down (if possible) plus
reprints the screen if "out of window".
***********************************************************/

down( n )
int n ;
{
    int i, err ;
    
    insert( row_buf, prev_row, tot_len, eoln ) ;
    err = NO_ERR ;
    for( i = 0 ; i < n ; i++ ) {
        if( prev_row->next->next == NULL ) {
            err = END_FL ;
            break ;
        }
        prev_row = prev_row->next ;
        row++ ;
    }
    reprint() ;
    get_current() ;
    if( tot_len == tcols - 1 ) {
        if( NOT eoln ) col = tot_len - 1 ;
    }
    else col = ( tot_len < col ? tot_len : col ) ;
    pr_msg( err, NULL ) ;
}

/***********************************************************
Up() : moves the cursor n rows up if possible.
***********************************************************/
    
up( n )
int n ;
{
    int i, err ;

    insert( row_buf, prev_row, tot_len, eoln ) ;
    err = NO_ERR ;
    for( i = 0 ; i < n ; i++ ) {
        if( prev_row->previous == NULL ) {
            err = TOP_FL ;
            break ;
        }
        prev_row = prev_row->previous ;
        row-- ;
    }
    reprint() ;
    get_current() ;
    if( tot_len == tcols - 1 ) {
        if( NOT eoln ) col = tot_len - 1 ;
    }
    else col = ( tot_len < col ? tot_len : col ) ;
    pr_msg( err, NULL ) ;
}


isdigit( c )
char c ;
{
    return( c >= '0' AND c <= '9' ? TRUE : FALSE ) ;
}

