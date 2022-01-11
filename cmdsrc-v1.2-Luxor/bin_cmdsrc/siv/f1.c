
/***********************************************************
Misc. functions...
***********************************************************/

#include             <stdio.h>
#include             "define.h"
#include             "struct.h"

extern int           tcols ;
extern int           trows ;
extern int           row ;
extern int           col ;
extern int           wdw_size ;
extern int           language ;
extern char          *end_wdw ;
extern struct row_p  *prev_row ;
extern char          *row_buf ;
extern int           eoln ;
extern int           tot_len ;
extern char          *file_name ;
extern char          *search ;
extern char          *space ;
extern char          *replace ;

/*****************************************************************
Outstr() is used instead of printf() i.e. for printing a string
on the tty.
*****************************************************************/

outstr( string )
char *string ; 
{
    while (*string) {
	putchar(*string) ;
	*string++ ;
    }
}

/***********************************************************
Clr_msg() clears the message area.
***********************************************************/

clr_msg()
{
    cursor( trows - 1, 0 ) ;
    pr_row( space, 0, tcols - 1 ) ;
     cursor( trows - 1, 0 ) ;
}

/***********************************************************
Put_msg() writes a string at row 'r'.
***********************************************************/

put_msg(r, string)
char *string ;
int r ;
{
    cursor( r, 0 ) ;
    printf( string ) ;
}

    

not_imp()
{
    pr_msg( NO_IMP, NULL ) ;
}


/***********************************************************
Cpystr() copies a string at "from" of length "l" to "to".
***********************************************************/

cpystr( from, l, to ) char *from, *to ; int l ;
{
    while( l-- ) *to++ = *from++ ;
}

/***********************************************************
Last_row() : checks if the current row is the last row.
I.e. if prev_row->next == NULL.
***********************************************************/

FLAG last_row()
{
    return( prev_row->next == NULL ? TRUE : FALSE ) ;
}

/***********************************************************
Firstrow() : checks if the current row is the first one.
***********************************************************/

FLAG firstrow()
{
    return( prev_row->previous == NULL ? TRUE : FALSE ) ;
}

/***********************************************************
Rewrite() : rewrites the whole screen and puts the current
row at the top of the window.
***********************************************************/

rewrite()
{
    struct row_p *rp ;
    int i ;
    
    row = 0 ; rp = prev_row->next ;
    clrscr() ;
    pr_row( row_buf, 0, tot_len ) ;
    if( eoln ) putchar('\n') ;
    else putchar('!') ;
    for( i = 1 ; i < wdw_size ; i++ ) {
	if( rp == NULL ) break ;
	pr_row( rp->row, 0, rp->length ) ;
	if( rp->eoln ) putchar('\n') ;
	else putchar('!') ;
	rp = rp->next ;
    }
    put_msg( wdw_size, end_wdw ) ;
}


printable( c ) 
int c ;
{
    return( c >= 32 && c <= 126 ? TRUE : FALSE ) ;
}

static char *messages[] = {
    " " ,
    " " ,
    "Top of file" ,
    "B|rjan av filen" ,
    "End of file" ,
    "Slut p} filen" ,
    "Not implemented" ,
    "Ej implementerad" ,
    "Do you really want to exit (y/n) ??? " ,
    "Vill du verkligen sluta (j/n) ??? " ,
    "Space = replace, N = next, CTRL_G = quit" ,
    "Mellanslag = byt, N = n{sta, CTRL_G = bryt" ,
    "Replaced %s occurrences" ,
    "Bytte %s f|rekomster" ,
    "Search for : " ,
    "S|k efter : " ,
    "Aborted" ,
    "Avbr|t" ,
    "Can't find it" ,
    "Hittar den ej" ,
    "Old string : " ,
    "Gammal str{ng : " ,
    "New string : " ,
    "Ny str{ng : " ,
    "Read file : " ,
    "L{s fil : " ,
    "Insert file : " ,
    "S{tt in fil : " ,
    "Can't save %s" ,
    "Kan inte spara %s" ,
    "Wrote %s" ,
    "Skrev %s" ,
    "Nonexistent file : %s" ,
    "%s existerar ej"  ,
    "Can't insert here" ,
    "Kan inte s{tta in h{r" ,
    "New file : %s" ,
    "Ny fil : %s" ,
    "Can't edit directories" ,
    "Kan inte editera directories" ,
    "Read permission denied" ,
    "Inga l{sr{ttigheter" ,
    "Can't create %s" ,
    "Kan inte skapa %s" ,
    "Write permission denied" ,
    "Skrivr{ttigheter saknas" ,
    "Can't backup %s" ,
    "Kan inte skapa backup-kopia av %s" ,
    "Argument's too large" ,
    "F|r stort argument" ,
    "Argument : " ,
    "Argument : " 
} ;
    


pr_msg( n, str )
int n ;
char *str ;
{
    clr_msg() ;
    cursor( trows - 1, 0 ) ;
    printf( messages[ 2 * n + language ] , str ) ;
}

