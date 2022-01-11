NAME	= makefile
CC	= cc
CFLAGS	= -DLIB -O
LIBES	= lib

$(LIBES): *.[cs]
	echo make -f $(NAME) $? | sed s/\\.[cs]/\\.o/g | sh -v
	ar rv $(LIBES) *.o
	ranlib $(LIBES)
	rm *.o
