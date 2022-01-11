#define  FLAG      int          /* boolean                     */
#define  TRUE      1            /* boolean value TRUE          */
#define  FALSE     0            /* boolean value FALSE         */

/****************************************************************
                    Control-characters 
****************************************************************/

#define MARK           0           /* set mark                 */
#define CTRL_A         1           /* beginning of line        */
#define CTRL_B         2           /* backward character       */
#define CTRL_C         3           /* exit                     */
#define CTRL_D         4           /* delete next character    */
#define CTRL_E         5           /* end of line              */
#define CTRL_F         6           /* forward character        */
#define CTRL_G         7           /* abort                    */
#define CTRL_H         8           /* delete previous char     */
#define CTRL_I         9           /* tab                      */
#define CTRL_J        10           /* line feed                */
#define CTRL_K        11           /* kill to end of line      */
#define CTRL_L        12           /* redraw window            */
#define CTRL_M        13           /* newline                  */
#define CTRL_N        14           /* next line                */
#define CTRL_O        15           /* insert line              */
#define CTRL_P        16           /* previous line            */
#define CTRL_Q        17           /*                          */
#define CTRL_R        18           /* search reverse           */
#define CTRL_S        19           /*                          */
#define CTRL_T        20           /* transpose character      */
#define CTRL_U        21           /* argument prefix          */
#define CTRL_V        22           /* next page                */
#define CTRL_W        23           /* delete to kill buffer    */
#define CTRL_X        24           /* prefix                   */
#define CTRL_Y        25           /* yank from kill-buffer    */
#define CTRL_Z        26           /* screen scroll up         */
#define CTRL_OE       28           /* quote character          */
#define CTRL_AA       29           /* search forward           */
#define DEL          127           /* delete                   */
#define ESC           27           /* prefix                   */
#define TOP_F        '<'           /* top of file              */
#define BCK_W        'V'           /* previous window          */
#define END_F        '>'           /* end of file              */
#define QUERY        'Q'           /* query replace            */

/****************************************************************
Messages and error-codes...
****************************************************************/

#define NO_ERR       0          /* No error                    */
#define TOP_FL       1          /* Top of file                 */
#define END_FL       2          /* End of file                 */
#define NO_IMP       3          /* Not implemented             */
#define XIT_ED       4          /* Exit editor                 */
#define Q_HELP       5          /* Query help                  */
#define REPLCE       6          /* Number of replaced strings  */
#define SEARCH       7          /* String to search for        */
#define ABORTD       8          /* Aborted                     */
#define NO_FND       9          /* Can't find string           */
#define OLD_ST       10         /* Old string to replace       */
#define NEW_ST       11         /* New string to replace with  */
#define READ_F       12         /* Read filename               */
#define INSERT       13         /* Read file to insert         */
#define NO_SAV       14         /* Couldn't save file          */
#define WRTE_F       15         /* Wrote file                  */
#define NO_XST       16         /* Nonexistent file            */
#define NO_INS       17         /* Can't insert                */
#define NEW_FL       18         /* New file                    */
#define NO_DIR       19         /* Can't edit directories      */
#define NO_REA       20         /* No read permission          */
#define NO_CRT       21         /* Can't create file           */
#define NO_WRT       22         /* No write permission         */
#define NO_BAK       23         /* Can't make .BAK-file        */
#define BIGARG       24         /* Argument's too large        */
#define ARGMNT       25         /* Argument                    */

/****************************************************************
Error-codes in the "term.c" file.
****************************************************************/

#define OK           0 
#define NOMEM        1 
#define NOTERM       2 
#define NOFILE       3 
#define NOENTRY      4 
#define NOTSUPPORTED 5

#define NOT          !
#define FAILED       !  
#define AND          &&
#define OR           ||
#define TO           ,

#define PMODE        0644    /* fileprotection rw-r-r         */
#define READ         0
#define WRITE        1

#define ABORTED      0
#define CONT         1
#define NOTEQ        0
#define EQUAL        1
#define MAXLEN       40
#define NOTSET       (-1)
#define MAXARG       500

#define ENGLISH      0
#define SWEDISH      1



#define DDT          1
