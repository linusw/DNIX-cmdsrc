/*
 * Put line numbers on a list
 */
char eject[] = "/*eject*/";
main()
{
	register int i,j,c,c1,cnt;
	char a[10];
	cnt = 0;
	c = '\n';
	a[9] = '\0';
	while((c1 = getchar()) != -1) {
		putchar(c);
		for(j = 0; j < 9; j++) {
			a[j] = a[j+1];
		}
		a[8] = c;
		if(c == '/') {
			if(strcmp(a,eject) == 0) {
				putchar('\014');
			}
		}
		if(c == '\n') printf("%4d\t",++cnt);
		c = c1;
	}
}
