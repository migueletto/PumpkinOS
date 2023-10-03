/******************************************************************************
** support.c
*******************************************************************************
**
** description:
**
** Some support routines.
**
******************************************************************************/

#include "galasm.h"
#include <stdlib.h>
#include <string.h>
 
/******************************************************************************
** GetBaseName(char *filename)
*******************************************************************************
** input:   *filename   filename
**
** output:  pointer to a string with a filename and
**			enough space to add the extension later
**			or NULL.
**
** remarks: the returned pointer must be free()ed 
**
******************************************************************************/

char *GetBaseName(char *filename)
{
	int c,n;

	char *p;

	c = strlen(filename);

	for(n = c; n != 0; n--)
	{ 
		if(filename[n-1] == '.')
		{
			n--;
			break;
		}
	}

	if(n == 0) n = c;

	if((p = (char *)malloc(n+5)))
	{
		strncpy(p, filename, n); 
		p[n+4] = '\0';

		return(p);
	}
	else
		return(NULL);
}


/******************************************************************************
** FileSize()
*******************************************************************************
** input:   *filename   filename
**
** output:  -1   file does not exist
**          -2   not enough memory for FileInfoBlock
**          >0   length of the specified file
**
** remarks: returns length of a file
**
******************************************************************************/
 
int FileSize(char *filename)
{
	FILE *fp;

	int size;

	if((fp = fopen(filename, "r")))
	{
		fseek(fp, 0, SEEK_END);

		size = ftell(fp);

		fclose(fp);
	}
	else
		return(-1);

	return(size);
}

/******************************************************************************
** ReadFile()
*******************************************************************************
** input:   *filename       name of file
**          filesize        length of file [bytes]
**          *filebuff       where to store the specified file
**
** output:  TRUE:  file loaded
**          FALSE: failed to load
**
** remarks: loads the specified file into memory
**
******************************************************************************/

int ReadFile(char *filename, int filesize, char *filebuff)
{
    int  actlen;
    FILE *fp;

	if((fp = fopen(filename, "r")))
	{
		actlen = fread(filebuff,1,filesize,fp);

		fclose(fp);

		if(actlen == filesize)
			return(TRUE);
	}
	return(FALSE);
}

/* The functions AddByte(), AddString(), IncPointer(), DecPointer() */
/* and FreeBuffer() are useful for handling chained lists           */

/******************************************************************************
** AddByte()
*******************************************************************************
** input:   *buff   pointer to buffer structure
**          code    byte to add to the list
**
** output:  0:   o.k.
*           !=0: error occured (e.g. not enough free memory)
**
** remarks: This function does set a byte in a buffer. If the end of the buffer
**          is reached, a new buffer is added to the chained list of buffers.
**
******************************************************************************/

int AddByte(struct ActBuffer *buff, UBYTE code)
{
    struct  Buffer  *mybuff;

    if ((buff->Entry) < (buff->BuffEnd))    /* is the current address within */
    {                                       /* the buffer ?                  */
        *buff->Entry++ = code;              /* the fill in the byte */
    }
    else
    {                                       /* if not, then add a new buffer */
		if(!(mybuff = (struct Buffer *)calloc(sizeof(struct Buffer),1)))
        {
            ErrorReq(2);
            return(-1);
        }

        buff->ThisBuff->Next = mybuff;      /* new buffer is current buffer */
        mybuff->Prev   = buff->ThisBuff;    /* previous is old buffer       */
        buff->ThisBuff = mybuff;            /* current buffer is new buffer */
        buff->Entry    = (UBYTE *)(&mybuff->Entries[0]);
        buff->BuffEnd  = (UBYTE *)mybuff + (long)sizeof(struct Buffer);
        *buff->Entry++ = code;
    }
    return(0);
}

/******************************************************************************
** AddString()
*******************************************************************************
** input:   *buff    pointer to buffer structure
**          strnptr  pointer to string to be added to the buffer
**
** output:  0:   o.k.
**          !=0: error occured (e.g. not enough free memory)
**
** remarks: This function does add a string to a buffer. If the end of the
**          buffer is reached, a new buffer is added to the chained list of
**          buffers.
**
******************************************************************************/
 
int AddString(struct ActBuffer *buff, UBYTE *strnptr)
{
    while (*strnptr)
    {
        if (AddByte(buff, *strnptr++))
            return(-1);
    }
    return(0);
}

/******************************************************************************
** IncPointer()
*******************************************************************************
** input:   *buff   pointer to buffer structure
**
** output:  none
**
** remarks: This function does increment the pointer of the current buffer
**          structure. If the pointer points to the end of the buffer, the
**          pointer will be set to the beginning of the next entry of the
**          chained list.
**          ATTENTION: there is no test whether the absolut end of the chained
**                     list is reached or not!
**
******************************************************************************/
 
void IncPointer(struct ActBuffer *buff)
{

    buff->Entry++;

    if (buff->Entry == buff->BuffEnd)       /* end of buffer reached? */
    {
        buff->ThisBuff = buff->ThisBuff->Next;  /* yes, then set the pointer */
                                                /* to the next buffer        */

        buff->Entry    = (UBYTE *)(&buff->ThisBuff->Entries[0]);

        buff->BuffEnd  = (UBYTE *)buff->ThisBuff + (long)sizeof(struct Buffer);
    }
}

/******************************************************************************
** DecPointer()
*******************************************************************************
** input:   *buff   pointer to buffer structure
**
** output:  none
**
** remarks: This function does decrement the pointer of the current buffer
**          structure. If the pointer points to the start of the buffer, the
**          pointer will be set to the beginning of the previous entry of the
**          chained list.
**          ATTENTION: there is no test whether the absolut start of the chained
**                     list is reached or not!
**
******************************************************************************/

void DecPointer(struct ActBuffer *buff)
{

    buff->Entry--;

    if (buff->Entry < &buff->ThisBuff->Entries[0])  /* start of buffer reached? */
    {
        buff->ThisBuff = buff->ThisBuff->Prev;    /* pointer to previous buffer */

        buff->BuffEnd  = (UBYTE *)buff->ThisBuff + (long)sizeof(struct Buffer);

        buff->Entry    = (UBYTE *)((buff->BuffEnd)-1L);
    }

}

/******************************************************************************
** FreeBuffer()
*******************************************************************************
** input:   *buff   pointer to buffer structure
**
** output:  none
**
** remarks: This function does free memory which is allocated by the chained
**          list.
**
******************************************************************************/

void FreeBuffer(struct Buffer *buff)
{
	struct Buffer *nextbuff;

	while(buff)
	{
		nextbuff = buff->Next;
		
		free(buff);
		
		buff = nextbuff;
	}
}	

/******************************************************************************
** GetGALName()
*******************************************************************************
** input:   gal type
**
** output:  pointer to a string with the gal name
**
******************************************************************************/

char *GetGALName(int galtype)
{
	switch(galtype)
	{
		case GAL16V8:	return("16V8");
		case GAL20V8:	return("20V8");
		case GAL22V10:	return("22V10");
		case GAL20RA10:	return("20RA10");
		
		default:
				return "UNKNOWN";
	}
}

/******************************************************************************
** errorReq()
*******************************************************************************
** input:   error number
**
******************************************************************************/

void ErrorReq(int errornum)
{
	printf("Error: %s\n",ErrorArray[errornum]);
}
