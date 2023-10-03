/*
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

  20151124 Donna Whisnant: Bug fix for range checking of WriteMemory()
           and Allocate_Memory_And_Rewind() report of addresses in hexadecimal
  20160930 JP: corrected the wrong error report "Force/Check"
  20170304 JP: added the 16-bit checksum 8-bit wide
*/

#include "common.h"

filetype    Filename;           /* string for opening files */
char        Extension[MAX_EXTENSION_SIZE];       /* filename extension for output files */

FILE        *Filin,             /* input files */
            *Filout;            /* output files */

#ifdef USE_FILE_BUFFERS
char		*FilinBuf,          /* text buffer for file input */
            *FiloutBuf;         /* text buffer for file output */
#endif

int Pad_Byte = 0xFF;
bool Enable_Checksum_Error = false;
bool Status_Checksum_Error = false;
byte 	Checksum;
unsigned int Record_Nb;
unsigned int Nb_Bytes;

/* This will hold binary codes translated from hex file. */
byte *Memory_Block;
unsigned int Lowest_Address, Highest_Address;
unsigned int Starting_Address, Phys_Addr;
unsigned int Records_Start; // Lowest address of the records
unsigned int Max_Length = 0;
unsigned int Minimum_Block_Size = 0x1000; // 4096 byte
unsigned int Floor_Address = 0x0;  
unsigned int Ceiling_Address = 0xFFFFFFFF; 
int Module;
bool Minimum_Block_Size_Setted = false;
bool Starting_Address_Setted = false;
bool Floor_Address_Setted = false;
bool Ceiling_Address_Setted = false;
bool Max_Length_Setted = false;
bool Swap_Wordwise = false;
bool Address_Alignment_Word = false;
bool Batch_Mode = false;
bool Verbose_Flag = false;

int Endian = 0;

t_CRC Cks_Type = CHK8_SUM;
unsigned int Cks_Start = 0, Cks_End = 0, Cks_Addr = 0, Cks_Value = 0;
bool Cks_range_set = false;
bool Cks_Addr_set = false;
bool Force_Value = false;

unsigned int Crc_Poly = 0x07, Crc_Init = 0, Crc_XorOut = 0;
bool Crc_RefIn = false;
bool Crc_RefOut = false;

void usage(void)
{
    fprintf (stderr,
             "\n"
             "usage: %s [OPTIONS] filename\n"
             "Options:\n"
             "  -a            Address Alignment Word (hex2bin only)\n"
             "  -b            Batch mode: exits if specified file doesn't exist\n"
             "  -c            Enable record checksum verification\n"
             "  -C [Poly][Init][RefIn][RefOut][XorOut]\n                CRC parameters\n"
             "  -e [ext]      Output filename extension (without the dot)\n"
             "  -E [0|1]      Endian for checksum/CRC, 0: little, 1: big\n"
             "  -f [address]  Address of check result to write\n"
             "  -F [address] [value]\n                Address and value to force\n"
             "  -k [0-5]      Select check method (checksum or CRC) and size\n"
             "  -d            display list of check methods/value size\n"
             "  -l [length]   Maximal Length (Starting address + Length -1 is Max Address)\n"
             "                File will be filled with Pattern until Max Address is reached\n"
             "  -m [size]     Minimum Block Size\n"
             "                File Size Dimension will be a multiple of Minimum block size\n"
             "                File will be filled with Pattern\n"
             "                Length must be a power of 2 in hexadecimal [see -l option]\n"
             "                Attention this option is STRONGER than Maximal Length  \n"
             "  -p [value]    Pad-byte value in hex (default: %x)\n"
             "  -r [start] [end]\n"
             "                Range to compute checksum over (default is min and max addresses)\n"
             "  -s [address]  Starting address in hex for binary file (default: 0)\n"
             "                ex.: if the first record is :nn010000ddddd...\n"
             "                the data supposed to be stored at 0100 will start at 0000\n"
             "                in the binary file.\n"
             "                Specifying this starting address will put pad bytes in the\n"
             "                binary file so that the data supposed to be stored at 0100\n"
             "                will start at the same address in the binary file.\n"
             "  -t [address]  Floor address in hex (hex2bin only)\n"
             "  -T [address]  Ceiling address in hex (hex2bin only)\n"
             "  -v            Verbose messages for debugging purposes\n"
             "  -w            Swap wordwise (low <-> high)\n\n",
             Pgm_Name,Pad_Byte);
    exit(1);
} /* procedure USAGE */

