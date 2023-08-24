This is an adptation of Chocolate Doom (https://www.chocolate-doom.org/) to PumpkinOS.
It is not a complete port, as some features may not be present (for example, sound is missing).
It allows you to play single-player versions of these games: Doom, Heretic, Hexen and Strife.
Each game is built as a separate PRC, and each one has a separate home folder.
The home folder is used to store WADs, configuration files e save files.

Currently supported WADs (IWAD):

WAD file      Size      Home folder              Comment
------------  --------  ----------------------   ----------------------------
DOOM1.WAD      4196020  /PALM/Programs/Doom      DOOM (shareware version)
DOOM.WAD      11159840  /PALM/Programs/Doom      DOOM (registered version)
DOOM2.WAD     14943400  /PALM/Programs/Doom      DOOM II
PLUTONIA.WAD  17420824  /PALM/Programs/Doom      The Plutonia Experiment
TNT.WAD       18195736  /PALM/Programs/Doom      TNT: Evilution
HERETIC1.WAD   5120300  /PALM/Programs/Heretic   Heretic (shareware version)
HERETIC.WAD   11096488  /PALM/Programs/Heretic   Heretic (registered version)
HEXEN.WAD     20083672  /PALM/Programs/Hexen
STRIFE1.WAD   28372168  /PALM/Programs/Strife

Doom, as you can see in the list, accepts five different WADs (called "variants" here).
If the home folfer contains more than one variant, you will be asked to select one.
If there is only one variant, it will be used automatically.
It there are no variants, an error message will appear.

Extra WADs (PWAD) are supported, and must also be placed in the home folder.
If you have arrival.wad, for example, which is a PWAD for Doom II, put it inside /PALM/Programs/Doom.
When you open the Doom application, besides choosing the variant, you will now have
a chance to choose an extra WAD from a selection box (choose "none", if you just want to run Doom II).

The configuration files are simple text files, as used in Chocolate Doom.
They are created with default values when you run a game for the first time.
The application's Config menu allows you to configure the "show message" option
and to adjust the gamma factor (the lower the value, the darker the display).
To change any other option (like key assignments) you must edit the file manually.
Key values are ASCII codes, not scan codes like in Chocolate Doom.
