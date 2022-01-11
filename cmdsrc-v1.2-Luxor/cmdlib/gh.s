	.globl	_setgrent,_setpwent,_getgrgid,_getpwuid
	.globl	_getgrnam,_getpwnam,_localtime,_timezone
	.globl	_endpwent
	.globl	_setgren,_setpwen,_getgrgi,_getpwui
	.globl	_getgrna,_getpwna,_localti,_timezon
	.globl	_endpwen
	.text
_setgrent: jmp	_setgren
_setpwent: jmp  _setpwen
_getgrgid: jmp  _getgrgi
_getpwuid: jmp  _getpwui
_getgrnam: jmp  _getgrna
_getpwnam: jmp  _getpwna
_localtime: jmp _localti
_timezone: jmp _timezon
_endpwent: jmp _endpwen
	.end
