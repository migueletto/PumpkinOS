/*
 * mio.c - Colin Kelley  1-18-87
 *   routines to emulate temporary file handling with memory instead
 *
 */

#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MALLOC_SIZE 10000

static unsigned char *mhead;		/* pointer to start of malloc()d area */
static unsigned char *mend;			/* pointer to current (just beyond) EOF*/
static unsigned char *mptr;			/* pointer to current position */
static unsigned int msize;			/* size of chunk mhead points to */

FILE *
mfopen(filename,mode)
char *filename,*mode;
{
	if ((mhead = malloc(MALLOC_SIZE)) == 0) {
		msize = 0;
		return (0);
	}
	msize = MALLOC_SIZE;
	mend = mptr = mhead;
	return ((FILE *)1);				/* not used */
}

int
mfclose(f)
FILE *f;
{
	if (mhead) {
		free(mhead);
		return (0);
	}
	else
		return (-1);
}

unsigned int
mfputc(c,f)
unsigned int c;
FILE *f;
{
register unsigned char *p;
	while (mptr >= mhead + msize) {
		if ((p = realloc(mhead,msize+MALLOC_SIZE)) == (unsigned char *)-1) {
			fputs("mio: out of memory\n",stderr);
			return (-1);
		}
		else {
			msize += MALLOC_SIZE;
			mptr = (unsigned char *) (p + (unsigned int)(mptr - mhead));
			mhead = p;
		}
	}
	*mptr = c & 255;
	mend = ++mptr;
	return c;
}

unsigned int
mfgetc(f)
FILE *f;
{
	if (mptr >= mend)		/* no characters left */
		return (-1);
	else
		return (*mptr++);
}

int
mfseek(f,loc,origin)
FILE *f;
long loc;
int origin;
{
	if (origin != 0) {
		fputs("mseek() only implemented with 0 origin",stderr);
		return (-1);
	}
	mptr = mhead + loc;
	return (0);
}

int
mfread(ptr, size, nitems,f)
char *ptr;
unsigned int size, nitems;
FILE *f;
{
register unsigned int i = 0;
	while (i < nitems) {
		if ((mptr + size) > mend)
			break;
		memmove(ptr,mptr,size); /*bcopy(mptr,ptr,size);*/
		ptr += size;
		mptr += size;
		i++;
	}
	return (i);
}

int
mfwrite(ptr, size, nitems, f)
char *ptr;
int size, nitems;
FILE *f;
{
register int i = 0;
register unsigned char *p;
	while (i < nitems) {
		while (mptr + size >= mhead + msize) {
			if ((p = realloc(mhead,msize+MALLOC_SIZE)) == (unsigned char *)-1){
				fputs("mio: out of memory\n",stderr);
				return (-1);
			}
			else {
				msize += MALLOC_SIZE;
				mptr = (unsigned char *) (p + (unsigned int)(mptr - mhead));
				mhead = p;
			}
		}
		if ((mptr + size) > mhead + msize)
			break;
		memmove(mend,ptr,size); /*bcopy(ptr,mend,size);*/
		ptr += size;
		mend += size;
		mptr = mend;
	}
	return (i);
}
