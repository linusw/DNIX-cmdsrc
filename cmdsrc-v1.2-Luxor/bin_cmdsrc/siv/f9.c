/***********************************************************
This file contains the CTRL_X commands, i.e. the load-/save-
from file routines.
***********************************************************/

#include             "define.h"
#include             "struct.h"
#include             <stdio.h>
#include             <errno.h>
#include             <sys/types.h>
#include             <sys/stat.h>

extern int           wdw_size  ;
extern int           tot_len   ;
extern int           tcols     ;
extern int           trows     ;
extern int           col       ;
extern int           row       ;
extern int           errno;
extern FLAG          eoln      ;
extern char          *row_buf  ;
extern char          f_name[]  ;
extern struct row_p  *prev_row ;
extern struct row_p  first_r   ;


/***********************************************************
Newfile() : read the name of a file to be edit.
***********************************************************/

newfile()
{
    char buf[MAXLEN], chr ;
    FLAG file_legal() ;

    clr_msg() ;
    pr_msg( READ_F, NULL ) ;
    while( ( chr = getchar() ) == ' ' ) ;
    if( input( chr, buf, file_legal ) == ABORTED ) return ;
    newread( buf ) ;
}

/***********************************************************
Newread() : loads the editor with a new file. Anything
previously in the textbuffer is scratched.
***********************************************************/

newread( file )
char * file ;
{
    FILE *fopen(), *fp ;
    char *get_row() ;

    strcpy( f_name, file ) ;

    /* Scratch old contents in textbuffers                */
    scratch( &first_r ) ;
    prev_row = &first_r ;

    /* open file                                          */
    if( ( fp = fopen( f_name, "r" ) ) == NULL ) {

        /* File does not exist. Buffer is empty.          */
        col = row = tot_len = 0 ;
        ctrl_L() ;
        return ;
    }

    /* File exists. Load into the empty buffer            */
    readfile( fp ) ;
    fclose( fp ) ;

    /* Set global variables and move first row to row_buf */
    prev_row = &first_r ;
    col = row = 0 ;
    if( prev_row->next != NULL ) {

        /* File was not empty                             */
        tot_len = prev_row->next->length ;
        eoln = prev_row->next->eoln ;
        cpystr( get_row( prev_row->next ) ,
                tot_len ,
                row_buf ) ;
    }
    else {
        eoln = TRUE ;
        tot_len = 0 ;
    }
    ctrl_L() ;
}

/***********************************************************
wr_exit() :
***********************************************************/

wr_exit()
{
    if( FAILED save() ) return ;
    else {
        clr_msg() ;
        cursor( trows - 1, 0 ) ;
        reset_term() ;
        exit(0) ;
    }
}

/***********************************************************
Save() : saves the whole textbuffer in the file f_name
***********************************************************/

save()
{
    FILE *fopen(), *fp ;

    /* Check if file is writeable                         */
    if( NOT writeable( f_name ) ) return( FALSE ) ;

    /* Try to open file                                   */
    if( ( fp = fopen( f_name, "w" ) ) == NULL ) {
        pr_msg( NO_SAV, f_name ) ;
        return( FALSE ) ;
    }
    savefile( fp ) ;
    fclose( fp ) ;
    pr_msg( WRTE_F, f_name ) ;
    return( TRUE ) ;
}

/***********************************************************
Ins_file() : inserts a file in the current buffer, i.e. it's
content is not scratched.
***********************************************************/

ins_file()
{
    struct row_p *tmp_row ;
    FLAG  file_legal() ;
    FILE  *fopen(), *fp  ;
    char  *get_row(), buf[MAXLEN] ;

    if( tot_len == 0 ) {
        if( firstrow() OR prev_row->eoln ) {

            /* Input filename                             */
            clr_msg() ;
            pr_msg( INSERT, NULL ) ;
            if( input( getchar(), buf, file_legal ) == ABORTED )
                return ;

            /* Check if file is readable                  */
            if( NOT readable( buf ) ) return ;
            if( ( fp = fopen( buf, "r" ) ) == NULL ) {
                pr_msg( NO_XST, buf ) ;
                return( FALSE ) ;
            }

            /* Insert the file                            */
            tmp_row = prev_row ;
            readfile( fp ) ;
            fclose( fp ) ;
            prev_row = tmp_row ;
            if( prev_row->next != NULL ) {
                tot_len = prev_row->next->length ;
                eoln = prev_row->next->eoln ;
                cpystr( get_row( prev_row->next ) ,
                        tot_len ,
                        row_buf ) ;
            }
            ctrl_L() ;
            return ;
        }
    }
    else {
        pr_msg( NO_INS, NULL ) ;
        return( FALSE ) ;
    }
}

/***********************************************************
Scratch() : deletes all rows in the textbuffer
***********************************************************/

scratch( rp )
struct row_p *rp ;
{
    while( rp->next != NULL ) {
        get_row( rp->next ) ;
    }
}

/***********************************************************
Readfile() : reads rows from file fp  and inserts them after
the prev_row.
***********************************************************/