void DisplayCheckMethods(void)
{
    fprintf (stderr,
             "Check methods/value size:\n"
             "0:  Checksum  8-bit\n"
             "1:  Checksum 16-bit (adds 16-bit words into a 16-bit sum, data and result BE or LE)\n"
             "2:  CRC8\n"
             "3:  CRC16\n"
             "4:  CRC32\n"
             "5:  Checksum 16-bit (adds bytes into a 16-bit sum, result BE or LE)\n");
    exit(1);
}

void *NoFailMalloc (size_t size)
{
    void *result;

    if ((result = malloc (size)) == NULL)
    {
        fprintf (stderr,"Can't allocate memory.\n");
        exit(1);
    }
    return (result);
}

/* Open the input file, with error checking */
void NoFailOpenInputFile (char *Flnm)
{
    while ((Filin = fopen(Flnm,"r")) == NULL)
    {
    	if (Batch_Mode)
    	{
			fprintf (stderr,"Input file %s cannot be opened.\n", Flnm);
    		exit(1);
    	}
    	else
    	{
			fprintf (stderr,"Input file %s cannot be opened. Enter new filename: ",Flnm);
			if (Flnm[strlen(Flnm) - 1] == '\n') Flnm[strlen(Flnm) - 1] = '\0';
    	}
    }

#ifdef USE_FILE_BUFFERS
    FilinBuf = (char *) NoFailMalloc (BUFFSZ);
    setvbuf(Filin, FilinBuf, _IOFBF, BUFFSZ);
#endif
} /* procedure OPENFILIN */

/* Open the output file, with error checking */
void NoFailOpenOutputFile (char *Flnm)
{
    while ((Filout = fopen(Flnm,"wb")) == NULL)
    {
    	if (Batch_Mode)
    	{
			fprintf (stderr,"Output file %s cannot be opened.\n", Flnm);
    		exit(1);
    	}
    	else
    	{
			/* Failure to open the output file may be
			 simply due to an insufficient permission setting. */
			fprintf(stderr,"Output file %s cannot be opened. Enter new file name: ", Flnm);
			if (Flnm[strlen(Flnm) - 1] == '\n') Flnm[strlen(Flnm) - 1] = '\0';
    	}
    }

#ifdef USE_FILE_BUFFERS
    FiloutBuf = (char *) NoFailMalloc (BUFFSZ);
    setvbuf(Filout, FiloutBuf, _IOFBF, BUFFSZ);
#endif

} /* procedure OPENFILOUT */

void GetLine(char* str,FILE *in)
{
    char *result;

    result = fgets(str,MAX_LINE_SIZE,in);
    if ((result == NULL) && !feof (in)) fprintf(stderr,"Error occurred while reading from file\n");
}

// 0 or 1
int GetBin(const char *str)
{
    int result;
    unsigned int value;

    result = sscanf(str,"%u",&value);

    if (result == 1) return value & 1;
    else
    {
        fprintf(stderr,"GetBin: some error occurred when parsing options.\n");
        exit (1);
    }
}

int GetDec(const char *str)
{
    int result;
    unsigned int value;

    result = sscanf(str,"%u",&value);

    if (result == 1) return value;
    else
    {
        fprintf(stderr,"GetDec: some error occurred when parsing options.\n");
        exit (1);
    }
}

int GetHex(const char *str)
{
    int result;
    unsigned int value;

    result = sscanf(str,"%x",&value);

    if (result == 1) return value;
    else
    {
        fprintf(stderr,"GetHex: some error occurred when parsing options.\n");
        exit (1);
    }
}

// Char t/T: true f/F: false
bool GetBoolean(const char *str)
{
    int result;
    unsigned char value, temp;

    result = sscanf(str,"%c",&value);
    temp = tolower(value);

    if ((result == 1) && ((temp == 't') || (temp == 'f')))
    {
        return (temp == 't');
    }
    else
    {
        fprintf(stderr,"GetBoolean: some error occurred when parsing options.\n");
        exit (1);
    }
}

void GetFilename(char *dest,char *src)
{
    if (strlen(src) < MAX_FILE_NAME_SIZE)
    {
        strcpy(dest,src);
    }
    else
    {
        fprintf(stderr,"filename length exceeds %d characters.\n",MAX_FILE_NAME_SIZE);
        exit (1);
    }
}

