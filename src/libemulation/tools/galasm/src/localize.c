/******************************************************************************
** localize.h
*******************************************************************************
**
** description:
**
** Error strings.
**
******************************************************************************/
 



char *ErrorArray[] =
{
"",
"Can't find file!",
"Not enough free memory!",
"Can't load file!",
"File is empty!",
"Error in JEDEC file!",
"Impossible error: GAL is not empty!",
"No pinnames found.",
"Can't close file!",
"Wrong type of GAL selected!",
"Impossible error: Can't load 's:GALer.config'!",
"Impossible error: Can't save 's:GALer.config'!",
"Impossible error: Can't open window!",
"Can't save file!",
};

char *AsmErrorArray[] = 
{
"error in source file found",
"Line  1: type of GAL expected",
"unexpected end of file",
"pinname expected after '/'",
"max. length of pinname is 8 characters",
"illegal character in pin declaration",
"illegal VCC/GND assignment",
"pin declaration: expected VCC at VCC pin",
"pin declaration: expected GND at GND pin",
"pinname defined twice",
"illegal use of '/'",
"unknown pinname",
"NC (Not Connected) is not allowed in logic equations",
"unknown suffix found",
"'=' expected",
"this pin can't be used as output",
"same pin is defined multible as output",
"before using .E, the output must be defined",
"GAL22V10: AR and SP is not allowed as pinname",
".E, .CLK, .ARST and .APRST is not allowed to be negated",
"mode 2: pins 12, 19 can't be used as input",
"mode 2: pins 15, 22 can't be used as input",
"tristate control is defined twice",
"GAL16V8/20V8: tri. control for reg. output is not allowed",
"tristate control without previous '.T'",
"use GND, VCC instead of /VCC, /GND",
"mode 3: pins 1,11 are reserved for 'Clock' and '/OE'",
"mode 3: pins 1,13 are reserved for 'Clock' and '/OE'",
"use of VCC and GND is not allowed in equations",
"only one product term allowed (no OR)",
"too many product terms",
"use of AR and SP is not allowed in equations",
"negation of AR and SP is not allowed",
"no equations found",
".CLK is not allowed when this type of GAL is used",
".ARST is not allowed when this type of GAL is used",
".APRST is not allowed when this type of GAL is used",
"GAL20RA10: pin 1 can't be used in equations",
"GAL20RA10: pin 13 can't be used in equations",
"AR, SP: no suffix allowed",
"AR or SP is defined twice",
"missing clock definition (.CLK) of registered output",
"before using .CLK, the output must be defined",
"before using .ARST, the output must be defined",
"before using .APRST the output must be defined",
"several .CLK definitions for the same output found",
"several .ARST definitions for the same output found",
"several .APRST definitions for the same output found",
"use of .CLK, .ARST, .APRST only allowed for registered outputs"
};
