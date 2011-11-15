/* 
   sorbT.c - test routines to read a 3D game controller through the
             serial port 


    Copyright (C) 1997  Brett Viren

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   ---- end copyleft notice ----

   This code was developed with help from many, but no help from a
   company known as SpaceTec which happens to make a 3D game controller
   called the SpaceBall 360 which if you connect to your serial port
   and run this code, meaninful numbers will just happen to be produced. 
   Wow, what a coincidence.

   This code must be under the GPL as it has code from minicom which
   is under GPL (thanks!).

   The rest of the code is written by Brett Viren (Brett.Viren@sunysb.edu)
   on his free time. Do with it what you will, as long as you abide by
   the GPL.

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>

#include "sorb.h"

extern char *optarg;
extern int optind, opterr, optopt;

void main(int argc, char **argv)
{
    char buf[1024];
    int i, n, nbufs=1024, t[3], r[3];
    int orbfd;
    char orbdev[1024] = SORB_DEF_DEV;
    float rot_thresh[3] = {0,0,0}, trans_thresh[3] = {0,0,0};
    int status, print_raw = 0;;

    while ((status=getopt(argc,argv,"r:t:d:R")) != EOF) {
        switch (status) {
        case 't':		/* Set translational threshold */
	    sscanf(optarg,"%f %f %f",
		   trans_thresh,trans_thresh+1,trans_thresh+2);
	    break;
	case 'r':		/* Set rotational threshold */
	    sscanf(optarg,"%f %f %f",
		   rot_thresh,rot_thresh+1,rot_thresh+2);
	    break;
	case 'd':		/* Set the a device other than the def. */
	    strcpy(orbdev,optarg);
	    break;
	case 'R':		/* Set raw print mode for motion packets */
	    print_raw = 1;
	    break;
	}
    }

    orbfd = sorb_open(orbdev);

    sorb_set_thresh(trans_thresh,rot_thresh);

    sorb_init(orbfd);

    n = sorb_init_read(orbfd,buf,nbufs);

    fprintf(stderr,"Hardware mesg: ");
    for (i = 0; i < n; ++i) fprintf(stderr,"%c",buf[i]);
    fprintf(stderr,"\n");
    fprintf(stderr,
	    "Software mesg: Information is free, use it as you wish.\n\n");

    while (1) {

	n = sorb_read(orbfd,buf,nbufs);

	/* button press/release */
	if (buf[0] == 75)
	    sorb_decimal_print(buf,n);

	/* motion */
	if (buf[0] == 68) {
	    if (print_raw) {
		int i;
		for (i = 0; i < 12; ++i) fprintf(stderr,"%d ",(int)buf[i]);
		fprintf(stderr,"\n");
	    }
	    else {
		sorb_bits_convert(buf+2,t,r);
		fprintf(stderr,"%d %2d (%4d %4d %4d) --> (%4d %4d %4d) %4d\n",
			buf[0],buf[1],t[0],t[1],t[2],r[0],r[1],r[2],
			buf[11]^'!');
	    }
	}
    }
}



