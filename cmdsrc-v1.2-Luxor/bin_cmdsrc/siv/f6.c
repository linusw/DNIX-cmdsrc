/***********************************************************
                   I M P O R T A N T

This is a tricky one. The routines in this file either 
inserts or appends a character at the cursor. There are
3 basic cases, but first a couple of notes :

  * A LINE denotes the characters between the previous
    carriage return and the next.

  * A ROW denotes the characters you see on one row
    of the CRT. I.e. a row is never longer than a line.

There are essentially three cases to handle :

  <1> The line is shorter than (tcols - 1), i.e. you can
      insert/append a character without adding a new row.
      
  <2> The length of the line is = (tcols - 1), i.e. you have
      to insert a blank line, take the last character in the 
      row and put it first in the blank row .

  <3> The row does not contain an eoln, i.e. the line occupies
      more than one row. In this case you have to push the
      last character in every row to the first position on
      the next row until you find a row that has a eoln.

***********************************************************/
 
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
extern char          *row_buf  ;
extern char          *tmp_buf  ;
extern struct row_p  *prev_row ;
extern struct row_p  first_r   ;

/***********************************************************
Add_char() appends or inserts a character into the current
position.
***********************************************************/

add_char( c ) char c ;
{
    cursor( row, col ) ;
    putchar( c < ' ' ? '^' : c ) ;

    if( eoln && (tot_len < (tcols - 1)) ) state_1( c ) ;
    else if( eoln ) state_2( c ) ;
    else state_3( c ) ;
}
	   
static state_1( c )
char c ;
{

    /* Shift the characters to the right one step right   */
    shift_f( row_buf, col, tot_len, c ) ;
    col++ ;
    tot_len++ ;

    /* Print the rightmost characters                     */
    pr_row( row_buf , col, tot_len ) ;
}


static state_2( c )
char c ;
{
    struct row_p *ins_text() ;
    char tmp_chr ;

    if( col == tot_len ) {

	/* Save the old row & create a new one           */
	eoln = FALSE ;
	insert( row_buf, prev_row, tot_len, eoln ) ;

	/* Make the new row "current"                    */
	prev_row = prev_row->next ;
	eoln = TRUE ;
	curleft() ;
	putchar('!') ;
	ins_blank( 1 ) ;
	col = 0 ;
	cursor( ++row, col++ ) ;
	putchar(c) ;
	tot_len = 1 ;
	*row_buf = c ;
    }
    else {
	if( col == (tot_len - 1) ) {

	    /* Push down the last character to a new row */
	    /* Put 'c' in it's place & set the new row   */
            /* to the current row.                       */
	    tmp_chr = *(row_buf + tcols - 2) ;
	    *(row_buf + tcols - 2) = c ;
	    putchar('!') ;
	    ins_blank( 1 ) ;
	    cursor( row + 1, 0 ) ;
	    putchar( tmp_chr ) ;
	    eoln = FALSE ;
	    insert( row_buf, prev_row, tot_len, eoln ) ;
	    col = 0 ; row++ ;
	    *row_buf = tmp_chr ;
	    tot_len = 1 ;
	    eoln = TRUE ;
	    prev_row = prev_row->next ;
	}
	else {

	    /* Push down the last character to a new row */
	    /* Insert 'c' but don't change current row.  */
	    shift_f( row_buf, col, tot_len, c ) ;
	    col++ ;
	    c = *(row_buf + tcols - 1) ;
	    pr_row( row_buf, col, tot_len ) ;
	    putchar('!') ;
	    if( row < (trows - 3) ) {
		ins_blank( 1 ) ;
		cursor( row + 1, 0 ) ;
		putchar( c ) ;
	    }
	    tot_len = 1 ;
	    insert( &c, prev_row, tot_len, eoln ) ;
	    eoln = FALSE ;
	    tot_len = tcols - 1 ;
	}
    }
}