void GetExtension(const char *str,char *ext)
{
    if (strlen(str) > MAX_EXTENSION_SIZE)
        usage();

    strcpy(ext, str);
}

/* Adds an extension to a file name */
void PutExtension(char *Flnm, char *Extension)
{
    char        *Period;        /* location of period in file name */

    /* This assumes DOS like file names */
    /* Don't use strchr(): consider the following filename:
     ../my.dir/file.hex
    */
    if ((Period = strrchr(Flnm,'.')) != NULL)
    {
        *(Period) = '\0';
        if (strcmp(Extension, Period+1) == 0)
        {
            fprintf (stderr,"Input and output filenames (%s) are the same.\n", Flnm);
            exit(1);
        }
    }
    strcat(Flnm,".");
    strcat(Flnm, Extension);
}

void VerifyChecksumValue(void)
{
    if ((Checksum != 0) && Enable_Checksum_Error)
	{
		fprintf(stderr,"Checksum error in record %d: should be %02X\n",
			Record_Nb, (256 - Checksum) & 0xFF);
		Status_Checksum_Error = true;
	}
}

/* Check if are set Floor and Ceiling Address and range is coherent*/
void VerifyRangeFloorCeil(void)
{
    if (Floor_Address_Setted && Ceiling_Address_Setted && (Floor_Address >= Ceiling_Address))
    {
        fprintf (stderr,"Floor address %08X higher than Ceiling address %08X\n",Floor_Address,Ceiling_Address);
        exit(1);
    }
}

void CrcParamsCheck(void)
{
    switch (Cks_Type)
    {
    case CRC8:
        Crc_Poly &= 0xFF;
        Crc_Init &= 0xFF;
        Crc_XorOut &= 0xFF;
        break;
    case CRC16:
        Crc_Poly &= 0xFFFF;
        Crc_Init &= 0xFFFF;
        Crc_XorOut &= 0xFFFF;
        break;
    case CRC32:
        break;
    default:
        fprintf (stderr,"See file CRC list.txt for parameters\n");
        exit(1);
    }
}

void WriteMemBlock16(uint16_t Value)
{
    if (Endian == 1)
    {
        Memory_Block[Cks_Addr - Lowest_Address]    = u16_hi(Value);
        Memory_Block[Cks_Addr - Lowest_Address +1] = u16_lo(Value);
    }
    else
    {
        Memory_Block[Cks_Addr - Lowest_Address +1] = u16_hi(Value);
        Memory_Block[Cks_Addr - Lowest_Address]    = u16_lo(Value);
    }
}

void WriteMemBlock32(uint32_t Value)
{
    if (Endian == 1)
    {
        Memory_Block[Cks_Addr - Lowest_Address]    = u32_b3(Value);
        Memory_Block[Cks_Addr - Lowest_Address +1] = u32_b2(Value);
        Memory_Block[Cks_Addr - Lowest_Address +2] = u32_b1(Value);
        Memory_Block[Cks_Addr - Lowest_Address +3] = u32_b0(Value);
    }
    else
    {
        Memory_Block[Cks_Addr - Lowest_Address +3] = u32_b3(Value);
        Memory_Block[Cks_Addr - Lowest_Address +2] = u32_b2(Value);
        Memory_Block[Cks_Addr - Lowest_Address +1] = u32_b1(Value);
        Memory_Block[Cks_Addr - Lowest_Address]    = u32_b0(Value);
    }
}

