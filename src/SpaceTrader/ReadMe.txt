SPACE TRADER 1.2.2 SOURCE CODE 
******************************

This package contains the source code of Space Trader 1.2.2. The author and copyright holder of Space Trader is Pieter Spronck. The homepage for the game is http://www.spronck.net/picoverse/spacetrader.

You should unzip the package with the option to create subdirectories on.


License
*******

This code is distributed under the GNU General Public License. The GPL license is found in the file License.txt, included in the package, and can also be found on the Web at http://www.opensource.org/licenses/gpl-license.html or http://www.gnu.org/copyleft/gpl.html.

In general, this license protects the author's copyright to his own code, but allows others to create their own programs based on this code, provided they distribute these programs under the same conditions as the original program, that is, also under the GPL license (which means, among other things, that you should release the program with source code). There are, of course, more details to the license, which you can look up in the complete license text.

Please do not release a new version of Space Trader without consulting me. For the time being, I would prefer to remain the only person who releases Space Trader versions. You can, however, create new programs based on the Space Trader code, port Space Trader to another platform or translate it into another language (please, consult me before you do that, and be warned that I still intend to enhance the program), use parts of the code in your own programs, and experiment with changes which you can then email to me so I can incorporate them in a new release.

In addition, if you do wish to make use of this code for another program, I'd like you to mention this somewhere in your program, and to inform me of your efforts, especially if you intend to distribute it. If nothing else, I might give you some advice on how to best use the code. Or, in case you intend to port it to another platform, I can inform you whether or not someone else is already creating such a port.

Note that the in-game pictures, except for the program's icon, are copyrighted by Alexander Lawrence. You can contact him at al_virtual@yahoo.com.


Coding environment
******************

Space Trader is developed in C with CodeWarrior for the Palm, version 8.3. I used the 4.0 SDKs with the latest patches which include many warnings about OS 5.0 idiosyncracies. Any issues previous versions of the code had with OS 5.0 have been resolved for this release. For version 1.2.2 of Space Trader I found the Constructor had problems, so I used the Constructor version 1.9, which comes with CodeWarrior 9. I could not transfer all source codes to CodeWarrior 9, because they started to generate errors there.

I use a tab setting of 4.

In the Space Trader project file, three targets are defined: a color version, a grayscale version and a black & white version. Each of these targets creates a PRC file with the name SpaceTrader.prc. You should make each target separately and rename it to the appropriate name. I could have specified a different output name for each target, but then you would not be able to replace one with another on your Palm. If you want to add a new picture to the game, you must add the black & white version to the MerchantBW resource file, the grayscale version to the MerchantGray resource file, and the color version to the MerchantColor resource file. You must also add a BitmapFamily for the new picture to each of these resource files, and this BitmapFamily MUST have the same name and number in each of the files. Check the other BitmapFamilies for examples. 

The current code supports Palm OS 2.0 and higher. I have not used any functions that didn't exist in OS version 2.0. Of course, the color and grayscale versions need at least OS 3.5 to run. If you update the code, be very careful using functions not defined in the OS 2.0 feature set. As soon as you use such a function, you restrict the user base considerably. Also note that several control types (like graphic buttons) and fonts (like LargeBold) didn't exist in OS 2.0.

If you create your own version of this program, you should definitely change the Creator ID, otherwise you will get into problems when Space Trader is installed on the same Palm where you install your own version. The Creator ID of Space Trader is "STra", and you need to change it inside the CodeWarrior Constructor (the ID exists in several places in the resource file and in the program code, and it must be changed in all those places).


Remarks
*******

Space Trader is my first Palm program. I am not a Palm programming wizard and haven't studied this environment very deeply. It may be that I haven't used the best way to code certain things. 

Copyright (C) 2000, 2001, 2002, 2005 by Pieter Spronck.