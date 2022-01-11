/***********************************************************

  * * *  S T A N D A R D S  &  C O N V E N T I O N S * * *

  <1> A ROW is a string of characters of maxsize (tcols-1).
      I.e. one row on the CRT.
  <2> A LINE is the string between two newlines.
  <3> The row that holds the cursor is called the CURRENT
      ROW, and the cursor position is called the CURRENT
      POSITION. 
  <4> The text is stored in a number of buffers called
      TEXT-BUFFERS. These are created by calls to malloc().
  <5> Pointers to every row are stored in areas called
      "ROW_P"-BUFFERS. See fig in <struct.c>.
  <6> Pointers to nonused areas in the text-buffer are
      stored in the "FREE_P"-BUFFERS. See fig in <struct.c>.
  <7> What you see on the screen abowe the final two rows
      is called the WINDOW.

***********************************************************/


/* Terminal parameters                                    */

int   tcols ;	   /* no of columns supported by tty      */
int   trows ;      /*  -"-  rows         -"-              */
int   wdw_size ;   /* size of window, i.e. trows - 2      */

/* window parameters                                      */

int   row ;        /* cursors row position                */
int   col ;        /* cursors column position             */
 
/* various pointers to the "free_p"-block                 */

struct free_p first_f ;     /* pointer to the first "free"
                            area in the text-buffer       */
struct free_p *free_f ;     /* pointer to a list of "free"
                            free_p:s, i.e. free_p's that are
			    not currently used.           */
struct free_p dummy ;       /* dummy-object ending the
                            first_f-list.                 */

/* various pointers to the "row_p"-block                 */

struct row_p first_r ;     /* pointer to the row_p that
                            points to the first row.     */
struct row_p *prev_row ;   /* points to the row before the
                           current row.                  */
struct row_p *free_r ;     /* points to a list of "free"
                            row_p:s, i.e. row_p's that can
                            be used for new rows.        */

/* various pointers to row-buffers                       */

char *row_buf ;            /* The row that holds the 
                            cursor is put in this buffer  */
char *tmp_buf ;             /* Temporary row_buffer.     */
char *chr_p ;               /* Points to the current
                            position in the current row  */
int tot_len ;               /* total length of the row in
                            the row-buffer.              */
int eoln ;			    

/* pointers etc to the kill buffer.                       */

struct row_p kill_buf ;     /* pointer to kill-buffer.    */

/* Position of the mark                                   */

long mark ;                 /* position of mark           */

/* Misc.                                                  */

char *end_wdw ;             /* The dotted line under the
                            window.                       */
char *space ;               /* Blank line                 */

char f_name[40] ;           /* filename                   */
char search[40] ;           /* string to search for       */
char replce[40] ;           /* string to replace with     */
char prev_cmnd  ;           /* the previous command       */
char cmnd       ;           /* current command            */

int  language   ;           /* 0 = english, 1 = swedish   */