readfile( fp )
FILE *fp ;
{
    struct row_p *ins_text() ;
    int chr, i, n ;

    n = 0 ;
    while( ( chr = getc( fp ) ) != EOF ) {
        if( chr == '\n' ) {

            /* Insert row                                 */
            eoln = TRUE ;
            tot_len = n ;
            insert( row_buf, prev_row, tot_len, eoln ) ;
            prev_row = prev_row->next ;
            n = 0 ;
        }
        else if( chr == '\t' ) {
            for( i = n % 8 ; i < 8 ; i++ ) {
                if( n == tcols - 1 ) {
                    eoln = FALSE ;
                    tot_len = n ;
                    insert( row_buf, prev_row, tot_len, eoln ) ;
                    prev_row = prev_row->next ;
                    n = 0 ;
                }
                *( row_buf + n) = ' ' ;
                n++ ;
            }
        }
        else {
            if( n == tcols - 1 ) {

                /* Insert row                             */
                eoln = FALSE ;
                tot_len = n ;
                insert( row_buf, prev_row, tot_len, eoln ) ;
                prev_row = prev_row->next ;
                n = 0 ;
            }
            *(row_buf + n ) = chr ;
            n++ ;
        }
    }
    eoln = TRUE ;
    tot_len = n ;
    insert( row_buf, prev_row, tot_len, eoln ) ;
}

/***********************************************************
Savefile() : saves the whole textbuffer in the file fp.
***********************************************************/

savefile( fp )
FILE *fp ;
{
    struct row_p *rp, *ins_text() ;
    int i ;

    rp = &first_r ;

    /* Insert the current row                             */
    insert( row_buf, prev_row, tot_len, eoln ) ;
    while( TRUE ) {

        /* save  one row                                  */
        rp = rp->next ;
        for( i = 0 ; i < rp->length ; i++ ) {
            putc( *(rp->row + i ) , fp ) ;
        }
        if( rp->eoln ) {

            /* Check if more rows                         */
            if( rp->next == NULL ) break ;
            putc( '\n', fp ) ;
        }
    }

    /* No more rows, fetch the current row                */
    cpystr( get_row( prev_row->next ) ,
            tot_len ,
            row_buf ) ;
}

/***********************************************************
Readable() : checks if the file is readable i.e. :

               * That the file exists.
               * That it's not a directory.
               * That read permission exists.
               
***********************************************************/

readable( name )
char *name ;
{
    struct stat stbuf ;

    if( stat( name, &stbuf ) == -1 ) {
        pr_msg( NEW_FL, name ) ;
        return( TRUE ) ;
    }
    if( ( stbuf.st_mode & S_IFMT ) == S_IFDIR ) {
        pr_msg( NO_DIR, NULL ) ;
        return( FALSE ) ;
    }
    if( ( stbuf.st_mode & S_IREAD ) == S_IREAD ) {
        return( TRUE ) ;
    }
    pr_msg( NO_REA, NULL ) ;
    return( FALSE ) ;
}

/***********************************************************
Writeable() : checks if name is writeable, i.e. :

       * That the file exists.
       * That the file has the same user ID as the user.
       * That the file has write permission.

***********************************************************/

writeable( name )
char *name ;
{
    int  wfd ;

    if( access( name, 2 ) < 0 ) {
        if( errno == ENOENT ) {

            /* Create a new file                             */
            if( ( wfd = creat( name, PMODE ) ) == -1 ) {
                pr_msg( NO_CRT, name ) ;
                return( FALSE ) ;
            }
            close( wfd ) ;
            return( TRUE ) ;
        }
        else {
            pr_msg( NO_WRT, NULL ) ;
            return( FALSE ) ;
        }
    }
    if( FAILED bckup_file( name ) ) {
        pr_msg( NO_BAK, name ) ;
/*      allow exit even if backup failed        */
    }
    return( TRUE ) ;
}

/***********************************************************
Bckup_file() : make cp <name> <name>.BAK
***********************************************************/

bckup_file( name )
char *name ;
{
    FILE *fopen(), *inp, *outp ;
    char chr, *strcat(), buf[MAXLEN] ;

    if( strlen( name ) > 35 ) return( TRUE ) ;
    strcpy( buf, name ) ;
    if( is_bak( buf ) ) return( TRUE ) ;
    if( ( inp = fopen( buf, "r" ) ) == NULL ) {
        return( FALSE ) ;
    }
    if( ( outp = fopen( strcat( buf, ".BAK" ) , "w" ) ) == NULL ) {
        return( FALSE ) ;
    }
    while( ( chr = getc( inp ) ) != EOF ) {
        putc( chr, outp ) ;
    }
    fclose( inp ) ;
    fclose( outp ) ;
    return( TRUE ) ;
}

is_bak( name )
char *name ;
{
    while( *name ) {
        if( *name == '.' ) {
            if( strcmp( ++name, "BAK" ) == 0 ) {
                return( TRUE ) ;
            }
            else continue ;
        }
        name++ ;
    }
    return( FALSE ) ;
}
    
/***********************************************************
Info() : prints information on the file you're editing. 
First the filename then the total number of characters and
finally the number of characters before the cursor.
***********************************************************/

info()
{
    long pos, total ;
    struct row_p *rp ;
    FLAG found ;

    pos = 1 ;
    total = 0 ;
    found = FALSE ;
    insert( row_buf, prev_row, tot_len, eoln ) ;
    prev_row = prev_row->next ;
    rp = first_r.next ;
    while( rp != NULL ) {
        if( rp == prev_row ) {
            pos = pos + col ;
            found = TRUE ;
        }
        if( NOT found ) pos = pos + rp->length + rp->eoln ;
        total = total + rp->length + rp->eoln ;
        rp = rp->next ;
    }
    clr_msg() ;
    cursor( trows - 1, 0 ) ;
    printf( "%s    %ld    %ld" , f_name, total, pos ) ;
    get_prev() ;
}
