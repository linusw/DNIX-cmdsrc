CC=cc
CFLAGS=
SOURCE = tar.c io.c help.c

tar:	lib_obj
	$(CC) $(CFLAGS) lib_obj lib_obj -n -o tar

lib_obj:	$(SOURCE)
	$(CC) $(CFLAGS) -c $?
	echo ar rv lib_obj $? | sed s/\\.c/\\.o/g | sh -v
	rm *.o

