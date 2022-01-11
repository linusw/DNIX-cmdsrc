/*
*	ABCenix shell
*
*	Peter Helfer
*	Watchdog Computer Assistance
*
*	==================================
*
*	== sym.h ==	Symbol definitions
*
*	==================================
*
*	Edit 012	1983-07-11
*	84-09-07:	Changed some char constants to signed since chars
*			are declared as signed quantitys.
*/

/*  characters  */

#define C_SP	' '
#define C_NL	'\n'
#define	C_EOF	(char)-1
#define C_NUL	'\0'
#define C_TAB	'\t'
#define C_EXCL	'!'
#define C_AMP	'&'
#define C_SEMI	';'
#define C_GT	'>'
#define C_LT	'<'
#define C_EQU	'='

/*  Some special characters  */

#define C_LVARX		ldpchar[0]	/*  Starts variable expression           */
#define C_RVARX		ldpchar[1]	/*  Ends variable expression             */
#define	C_PATH		'/'		/*  Separates file names in path name    */
#define	C_PATHLIM	':'		/*  Separates pathnames in $PATH         */
#define	C_WDIR		'.'		/*  Current working directory            */
#define	C_VAR		'$'		/*  Starts reference to shell variable   */
#define	C_QUOTE		ldpchar[2]	/*  Qoute next char                      */
#define	C_CMDSUB	ldpchar[5]	/*  Command substitution                 */
#define	C_SQUOT		'\''		/*  Quote until next C_SQUOT             */
#define	C_DQUOT		'"'		/*  Quote, but do command and parameter  */
/*					/*  substitution, except when C_QUOTEd   */
#define	QUOTE		-128		/*  To be ORed with quoted characters    */
#define	UNQUOTE		127		/*  To AND with to unquote               */

/* Reserved word codes  */

#define R_LGR		1	/*  left command grouper '('  */
#define R_RGR		2	/*  right d:o            ')'  */
#define R_BGN		3	/*  'begin'  */
#define R_END		4	/*  'end'    */
#define R_FOR		5	/*  'for'    */
#define R_WHI		6	/*  'while'  */
#define R_UNT		7	/*  'until'  */
#define R_DO		8	/*  'do'     */
#define R_DONE		9	/*  'done'   */
#define R_CAS		10	/*  'case'   */
#define R_IN		11	/*  'in'     */
#define R_ESAC		12	/*  'esac'   */
#define R_IF		13	/*  'if'     */
#define R_THEN		14	/*  'then'   */
#define R_ELSE		15	/*  'else'   */
#define R_ELIF		16	/*  'elif'   */
#define R_FI		17	/*  'fi'     */
#define R_PIPE		18	/*  '!'      */
#define R_BACK		19	/*  '&'      */
#define R_SEP		20	/*  ';'      */
#define R_ORF		21	/*  '!!'     */
#define R_ANDF		22	/*  '&&'     */
#define R_HDOC		23	/*  '<<'     */
#define R_APP		24	/*  '>>'     */
#define R_INP		25	/*  '<'      */
#define R_OUT		26	/*  '>'      */
#define R_INDUP		27	/*  '<&'     */
#define R_OUTDUP	28	/*  '>&'     */
#define	R_CLINP		29	/*  '<&-'    */
#define	R_CLOUT		30	/*  '>&-'    */
#define	R_NL		31	/*  newline  */
#define	R_EOF		32	/*  end of file  */
#define	R_ENDCAS	33	/*  ';;' terminates case-part  */

/* Internal command codes */

#define	I_NOP		1	/*  ':'		*/
#define	I_PARSEX	2	/*  '.'		*/
#define	I_BREAK		3	/*  'break'	*/
#define	I_CONT		4	/*  'continue'	*/
#define	I_CD		5	/*  'cd'	*/
#define	I_EVAL		6	/*  'eval'	*/
#define	I_EXEC		7	/*  'exec'	*/
#define	I_EXIT		8	/*  'exit'	*/
#define	I_EXPORT	9	/*  'export'	*/
#define	I_LOGIN		10	/*  'login'	*/
#define	I_NEWGRP	11	/*  'newgrp'	*/
#define	I_READ		12	/*  'read'	*/
#define	I_RDONLY	13	/*  'readonly'	*/
#define I_SET		14	/*  'set'	*/
#define	I_SHIFT		15	/*  'shift'	*/
#define	I_TIMES		16	/*  'times'	*/
#define	I_TRAP		17	/*  'trap'	*/
#define	I_UMASK		18	/*  'umask'	*/
#define	I_WAIT		19	/*  'wait'	*/
#define	I_SWE		20	/*  'swedish'	*/
#define	I_ENG		21	/*  'english'	*/

/* wildcard characters  */

#define WC_CHAR	'?'		/*  wild character  */
#define WC_STR	'*'		/*  wildstring  */
#define WC_LPAR	ldpchar[3]	/*  left parentheses enclosing alternatives  */
#define WC_RPAR	ldpchar[4]	/*  right d:o  */
#define	WC_SEQ	'-'		/*  any of a sequence of characters  */

/*  shell variable names and initial values  */

#define	PRIMVAR	"PS1"		/*  primary cursor  */
#define PRIMSINI "# "		/*  used if su      */
#define	PRIMINI	"$ "
#define	SECVAR	"PS2"		/*  secondary cursor  */
#define	SECINI	"> "
#define	PATHVAR	"PATH"		/*  directory search path  */
#define	PATHINI	":/bin:/usr/bin"
#define	LASTVAR	"?"		/*  exit status of last command  */
#define	LASTINI	"0"
#define	PARVAR	"#"		/*  no of positional parameters  */
#define	PARINI	"0"
#define LANGVAR "LANGUAGE"	/*  Language - swedish or english */
#define LANGINI "english"
#define	PROCVAR	"$"		/*  process no of this shell  */
#define	BAKVAR	"!"		/*  process no of last background process  */
#define	BAKINI	""
#define	FLAGVAR	"-"		/*  shell flags  */
#define	ALLVAR	"@"		/*  a string of all shell parameters  */
#define	STARVAR	"*"		/*  same as ALLVAR  */
#define	MAILVAR	"MAIL"		/*  name of mail file  */
#define	HOMEVAR	"HOME"		/*  default arg for cd command  */
#define	BLNKVAR	"IFS"		/*  chars used by blank interpretation  */
#define	BLNKINI	" \t\n"	

#define	SPECVAR	"?#$!@*-"			/*  In addition to regular variable names  */
/*						/*  (letter+letter/digit...), variables    */
/*						/*  with these one character names can be  */
/*						/*  evaluated (but not set with '='.       */