void WriteMemory(void)
{
    if ((Cks_Addr >= Lowest_Address) && (Cks_Addr < Highest_Address))
    {
        if(Force_Value)
        {
            switch (Cks_Type)
            {
                case 0:
                    Memory_Block[Cks_Addr - Lowest_Address] = Cks_Value;
                    fprintf(stdout,"Addr %08X set to %02X\n",Cks_Addr, Cks_Value);
                    break;
                case 1:
                    WriteMemBlock16(Cks_Value);
                    fprintf(stdout,"Addr %08X set to %04X\n",Cks_Addr, Cks_Value);
                    break;
                case 2:
                    WriteMemBlock32(Cks_Value);
                    fprintf(stdout,"Addr %08X set to %08X\n",Cks_Addr, Cks_Value);
                    break;
                default:;
            }
        }
        else if (Cks_Addr_set)
        {
            /* Add a checksum to the binary file */
            if (!Cks_range_set)
            {
                Cks_Start = Lowest_Address;
                Cks_End = Highest_Address;
            }
            /* checksum range MUST BE in the array bounds */

            if (Cks_Start < Lowest_Address)
            {
                fprintf(stdout,"Modifying range start from %X to %X\n",Cks_Start,Lowest_Address);
                Cks_Start = Lowest_Address;
            }
            if (Cks_End > Highest_Address)
            {
                fprintf(stdout,"Modifying range end from %X to %X\n",Cks_End,Highest_Address);
                Cks_End = Highest_Address;
            }

            crc_table = NULL;

            switch (Cks_Type)
            {
            case CHK8_SUM:
            {
                uint8_t wCKS = 0;

                for (unsigned int i=Cks_Start; i<=Cks_End; i++)
                {
                    wCKS += Memory_Block[i - Lowest_Address];
                }

                fprintf(stdout,"8-bit Checksum = %02X\n",wCKS & 0xff);
                Memory_Block[Cks_Addr - Lowest_Address] = wCKS;
                fprintf(stdout,"Addr %08X set to %02X\n",Cks_Addr, wCKS);
            }
            break;

            case CHK16:
            {
                uint16_t wCKS, w;

                wCKS = 0;

                if (Endian == 1)
                {
                    for (unsigned int i=Cks_Start; i<=Cks_End; i+=2)
                    {
                        w =  Memory_Block[i - Lowest_Address +1] | ((word)Memory_Block[i - Lowest_Address] << 8);
                        wCKS += w;
                    }
                }
                else
                {
                    for (unsigned int i=Cks_Start; i<=Cks_End; i+=2)
                    {
                        w =  Memory_Block[i - Lowest_Address] | ((word)Memory_Block[i - Lowest_Address +1] << 8);
                        wCKS += w;
                    }
                }
                fprintf(stdout,"16-bit Checksum = %04X\n",wCKS);
                WriteMemBlock16(wCKS);
                fprintf(stdout,"Addr %08X set to %04X\n",Cks_Addr, wCKS);
            }
            break;

            case CHK16_8:
            {
                uint16_t wCKS;

                wCKS = 0;

                for (unsigned int i=Cks_Start; i<=Cks_End; i++)
                {
                    wCKS += Memory_Block[i - Lowest_Address];
                }

                fprintf(stdout,"16-bit Checksum = %04X\n",wCKS);
                WriteMemBlock16(wCKS);
                fprintf(stdout,"Addr %08X set to %04X\n",Cks_Addr, wCKS);
            }
            break;

            case CRC8:
            {
                uint8_t CRC8;
                crc_table = NoFailMalloc(256);

                if (Crc_RefIn)
                {
                    init_crc8_reflected_tab(Reflect8[Crc_Poly]);
                    CRC8 = Reflect8[Crc_Init];
                }
                else
                {
                    init_crc8_normal_tab(Crc_Poly);
                    CRC8 = Crc_Init;
                }

                for (unsigned int i=Cks_Start; i<=Cks_End; i++)
                {
                    CRC8 = update_crc8(CRC8,Memory_Block[i - Lowest_Address]);
                }

                CRC8 = (CRC8 ^ Crc_XorOut) & 0xff;
                Memory_Block[Cks_Addr - Lowest_Address] = CRC8;
                fprintf(stdout,"Addr %08X set to %02X\n",Cks_Addr, CRC8);
            }
            break;

            case CRC16:
            {
                uint16_t CRC16;
                crc_table = NoFailMalloc(256 * 2);

                if (Crc_RefIn)
                {
                    init_crc16_reflected_tab(Reflect16(Crc_Poly));
                    CRC16 = Reflect16(Crc_Init);

                    for (unsigned int i=Cks_Start; i<=Cks_End; i++)
                    {
                        CRC16 = update_crc16_reflected(CRC16,Memory_Block[i - Lowest_Address]);
                    }
                }
                else
                {
                    init_crc16_normal_tab(Crc_Poly);
                    CRC16 = Crc_Init;


                    for (unsigned int i=Cks_Start; i<=Cks_End; i++)
                    {
                        CRC16 = update_crc16_normal(CRC16,Memory_Block[i - Lowest_Address]);
                    }
                }

                CRC16 = (CRC16 ^ Crc_XorOut) & 0xffff;
                WriteMemBlock16(CRC16);
                fprintf(stdout,"Addr %08X set to %04X\n",Cks_Addr, CRC16);
            }
            break;

            case CRC32:
            {
                uint32_t CRC32;

                crc_table = NoFailMalloc(256 * 4);
                if (Crc_RefIn)
                {
                    init_crc32_reflected_tab(Reflect32(Crc_Poly));
                    CRC32 = Reflect32(Crc_Init);

                    for (unsigned int i=Cks_Start; i<=Cks_End; i++)
                    {
                        CRC32 = update_crc32_reflected(CRC32,Memory_Block[i - Lowest_Address]);
                    }
                }
                else
                {
                    init_crc32_normal_tab(Crc_Poly);
                    CRC32 = Crc_Init;

                    for (unsigned int i=Cks_Start; i<=Cks_End; i++)
                    {
                        CRC32 = update_crc32_normal(CRC32,Memory_Block[i - Lowest_Address]);
                    }
                }

                CRC32 ^= Crc_XorOut;
                WriteMemBlock32(CRC32);
                fprintf(stdout,"Addr %08X set to %08X\n",Cks_Addr, CRC32);
            }
            break;

            default:
                ;
            }

            if (crc_table != NULL) free(crc_table);
        }
    }
    else
    {
        if(Force_Value || Cks_Addr_set)
        {
            fprintf (stderr,"Force/Check address outside of memory range\n");
        }
    }

    /* write binary file */
    fwrite (Memory_Block,
            Max_Length,
            1,
            Filout);

    free (Memory_Block);

    // Minimum_Block_Size is set; the memory buffer is multiple of this?
    if (Minimum_Block_Size_Setted==true)
    {
        Module = Max_Length % Minimum_Block_Size;
        if (Module)
        {
            Module = Minimum_Block_Size - Module;
            Memory_Block = (byte *) NoFailMalloc(Module);
            memset (Memory_Block,Pad_Byte,Module);
            fwrite (Memory_Block,
                    Module,
                    1,
                    Filout);
            free (Memory_Block);
            if (Max_Length_Setted==true)
                fprintf(stdout,"Attention Max Length changed by Minimum Block Size\n");
            // extended 
            Max_Length += Module;
            Highest_Address += Module;
            fprintf(stdout,"Extended\nHighest address:  %08X\n",Highest_Address);
            fprintf(stdout,"Max Length:       %u\n\n",Max_Length);

        }
    }
}

