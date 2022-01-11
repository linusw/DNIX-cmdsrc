/*
*	ABCenix shell
*
*	======================================
*
*	== errmess.h ==		error messages
*
*	======================================
*
*	Per-Arne Svensson
*	Dataindustrier AB DIAB
*	T[BY	84-05-10
*/
char	*ER_SYNT	= "syntax error: ";
char	*ER_BOP		= ": bad option";
char	*ER_CNEX	= ": cannot execute";
char	*ER_UNXP	= "unexpected.";
char	*ER_CNOP	= ": cannot open";
char	*ER_NL		= "newline";
char	*ER_ILLIO	= "Illigal io";
char*	ER_EOF		= "end of file";
char	*ER_NFND	= ": not found";
char	*ER_ILLVAR	= ": not an identifier";
char	*ER_BNUM	= ": bad number";
char	*ER_PNSET	= "parameter not set";
char	*ER_CNSH	= "cannot shift";
char	*ER_RDONLY	= " is readonly";
char	*ER_2BIG	= "too big";
char	*ER_ARGL2LNG	= "arglist too long";
char	*ER_TXTBSY	= "text busy";
char	*ER_CORED	= " - core dumped";
char	*ER_PTRACE	= "ptrace: ";

/*	System messages, (result from waiting on a process)		*/
char	*sys_msg[] = {
		0,
		"Hangup",
		0,	/*	Interrupt	*/
		"Quit",
		"Illegal instruction",
		"Trace/BPT trap",
		"IOT trap",
		"EMT trap",
		"Floating exception",
		"Killed",
		"Bus error",
		"Memory fault",
		"Bad system call",
		0,	/*	Broken pipe	*/
		"Alarm call",
		"Terminated",
		"Signal 16"
};
