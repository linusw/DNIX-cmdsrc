CC	= cc
CFLAGS	= -O -i -s
LIBES	= ../cmdlib/lib
TARGET	= ../etc/
OBJECTS	=	$(TARGET)init\
		$(TARGET)mknod\
		$(TARGET)mount\
		$(TARGET)setbclock\
		$(TARGET)update\
		$(TARGET)timezone\
		$(TARGET)wall

make: $(OBJECTS) ;
	cd mkuser; make
	cd rmuser; make
	cd mkfs; make

$(TARGET)init: init.c
	$(CC) -O -s $? $(LIBES) -o $@

$(TARGET)mknod: mknod.c
	$(CC) -O -s $? $(LIBES) -o $@

$(TARGET)mount: mount.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)passwd: passwd.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)setbclock:	setbclock.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)update: update.c
	$(CC) -O -s $? -o $@

$(TARGET)wall: wall.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@

$(TARGET)timezone:	timezone.c
	$(CC) $(CFLAGS) $? $(LIBES) -o $@