static state_3( c )
char c ;
{
    char tmp_chr ;
    int i ;
    struct row_p *rp, *ins_text() ;


    
    if( col == (tot_len - 1 )) {

	/* Push down the last character to a next row    */
	/* Put 'c' last in the current row and then set  */
	/* the next row to current.                      */
	tmp_chr = c ;
	c = *(row_buf + tcols - 2) ;
	*(row_buf + tcols - 2) = tmp_chr ;
	col = 0 ; row++ ;
	if( row > (trows - 3) ) {

	    /* Scroll the window                           */
	    scrl_up( trows / 2 ) ;
	    i = row ;
	    rp = prev_row->next ;
	    for( i ; i < (tcols - 2) ; i++ ) {
		if( rp == NULL ) break ;
		pr_row( rp->row, 0, rp->length ) ;
		if( rp->eoln ) putchar('\n') ;
		else putchar('!') ;
		rp = rp->next ;
	    }
	}
	cursor( row, col ) ;
	insert( row_buf, prev_row, tot_len, eoln ) ;
	prev_row = prev_row->next ;
	tot_len = prev_row->next->length ;
	eoln = prev_row->next->eoln ;
	cpystr( get_row( prev_row->next ) ,
	        tot_len ,
		row_buf ) ;
	add_char( c ) ;
	col = 0 ;
	return ;
    }
    else {

	/* Don't change the current row but push down    */
	/* the last character in every row until you     */
	/* find a eoln.                                   */
	rp = prev_row ;
	i = 0 ;
	shift_f( row_buf, col, tot_len, c ) ;
	c = *(row_buf + tcols - 1) ;
	pr_row( row_buf, ++col, tot_len ) ;
	putchar('!') ;
    }
    while( !prev_row->next->eoln ) {
	cpystr( get_row( prev_row->next ) ,
	        prev_row->next->length ,
		tmp_buf + 1 ) ;
	*tmp_buf = c ;
	c = *(tmp_buf + tcols - 1) ;
	i++ ;
	if( (row + i) < wdw_size ) {
	    pr_row( tmp_buf, 0, tcols -1 ) ;
	    putchar('!') ;
	}
	insert( tmp_buf, prev_row, tot_len, eoln ) ;
	prev_row = prev_row->next ;
    }

    /* A row containing a eoln has been found.           */
    tot_len = prev_row->next->length ;
    cpystr( get_row( prev_row->next ) ,
            tot_len ,
	    tmp_buf + 1 ) ;
    *tmp_buf = c ;
    if( tot_len == (tcols - 1) ) {

	/* The last character must be pushed down to a    */
	/* blank row.                                    */
	c = *(tmp_buf + tcols - 1) ;
	i++ ;
	if( (row + i) < wdw_size ) {
	    pr_row( tmp_buf, 0, tot_len ) ;
	    putchar('!') ;
	    if( row + i < wdw_size - 1 ) ins_blank( i + 1 ) ;
	}
	eoln = FALSE ;
	insert( tmp_buf, prev_row, tot_len, eoln ) ;
	prev_row = prev_row->next ;
	*tmp_buf = c ;
	tot_len = 0 ;
    }
    i++ ;
    ++tot_len ;
    if( (row + i) < wdw_size ) {
	cursor( row + i, 0 ) ;
        pr_row( tmp_buf, 0, tot_len ) ;
    }
    eoln = TRUE ;
    insert( tmp_buf, prev_row, tot_len, eoln ) ;
    eoln = FALSE ;
    prev_row = rp ;
    tot_len = tcols - 1 ;
}




/***********************************************************
Shift_f() shifts the characters after the current position
in "buf" one step forward and inserts "c" at the current
position.
***********************************************************/

static shift_f( buf, from, to, c )
char *buf, c ;
int  from, to ;
{
    for( ; to >= from ; to-- ) {
	*(buf + to + 1) = *(buf + to) ;
    }
    *(buf + from) = c ;
}

/***********************************************************
Pr_row() prints the characters within 'from' to 'to' in the
buffer 'buf'.
***********************************************************/

pr_row( buf, from, to ) 
register char *buf ;
register int  from, to ;
{
    register int  n ;
    register char out_b[200] ;

    n = 0 ;
    buf = buf + from ;
    while( from < to ) {
	if( *buf < ' ' ) *(out_b + n) = '^' ;
	else *(out_b + n) = *buf ;
	from++ ;
	n++ ;
	buf++ ;
    }
    write( 1, out_b, n ) ;
}







