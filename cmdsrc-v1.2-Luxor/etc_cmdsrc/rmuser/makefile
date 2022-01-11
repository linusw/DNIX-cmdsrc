#
#  Makefile for the rmuser command.
#

SOURCES = rmusrcmd.c chkstr.c clowstr.c cmderr.c emptybuf.c flinread.c \
	flinwrit.c getline.c getusr.c defs.h msg.h chname.c \
	deldir.c dirempck.c

OBJECTS = rmusrcmd.o chkstr.o clowstr.o cmderr.o emptybuf.o flinread.o \
	flinwrit.o getline.o getusr.o chname.o deldir.o dirempck.o

OBJDEF = rmusrcmd.o getusr.o dirempck.o

OBJMSG = rmusrcmd.o getusr.o

LIBES =

CFLAGS = -O


mkuser: $(OBJECTS)
	cc $(CFLAGS)  $(OBJECTS) $(LIBES) -o rmuser

install:
	cp rmuser ../../etc
	strip ../../etc/rmuser

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
	ar uv rmuser.a  $(SOURCES) makefile
