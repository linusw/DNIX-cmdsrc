
#include <stdio.h>
#include "define.h"         /* constants etc                     */
#include "struct.h"
#include "global.h"         /* global definitions                */


main( argc, argv )
int argc ;
char * argv[] ;
{
    initterm() ;
    set_trap() ;
    clrscr() ;
    if( argc > 1 )
        edit( argv[1] ) ;
    else
        edit( NULL ) ;
}


    
    
    
        

                       

           
 







        