void Allocate_Memory_And_Rewind(void)
{
    Records_Start = Lowest_Address;

    if (Starting_Address_Setted == true)
    {
        Lowest_Address = Starting_Address;
    }
    else
    {
        Starting_Address = Lowest_Address;
    }

    if (Max_Length_Setted == false)
        Max_Length = Highest_Address - Lowest_Address + 1;
    else
        Highest_Address = Lowest_Address + Max_Length - 1;

    fprintf(stdout,"Allocate_Memory_and_Rewind:\n");
	fprintf(stdout,"Lowest address:   %08X\n",Lowest_Address);
	fprintf(stdout,"Highest address:  %08X\n",Highest_Address);
	fprintf(stdout,"Starting address: %08X\n",Starting_Address);
	fprintf(stdout,"Max Length:       %u\n\n",Max_Length);

    /* Now that we know the buffer size, we can allocate it. */
    /* allocate a buffer */
    Memory_Block = (byte *) NoFailMalloc(Max_Length);

    /* For EPROM or FLASH memory types, fill unused bytes with FF or the value specified by the p option */
    memset (Memory_Block,Pad_Byte,Max_Length);

    rewind(Filin);
}

char *ReadDataBytes(char *p)
{
unsigned int i,temp2;
int result;

	/* Read the Data bytes. */
	/* Bytes are written in the Memory block even if checksum is wrong. */
	i = Nb_Bytes;

	do
	{
		result = sscanf (p, "%2x",&temp2);
		if (result != 1) fprintf(stderr,"ReadDataBytes: error in line %d of hex file\n", Record_Nb);
		p += 2;

		/* Check that the physical address stays in the buffer's range. */
		if (Phys_Addr < Max_Length)
		{
			/* Overlapping record will erase the pad bytes */
			if (Swap_Wordwise)
			{
				if (Memory_Block[Phys_Addr ^ 1] != Pad_Byte) fprintf(stderr,"Overlapped record detected\n");
				Memory_Block[Phys_Addr++ ^ 1] = temp2;
			}
			else
			{
				if (Memory_Block[Phys_Addr] != Pad_Byte) fprintf(stderr,"Overlapped record detected\n");
				Memory_Block[Phys_Addr++] = temp2;
			}

			Checksum = (Checksum + temp2) & 0xFF;
		}
	}
	while (--i != 0);

	return p;
}

