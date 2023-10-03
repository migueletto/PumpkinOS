/*
mot2bin converts a Motorola hex file to binary.

Copyright (C) 2015,  Jacques Pelletier
checksum extensions Copyright (C) 2004 Rockwell Automation
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

20040617 Alf Lacis: Added pad byte (may not always want FF).
         Added initialisation to Checksum to remove GNU
         compiler warning about possible uninitialised usage
         Added 2x'break;' to remove GNU compiler warning about label at
         end of compound statement
         Added PROGRAM & VERSION strings.

20071005 PG: Improvements on options parsing
20091212 JP: Corrected crash on 0 byte length data records
20100402 JP: ADDRESS_MASK is now calculated from MEMORY_SIZE

20120125 Danny Schneider:
         Added code for filling a binary file to a given Max_Length relative to
         Starting Address if Max-Address is larger than Highest-Address
20120509 Yoshimasa Nakane:
         modified error checking (also for output file, JP)
20141005 JP: added support for byte swapped hex files
         corrected bug caused by extra LF at end or within file
20141121 Slucx: added line for removing extra CR when entering file name at run time.
20150116 Richard Genoud (Paratronic): correct buffer overflows/wrong results with the -l flag
20150122 JP: added support for different check methods
20150221 JP: rewrite of the checksum write/force value
20150804 JP: added batch file option
*/

#define PROGRAM "mot2bin"
#define VERSION "2.5"

#include "common.h"

const char *Pgm_Name = PROGRAM;

