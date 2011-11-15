/* 
   sorb.c - Routines to initialize and read a 3D game controller through
            the serial port.

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
   is under GPL.

   The rest of the code is written by Brett Viren
   (Brett.Viren@sunysb.edu) on his free time (ie, not when on the pay
   clock of SUNY@Stony Brook). Do with it what you will, as long as
   you abide by the GPL.

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>

#include "sorb.h"

static int sorb_trans_thresh[3] = {0,0,0};
static int sorb_rot_thresh[3] = {0,0,0};

/* Private utility functions */
static int readn(int fd, char *buf, int nbuf);

/* Open the orb with correct flags. Returns file desc. or -1 w/ errno set. */
int sorb_open(char *sorb_dev)
{
    int fp = open (sorb_dev,SORB_OPEN);
    if (fp < 0) {
	perror("sorb_open()");
	exit(EXIT_FAILURE);
    }
    else return fp;
}


/* Initialize the orb. This sets up the serial port. */
void sorb_init(int fd)
{
    struct termios t;

    tcgetattr(fd, &t);

    /* Set the baud rate */
    cfsetospeed(&t, B9600);
    cfsetispeed(&t, B9600);

    /* Set character size to 8bits. This will get masked to 7bits later */
    t.c_cflag = (t.c_cflag & ~CSIZE) | CS8;

    /* This is ripped strait from minicom's sysdep1.c. Thanks! */
    t.c_iflag = IGNBRK;
    t.c_lflag = 0;
    t.c_oflag = 0;
    t.c_cflag |= CLOCAL | CREAD;
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 5;
    t.c_iflag |= IXON|IXOFF;

    /* Mask off parity */
    t.c_cflag &= ~(PARENB|PARODD);

    /* Set the serial attributes */
    tcsetattr(fd, TCSANOW, &t);
}

/* Read initial startup packet */
int sorb_init_read(int fd, char *buf, int nbuf)
{
    int r=0,i,n;
    for (i = 0; i < SORB_GREETING_SIZE; ++i) {
	n = read(fd,buf+i,1);
	if (n < 0) {
	    perror("sorb_init_read()");
	    return n;
	}
	buf[i] &= 0177;
	if (buf[i] == '\r') buf[i] = '\n';
	r += n;
    }
    return r;
}


/* Read a data packet from the orb */
int sorb_read(int fd, char *buf, int nbuf)
{
    int n;
    char c;

    /* Get first bit */
    if ((n = read (fd, &c, 1)) <0) return n;

    /* Button press/release w/out motion */
    if (c == SORB_BUTTON_NUMBER) {
	buf[0] = c;
	return 1 + readn(fd,buf+1,SORB_BUTTON_SIZE-1);
    }

    /* Button press/release with motion */
    else if (c == SORB_MOTION_NUMBER) {
	buf[0] = c;
	return 1 + readn(fd,buf+1,SORB_MOTION_SIZE-1);
    }

    /* Just a Carriage return - try again. */
    else if (c == 13 /* CR */) {
	return sorb_read(fd,buf,nbuf);
    }

    /* Huh? */
    else {
	fprintf(stderr,"Unknown packet number: %d\n",c);
	return 0;
    }
} /* sorb_read() */

/* Read exactly n bytes */
static int readn(int fd, char *buf, int nbuf)
{
    int i, n, r=0;
    for (i = 0; i < nbuf; i++) {
	n = read(fd,buf+i,1);	/* Read one byte at a time until done */
	if (n < 0) {
	    perror("readn()");
	    return n;
	}
	buf[i] &= 0177;		/* set MSB (parity bit) to zero */
	r += n;
    }
    return r;
}


/* Dump n chars as ints */
void sorb_decimal_print (char buf[], int n)
{
    int i;
    for (i = 0; i < n; ++i) {
	fprintf(stderr,"%4d ",buf[i]);
    }
    fprintf(stderr,"\n");
}

/*
  Convert 8 1/2 7bit bytes into 6 ten-bit integers  

    c[0]    c[1]    c[2]    c[3]    c[4]    c[5]    c[6]    c[7]    c[8]
xdddddddxdddddddxdddddddxdddddddxdddddddxdddddddxdddddddxdddddddxddddddd 
xdddddddxddd           dxdddddddxdd           ddxdddddddxd
      t[0]  ddddxdddddd       t[2] dddddxddddd      r[1]  ddddddxdddd ddd 
                  t[1]                   r[0]                   r[2]

*/
void sorb_bits_convert(char c[9], int t[3], int r[3])
{
    int i;
    char SpaceWare[] = "SpaceWare!";

    /* Strip the MSB, which is a parity bit */
    for (i = 0; i < 9; ++i) {
	c[i] &= 0177;		/* Make sure everything is 7bit */
	c[i] ^= SpaceWare[i];	/* What's this doing in the data? */
    }

    /* Turn chars into 10 bit integers */
    t[0] = ((c[0] & 0177)<<3)|((c[1] & 0160)>>4);
    t[1] = ((c[1] & 0017)<<6)|((c[2] & 0176)>>1);
    t[2] = ((c[2] & 0001)<<9)|((c[3] & 0177)<<2)|((c[4] & 0140)>>5);
    r[0] = ((c[4] & 0037)<<5)|((c[5] & 0174)>>2);
    r[1] = ((c[5] & 0003)<<8)|((c[6] & 0177)<<1)|((c[7] & 0100)>>6);
    r[2] = ((c[7] & 0077)<<4)|((c[8] & 0170)>>3);

    /* Get the sign right. */
    for(i = 0; i < 3; i ++) {
	if (t[i] > 511) t[i] -= 1024;
	if (fabs(t[i]) < sorb_trans_thresh[i]) t[i] = 0;
    }
    for(i = 0; i < 3; i ++) {
	if (r[i] > 511) r[i] -= 1024;
	if (fabs(r[i]) < sorb_rot_thresh[i]) r[i] = 0;
    }
}

void sorb_set_thresh(float t[3], float r[3])
{
    int i;
    for (i = 0; i < 3; ++i) {
	sorb_trans_thresh[i] = (int)(512*t[i]);
	sorb_rot_thresh[i] = (int)(512*r[i]);
    }
}
