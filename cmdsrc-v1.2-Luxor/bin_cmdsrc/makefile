CC	= cc
CFLAGS	= -O -n -s
LIBES	= ../cmdlib/lib
TARGET	= ../bin/
OBJECTS	=	$(TARGET)basename\
		$(TARGET)cat\
		$(TARGET)chgrp\
		$(TARGET)chmod\
		$(TARGET)chown\
		$(TARGET)cmp\
		$(TARGET)copy\
		$(TARGET)cp\
		$(TARGET)date\
		$(TARGET)dirname\
		$(TARGET)dd\
		$(TARGET)df\
		$(TARGET)du\
		$(TARGET)echo\
		$(TARGET)expr\
		$(TARGET)find\
		$(TARGET)fscl\
		$(TARGET)kill\
		$(TARGET)ln\
		$(TARGET)login\
		$(TARGET)lpd\
		$(TARGET)lpr\
		$(TARGET)ls\
		$(TARGET)mkdir\
		$(TARGET)mv\
		$(TARGET)nice\
		$(TARGET)num\
		$(TARGET)od-1\
		$(TARGET)od\
		$(TARGET)passwd\
		$(TARGET)pr\
		$(TARGET)ps\
		$(TARGET)queue\
		$(TARGET)rm\
		$(TARGET)rmdir\
		$(TARGET)sleep\
		$(TARGET)stty\
		$(TARGET)su\
		$(TARGET)sync\
		$(TARGET)test\
		$(TARGET)time\
		$(TARGET)touch\
		$(TARGET)tty\
		$(TARGET)unlink\
		$(TARGET)version\
		$(TARGET)who\
		$(TARGET)write

make: $(OBJECTS) ;
	cd sh; make
	cd fsck; make
	cd siv; make

simp:	$(OBJECTS);

$(TARGET)basename:	basename.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)cat: cat.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)chgrp: chgrp.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)chmod: chmod.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)chown: chown.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)cmp: cmp.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)copy: copy.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)cp: cp.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)date: date.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)dirname: dirname.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)dd: dd.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)df: df.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)du: du.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)echo: echo.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)expr: expr.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)find: find.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)fscl: fscl.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)kill: kill.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)ln: ln.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)login: login.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)lpd: lpd.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)lpr: lpr.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)ls: ls.c
	$(CC) -O -s $? $(LIBES) -o $@

$(TARGET)mv: mv.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)mkdir: mkdir.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)nice: nice.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)num: num.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)od: od.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)od-1: od-1.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)passwd:	passwd.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)pr: pr.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)ps: ps.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)queue: queue.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)rm: rm.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)rmdir: rmdir.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)sleep: sleep.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)stty: stty.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)su: su.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)sync: sync.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)test: test.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)time: time.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)touch: touch.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)tty: tty.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)unlink: unlink.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)version: version.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)who: who.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@
$(TARGET)write: write.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@