int main (int argc, char *argv[])
{
    /* line inputted from file */
    char Line[MAX_LINE_SIZE];

    /* flag that a file was read */
    bool Fileread;

    /* cmd-line parameter # */
    char *p;

    int result;

    /* Application specific */
    unsigned int First_Word, Address;
    unsigned int Type;
    unsigned int Exec_Address;
    unsigned int temp;
    unsigned int Record_Count, Record_Checksum;

    byte	Data_Str[MAX_LINE_SIZE];

    fprintf (stdout,PROGRAM" v"VERSION", Copyright (C) 2017 Jacques Pelletier & contributors\n\n");

    if (argc == 1)
        usage();

    strcpy(Extension, "bin"); /* default is for binary file extension */

	ParseOptions(argc, argv);

    /* when user enters input file name */

    /* Assume last parameter is filename */
    GetFilename(Filename,argv[argc -1]);

    /* Just a normal file name */
    NoFailOpenInputFile (Filename);
    PutExtension(Filename, Extension);
    NoFailOpenOutputFile(Filename);
    Fileread = true;

    /* When the hex file is opened, the program will read it in 2 passes.
    The first pass gets the highest and lowest addresses so that we can allocate
    the right size.
    The second pass processes the hex data. */

    /* To begin, assume the lowest address is at the end of the memory.
     While reading each records, subsequent addresses will lower this number.
     At the end of the input file, this value will be the lowest address.

     A similar assumption is made for highest address. It starts at the
     beginning of memory. While reading each records, subsequent addresses will raise this number.
     At the end of the input file, this value will be the highest address. */
    Lowest_Address = (unsigned int)-1;
    Highest_Address = 0;
    Records_Start = 0;
    Record_Nb = 0;
    First_Word = 0;

    /* get highest and lowest addresses so that we can allocate the right size */
    do
    {
        unsigned int i;

        /* Read a line from input file. */
        GetLine(Line,Filin);
        Record_Nb++;

        /* Remove carriage return/line feed at the end of line. */
        i = strlen(Line);

        if (--i != 0)
        {
            if (Line[i] == '\n') Line[i] = '\0';

            p = (char *) Data_Str;

            switch(Line[1])
            {
            case '0':
            	Nb_Bytes = 1; /* This is to fix the Highest_Address set to -1 when Nb_Bytes = 0 */
                break;

            /* 16 bits address */
            case '1':
                result = sscanf (Line,"S%1x%2x%4x",&Type,&Nb_Bytes,&First_Word);
	            if (result != 3) fprintf(stderr,"Error in line %d of hex file\n", Record_Nb);

                /* Adjust Nb_Bytes for the number of data bytes */
                Nb_Bytes = Nb_Bytes - 3;
                break;

            /* 24 bits address */
            case '2':
                result = sscanf (Line,"S%1x%2x%6x",&Type,&Nb_Bytes,&First_Word);
	            if (result != 3) fprintf(stderr,"Error in line %d of hex file\n", Record_Nb);

                /* Adjust Nb_Bytes for the number of data bytes */
                Nb_Bytes = Nb_Bytes - 4;
                break;

            /* 32 bits address */
            case '3':
                result = sscanf (Line,"S%1x%2x%8x",&Type,&Nb_Bytes,&First_Word);
	            if (result != 3) fprintf(stderr,"Error in line %d of hex file\n", Record_Nb);

                /* Adjust Nb_Bytes for the number of data bytes */
                Nb_Bytes = Nb_Bytes - 5;
                break;
            }

            Phys_Addr = First_Word;

            /* Set the lowest address as base pointer. */
            if (Phys_Addr < Lowest_Address)
                Lowest_Address = Phys_Addr;

            /* Same for the top address. */
            temp = Phys_Addr + Nb_Bytes -1;

            if (temp > Highest_Address)
                Highest_Address = temp;
        }
    }
    while (!feof (Filin));

    Allocate_Memory_And_Rewind();

    Record_Nb = 0;

    /* Read the file & process the lines. */
    do /* repeat until EOF(Filin) */
    {
        int i;

        Checksum = 0;

        /* Read a line from input file. */
        GetLine(Line,Filin);
        Record_Nb++;

        /* Remove carriage return/line feed at the end of line. */
        i = strlen(Line);

        if (--i != 0)
        {
            if (Line[i] == '\n') Line[i] = '\0';

            /* Scan starting address and nb of bytes. */
            /* Look at the record type after the 'S' */
            Type = 0;

            switch(Line[1])
            {
            case '0':
                result = sscanf (Line,"S0%2x0000484452%2x",&Nb_Bytes,&Record_Checksum);
	            if (result != 2) fprintf(stderr,"Error in line %d of hex file\n", Record_Nb);
                Checksum = Nb_Bytes + 0x48 + 0x44 + 0x52;

                /* Adjust Nb_Bytes for the number of data bytes */
                Nb_Bytes = 0;
                break;

            /* 16 bits address */
            case '1':
                result = sscanf (Line,"S%1x%2x%4x%s",&Type,&Nb_Bytes,&Address,Data_Str);
	            if (result != 4) fprintf(stderr,"Error in line %d of hex file\n", Record_Nb);
                Checksum = Nb_Bytes + (Address >> 8) + (Address & 0xFF);

                /* Adjust Nb_Bytes for the number of data bytes */
                Nb_Bytes = Nb_Bytes - 3;
                break;

            /* 24 bits address */
            case '2':
                result = sscanf (Line,"S%1x%2x%6x%s",&Type,&Nb_Bytes,&Address,Data_Str);
	            if (result != 4) fprintf(stderr,"Error in line %d of hex file\n", Record_Nb);
                Checksum = Nb_Bytes + (Address >> 16) + (Address >> 8) + (Address & 0xFF);

                /* Adjust Nb_Bytes for the number of data bytes */
                Nb_Bytes = Nb_Bytes - 4;
                break;

            /* 32 bits address */
            case '3':
                result = sscanf (Line,"S%1x%2x%8x%s",&Type,&Nb_Bytes,&Address,Data_Str);
	            if (result != 4) fprintf(stderr,"Error in line %d of hex file\n", Record_Nb);
                Checksum = Nb_Bytes + (Address >> 24) + (Address >> 16) + (Address >> 8) + (Address & 0xFF);

                /* Adjust Nb_Bytes for the number of data bytes */
                Nb_Bytes = Nb_Bytes - 5;
                break;

            case '5':
                result = sscanf (Line,"S%1x%2x%4x%2x",&Type,&Nb_Bytes,&Record_Count,&Record_Checksum);
	            if (result != 4) fprintf(stderr,"Error in line %d of hex file\n", Record_Nb);
                Checksum = Nb_Bytes + (Record_Count >> 8) + (Record_Count & 0xFF);

                /* Adjust Nb_Bytes for the number of data bytes */
                Nb_Bytes = 0;
                break;

            case '7':
                result = sscanf (Line,"S%1x%2x%8x%2x",&Type,&Nb_Bytes,&Exec_Address,&Record_Checksum);
	            if (result != 4) fprintf(stderr,"Error in line %d of hex file\n", Record_Nb);
                Checksum = Nb_Bytes + (Exec_Address >> 24) + (Exec_Address >> 16) + (Exec_Address >> 8) + (Exec_Address & 0xFF);
                Nb_Bytes = 0;
                break;

            case '8':
                result = sscanf (Line,"S%1x%2x%6x%2x",&Type,&Nb_Bytes,&Exec_Address,&Record_Checksum);
	            if (result != 4) fprintf(stderr,"Error in line %d of hex file\n", Record_Nb);
                Checksum = Nb_Bytes + (Exec_Address >> 16) + (Exec_Address >> 8) + (Exec_Address & 0xFF);
                Nb_Bytes = 0;
                break;
            case '9':
                result = sscanf (Line,"S%1x%2x%4x%2x",&Type,&Nb_Bytes,&Exec_Address,&Record_Checksum);
	            if (result != 4) fprintf(stderr,"Error in line %d of hex file\n", Record_Nb);
                Checksum = Nb_Bytes + (Exec_Address >> 8) + (Exec_Address & 0xFF);
                Nb_Bytes = 0;
                break;
            }

            p = (char *) Data_Str;

            /* If we're reading the last record, ignore it. */
            switch (Type)
            {
            /* Data record */
            case 1:
            case 2:
            case 3:
                if (Nb_Bytes == 0)
                {
                    fprintf(stderr,"0 byte length Data record ignored\n");
                    break;
                }

                Phys_Addr = Address;

				p = ReadDataBytes(p);

                /* Read the Checksum value. */
                result = sscanf (p, "%2x",&Record_Checksum);
	            if (result != 1) fprintf(stderr,"Error in line %d of hex file\n", Record_Nb);
                break;

            case 5:
                fprintf(stderr,"Record total: %d\n",Record_Count);
                break;

            case 7:
                fprintf(stderr,"Execution Address (unused): %08X\n",Exec_Address);
                break;

            case 8:
                fprintf(stderr,"Execution Address (unused): %06X\n",Exec_Address);
                break;

            case 9:
                fprintf(stderr,"Execution Address (unused): %04X\n",Exec_Address);
                break;

            /* Ignore all other records */
            default:;
            }

            Record_Checksum &= 0xFF;

            /* Verify Checksum value. */
            if (((Record_Checksum + Checksum) != 0xFF) && Enable_Checksum_Error)
            {
                fprintf(stderr,"Checksum error in record %d: should be %02X\n",Record_Nb, 255-Checksum);
                Status_Checksum_Error = true;
            }
        }
    }
    while (!feof (Filin));
    /*-----------------------------------------------------------------------------*/

    fprintf(stdout,"Binary file start = %08X\n",Lowest_Address);
    fprintf(stdout,"Records start     = %08X\n",Records_Start);
    fprintf(stdout,"Highest address   = %08X\n",Highest_Address);
    fprintf(stdout,"Pad Byte          = %X\n",  Pad_Byte);

    WriteMemory();

#ifdef USE_FILE_BUFFERS
    free (FilinBuf);
    free (FiloutBuf);
#endif

    fclose (Filin);
    fclose (Filout);

    if (Status_Checksum_Error && Enable_Checksum_Error)
    {
        fprintf(stderr,"Checksum error detected.\n");
        return 1;
    }

    if (!Fileread)
        usage();
    return 0;
}
