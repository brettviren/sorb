/* 
   sorb.h - header file for sorb.c


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

#ifndef SORB_H_SEEN
#define SORB_H_SEEN

#define SORB_DEF_DEV "/dev/ttyS0"

#define SORB_GREETING_SIZE 53	/* The packet size of initial packet */
#define SORB_BUTTON_SIZE 5	/* The packet size for no motion */
				/* (not counting CR)*/
#define SORB_BUTTON_NUMBER 75	/* The flag for a no-motion-packet */
#define SORB_MOTION_SIZE 12	/* The packet size for motion */
				/* (not counting CR)*/
#define SORB_MOTION_NUMBER 68	/* The flag for a motion-packet */


/* Flags for open() */
#define SORB_OPEN O_RDWR|O_NOCTTY

/* Open orb's device */
int sorb_open(char *sorb_dev);

/* Initialize serial port */
void sorb_init(int fd);

/* Read one time only, initial packets */
int sorb_init_read(int fd, char *buf, int nbuf);

/* Read one packet. First byte says what kind of packet was read. */
int sorb_read(int fd, char *buf, int nbuf);

/* Print the buffer as decimal numbers. */
void sorb_decimal_print (char buf[], int n);

/* Convert from bits to translations and rotation */
void sorb_bits_convert(char c[], int t[], int r[]);

/* Set discriminator-like threshold. Axis must be more than this percent
 * to be non-zero */
void sorb_set_thresh(float t[3], float r[3]);

#endif /* SORB_H_SEEN */
