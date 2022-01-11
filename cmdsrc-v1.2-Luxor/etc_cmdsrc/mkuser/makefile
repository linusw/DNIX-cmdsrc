#
#  Makefile for the mkuser command.
#

SOURCES = accline.c chkint.c chkpath.c chkstr.c clowstr.c cmderr.c \
	emptybuf.c emptychk.c freegid.c freeuid.c getdescr.c getgroup.c \
	getinid.c getline.c getpw.c getshell.c getunam.c getusrid.c \
	mkusrcmd.c defs.h msg.h makedir.c flincat.c fcopy.c \
	flinread.c flinwrit.c chname.c

OBJECTS = mkusrcmd.o accline.o chkint.o chkpath.o chkstr.o clowstr.o \
	cmderr.o emptybuf.o emptychk.o freegid.o freeuid.o getdescr.o \
	getgroup.o getinid.o getline.o getpw.o getshell.o getunam.o \
	getusrid.o makedir.o flincat.o fcopy.o flinread.o flinwrit.o \
	chname.o

OBJDEF = accline.o getpw.o getunam.o mkusrcmd.o makedir.o flincat.o \
	flinread.o

OBJMSG = accline.o emptychk.o getdescr.o getgroup.o getinid.o getpw.o \
	getshell.o getunam.o getusrid.o mkusrcmd.o

LIBES =

CFLAGS = -O


mkuser: $(OBJECTS)
	cc $(CFLAGS)  $(OBJECTS) $(LIBES) -o mkuser

install:
	cp mkuser ../../etc
	strip ../../etc/mkuser

$(OBJDEF): defs.h
$(OBJMSG): msg.h

#
#  Print modified files.
#

print: $(SOURCES) makefile
	pr $? >/dev/lp
	touch print		# Update date of dummy file

#
#  Update archive containing source files.
#

arch:
	ar uv mkuser.a  $(SOURCES) makefile