void ParseOptions(int argc, char *argv[])
{
int Param;
char *p;

    Starting_Address = 0;

    /* Parse options on the command line
    variables:
    use p for parsing arguments
    use i for number of parameters to skip
    use c for the current option
    */
    for (Param = 1; Param < argc; Param++)
    {
        int i = 0;
        char c;

        p = argv[Param];
        c = *(p+1); /* Get option character */

		if ( _IS_OPTION_(*p) )
        {
            // test for no space between option and parameter
            if (strlen(p) != 2) usage();

            switch(c)
            {
            case 'a':
                Address_Alignment_Word = true;
                i = 0;
                break;
			case 'b':
				Batch_Mode = true;
				i = 0;
				break;
            case 'c':
                Enable_Checksum_Error = true;
                i = 0;
                break;
            case 'd':
                DisplayCheckMethods();
            case 'e':
                GetExtension(argv[Param + 1],Extension);
                i = 1; /* add 1 to Param */
                break;
            case 'E':
                Endian = GetBin(argv[Param + 1]);
                i = 1; /* add 1 to Param */
                break;
            case 'f':
                Cks_Addr = GetHex(argv[Param + 1]);
                Cks_Addr_set = true;
                i = 1; /* add 1 to Param */
                break;
            case 'F':
                Cks_Addr = GetHex(argv[Param + 1]);
                Cks_Value = GetHex(argv[Param + 2]);
                Force_Value = true;
                i = 2; /* add 2 to Param */
                break;
            case 'k':
                Cks_Type = GetHex(argv[Param + 1]);
                {
                    if (Cks_Type > LAST_CHECK_METHOD) usage();
                }
                i = 1; /* add 1 to Param */
                break;
            case 'l':
                Max_Length = GetHex(argv[Param + 1]);
				if (Max_Length > 0x800000)
				{
					fprintf(stderr,"Max_Length = %u\n", Max_Length);
					exit(1);
				}
                Max_Length_Setted = true;
                i = 1; /* add 1 to Param */
                break;
            case 'm':
                Minimum_Block_Size = GetHex(argv[Param + 1]);
                Minimum_Block_Size_Setted = true;
                i = 1; /* add 1 to Param */
                break;
            case 'p':
                Pad_Byte = GetHex(argv[Param + 1]);
                i = 1; /* add 1 to Param */
                break;
            case 'r':
                Cks_Start = GetHex(argv[Param + 1]);
                Cks_End = GetHex(argv[Param + 2]);
                Cks_range_set = true;
                i = 2; /* add 2 to Param */
                break;
            case 's':
                Starting_Address = GetHex(argv[Param + 1]);
                Starting_Address_Setted = true;
                i = 1; /* add 1 to Param */
                break;
            case 'v':
                Verbose_Flag = true;
                i = 0;
                break;
      		case 't':
                Floor_Address = GetHex(argv[Param + 1]);
                Floor_Address_Setted = true;
                i = 1; /* add 1 to Param */
                break;
            case 'T':
                Ceiling_Address = GetHex(argv[Param + 1]);
                Ceiling_Address_Setted = true;
                i = 1; /* add 1 to Param */
                break;
            case 'w':
                Swap_Wordwise = true;
                i = 0;
                break;
            case 'C':
                Crc_Poly = GetHex(argv[Param + 1]);
                Crc_Init = GetHex(argv[Param + 2]);
                Crc_RefIn = GetBoolean(argv[Param + 3]);
                Crc_RefOut = GetBoolean(argv[Param + 4]);
                Crc_XorOut = GetHex(argv[Param + 5]);
                CrcParamsCheck();
                i = 5; /* add 5 to Param */
                break;

            case '?':
            case 'h':
            default:
                usage();
            } /* switch */

            /* Last parameter is not a filename */
            if (Param == argc-1) usage();

            // fprintf(stderr,"Param: %d, option: %c\n",Param,c);

            /* if (Param + i) < (argc -1) */
            if (Param < argc -1 -i) Param += i;
            else usage();

        }
        else
            break;
        /* if option */
    } /* for Param */
}
