/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Bitmap.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *        This file defines bitmap structures and routines.
 *
 *****************************************************************************/

#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

#include <PalmOptErrorCheckLevel.h>		// #define ERROR_CHECK_LEVEL


//-----------------------------------------------
// The Bitmap Structure.
//-----------------------------------------------

// bitmap version numbers
#define BitmapVersionZero		0
#define BitmapVersionOne		1
#define BitmapVersionTwo		2
#define BitmapVersionThree		3

// Compression Types for BitmapVersionTwo
typedef enum BitmapCompressionTag
{
	BitmapCompressionTypeScanLine = 0,
	BitmapCompressionTypeRLE,
	BitmapCompressionTypePackBits,
	
	BitmapCompressionTypeEnd,		// must follow last compression algorithm
	
	BitmapCompressionTypeBest		= 0x64,			
	BitmapCompressionTypeNone 		= 0xFF
} BitmapCompressionType;


// pixel format defined with BitmapVersionThree
typedef enum PixelFormatTag
{
	pixelFormatIndexed,
	pixelFormat565,
	pixelFormat565LE,			// not used by 68K-based OS
	pixelFormatIndexedLE		// not used by 68K-based OS
} PixelFormatType;


// constants used by density field
typedef enum DensityTag {
	kDensityLow				= 72,
	kDensityOneAndAHalf	= 108,
	kDensityDouble			= 144,
	kDensityTriple			= 216,
	kDensityQuadruple		= 288
} DensityType;


typedef struct BitmapFlagsType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS	// These fields will not be available in the next OS release!
{
	UInt16 	compressed:1;  				// Data format:  0=raw; 1=compressed
	UInt16 	hasColorTable:1;				// if true, color table stored before bits[]
	UInt16 	hasTransparency:1;			// true if transparency is used
	UInt16 	indirect:1;						// true if bits are stored indirectly
	UInt16 	forScreen:1;					// system use only
	UInt16	directColor:1;					// direct color bitmap
	UInt16	indirectColorTable:1;		// if true, color table pointer follows BitmapType structure
	UInt16	noDither:1;						// if true, blitter does not dither
	UInt16	littleEndian:1;						// if true, pixel data is little endian
	UInt16 	reserved:7;
}
#endif
BitmapFlagsType;
	
#define kTransparencyNone	((UInt32)0xFFFFFFFF)


// -----------------------------------------------
// This is the structure of a color table. It maps pixel values into
// RGB colors. Each element in the table corresponds to the next
// index, starting at 0.
// -----------------------------------------------
typedef struct RGBColorType
{
  UInt8           index;          // index of color or best match to cur CLUT or unused.
  UInt8           r;            // amount of red, 0->255
  UInt8           g;            // amount of green, 0->255
  UInt8           b;            // amount of blue, 0->255
}
RGBColorType;


// -----------------------------------------------
// Color Table
// -----------------------------------------------
typedef struct ColorTableType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_COLORTABLES
{
  // high bits (numEntries > 256) reserved
  UInt16          numEntries;     // number of entries in table
  // RGBColorType entry[];    array 0..numEntries-1 of colors
  //                   starts immediately after numEntries
  RGBColorType entry[0];
}
#endif
ColorTableType;

// get start of color table entries aray given pointer to ColorTableType
#define ColorTableEntries(ctP)  ((RGBColorType *)((ColorTableType *)(ctP)+1))

#define BITMAP_SPACE 64

// this definition correspond to the 'Tbmp' and 'tAIB' resource types
 // Base BitmapType structure
typedef struct BitmapType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS	// These fields will not be available in the next OS release!
 	{
	UInt8 data[BITMAP_SPACE];
 	Int16  				width;
	Int16  				height;
	UInt16  				rowBytes;
	BitmapFlagsType	flags;
	UInt8					pixelSize;
	UInt8					version;					
	}
#endif
BitmapType;
typedef BitmapType* BitmapPtr;


// This data structure is the PalmOS 5 version 3 BitmapType.
typedef struct BitmapTypeV3
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS	// These fields will not be available in the next OS release!
{
	UInt8 data[BITMAP_SPACE];

	// BitmapType
	Int16  				width;
	Int16  				height;
	UInt16  				rowBytes;
	BitmapFlagsType	flags;					// see BitmapFlagsType
	UInt8					pixelSize;				// bits per pixel
	UInt8					version;					// data structure version 3
	
	// version 3 fields
	UInt8					size;						// size of this structure in bytes (0x16)
	UInt8					pixelFormat;			// format of the pixel data, see pixelFormatType
	UInt8					unused;
	UInt8					compressionType;		// see BitmapCompressionType
	UInt16				density;					// used by the blitter to scale bitmaps
	UInt32				transparentValue;		// the index or RGB value of the transparent color
	UInt32				nextBitmapOffset;		// byte offset to next bitmap in bitmap family

	// if (flags.hasColorTable)
	//		{
	//		if (flags.indirectColorTable)
	//			ColorTableType* colorTableP;	// pointer to color table
	//		else
	//	  		ColorTableType	colorTable;		// color table, could have 0 entries (2 bytes long)
	//		}
	//
	// if (flags.indirect)
	//	  	void*	  bitsP;							// pointer to actual bits
	// else
	//   	UInt8	  bits[];						// or actual bits
	//
  ColorTableType *colorTable;
  UInt8 *bits;
  UInt32 bitsSize;
  BitmapType *next;
  void *ext;
}
#endif
BitmapTypeV3;
typedef BitmapTypeV3* BitmapPtrV3;


 // Version 2 BitmapType structure
typedef struct BitmapTypeV2
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS	// These fields will not be available in the next OS release!
 	{
	UInt8 data[BITMAP_SPACE];

	// BitmapType
 	Int16  				width;
	Int16  				height;
	UInt16  				rowBytes;
	BitmapFlagsType	flags;					// (compressed, hasColorTable, hasTransparency, indirect, forScreen, directColor)
	UInt8					pixelSize;
	UInt8					version;					
	
	// version 2 fields
	UInt16	 			nextDepthOffset;		// offset in longwords
	UInt8					transparentIndex;	
	UInt8					compressionType;
	UInt16	 			reserved;
	
	// if (flags.hasColorTable)
	//	  ColorTableType	colorTable			// color table, could have 0 entries (2 bytes long)
	//
	// if (flags.directColor)
	//	  BitmapDirectInfoType	directInfo;
	// 
	// if (flags.indirect)
	//	  void*	  bitsP;							// pointer to actual bits
	// else
	//    UInt8	  bits[];						// or actual bits
	//
  ColorTableType *colorTable;
  UInt32 transparentValue; // for directColor bitmaps
  UInt8 *bits;
  UInt32 bitsSize;
  BitmapType *next;
  void *ext;
	}
#endif
BitmapTypeV2;
typedef BitmapTypeV2* BitmapPtrV2;


 // Version 1 BitmapType structure
typedef struct BitmapTypeV1
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS	// These fields will not be available in the next OS release!
 	{
	UInt8 data[BITMAP_SPACE];

	// BitmapType
 	Int16  				width;
	Int16  				height;
	UInt16  				rowBytes;
	BitmapFlagsType	flags;					// (compressed, hasColorTable)
	UInt8					pixelSize;
	UInt8					version;					
	
	// version 1 fields
	UInt16	 			nextDepthOffset;		// offset in longwords
	UInt16				reserved[2];	

  UInt8 *bits;
  UInt32 bitsSize;
  BitmapType *next;
  void *ext;
	}
#endif
BitmapTypeV1;
typedef BitmapTypeV1* BitmapPtrV1;


 // Version 0 BitmapType structure
typedef struct BitmapTypeV0
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS	// These fields will not be available in the next OS release!
 	{
	UInt8 data[BITMAP_SPACE];

	// BitmapType
 	Int16  				width;
	Int16  				height;
	UInt16  				rowBytes;
	BitmapFlagsType	flags;					// (compressed)
	
	// version 0 fields
	UInt16				reserved[4];			// pixelSize and version fields do not exist, but the reserved array
														// was initialized to 0; the OS recognizes that pixelSize of 0 means  
  UInt8 *bits;
  UInt32 bitsSize;
  BitmapType *next;
  void *ext;
	}													// the the bitmap's depth is 1
#endif
BitmapTypeV0;
typedef BitmapTypeV0* BitmapPtrV0;



// -----------------------------------------------
// For direct color bitmaps (flags.directColor set), this structure follows
//  the color table if one is present, or immediately follows the BitmapType if a
//  color table is not present. 
// The only type of direct color bitmap that is currently supported in version 3
//  of the Window Manager (feature: sysFtrCreator, #sysFtrNumWinVersion) are
//  16 bits/pixel with redBits=5, greenBits=6, blueBits=5. 
// -----------------------------------------------
typedef struct BitmapDirectInfoType
{
	UInt8		  		redBits;					// # of red bits in each pixel
	UInt8		  		greenBits;				// # of green bits in each pixel
	UInt8		  		blueBits;				// # of blue bits in each pixel
	UInt8		  		reserved;				// must be zero
	RGBColorType  transparentColor;		// transparent color (index field ignored) 
}
BitmapDirectInfoType;		  


// high density trap selectors
#define HDSelectorBmpGetNextBitmapAnyDensity		0
#define HDSelectorBmpGetVersion						1
#define HDSelectorBmpGetCompressionType				2
#define HDSelectorBmpGetDensity						3
#define HDSelectorBmpSetDensity						4
#define HDSelectorBmpGetTransparentValue			5
#define HDSelectorBmpSetTransparentValue			6
#define HDSelectorBmpCreateBitmapV3					7
#define HDSelectorWinSetCoordinateSystem			8
#define HDSelectorWinGetCoordinateSystem			9
#define HDSelectorWinScalePoint						10
#define HDSelectorWinUnscalePoint					11
#define HDSelectorWinScaleRectangle					12
#define HDSelectorWinUnscaleRectangle				13
#define HDSelectorWinScreenGetAttribute				14
#define HDSelectorWinPaintTiledBitmap				15
#define HDSelectorWinGetSupportedDensity			16
#define HDSelectorEvtGetPenNative					17
#define HDSelectorWinScaleCoord						18
#define HDSelectorWinUnscaleCoord					19
#define HDSelectorWinPaintRoundedRectangleFrame	20
#define HDSelectorWinSetScalingMode					21
#define HDSelectorWinGetScalingMode					22

#define HDSelectorInvalid							23			// leave this selector at end


// If nobody has explicitly specified whether the high density trap dispatcher 
// should be used, set it based on the emulation level.
#ifndef USE_HIGH_DENSITY_TRAPS
	#if EMULATION_LEVEL == EMULATION_NONE
		#define	USE_HIGH_DENSITY_TRAPS	1
	#else
		#define	USE_HIGH_DENSITY_TRAPS	0
	#endif
#endif

#if USE_HIGH_DENSITY_TRAPS
	#define HIGH_DENSITY_TRAP(selector)	\
		_SYSTEM_API(_CALL_WITH_SELECTOR)(_SYSTEM_TABLE, sysTrapHighDensityDispatch, selector)
#else
	#define HIGH_DENSITY_TRAP(selector)
#endif
	

#ifdef __cplusplus
extern          "C"
{
#endif

// -----------------------------------------------
// Routines relating to bitmap management       
// -----------------------------------------------

extern BitmapType* BmpCreate (Coord width, Coord height, UInt8 depth, ColorTableType * colortableP, UInt16 * error)
					SYS_TRAP (sysTrapBmpCreate);

extern Err      BmpDelete (BitmapType * bitmapP)
					SYS_TRAP (sysTrapBmpDelete);

extern Err      BmpCompress (BitmapType * bitmapP, BitmapCompressionType compType)
					SYS_TRAP (sysTrapBmpCompress);

extern void*    BmpGetBits (BitmapType * bitmapP)
					SYS_TRAP (sysTrapBmpGetBits);

extern ColorTableType* BmpGetColortable (BitmapType * bitmapP)
					SYS_TRAP (sysTrapBmpGetColortable);

extern UInt16   BmpSize (const BitmapType * bitmapP)
					SYS_TRAP (sysTrapBmpSize);

extern UInt16   BmpBitsSize (const BitmapType * bitmapP)
					SYS_TRAP (sysTrapBmpBitsSize);
					
extern void		 BmpGetSizes (const BitmapType * bitmapP, UInt32 * dataSizeP, UInt32 * headerSizeP)
					SYS_TRAP (sysTrapBmpGetSizes);

extern UInt16   BmpColortableSize (const BitmapType * bitmapP)
					SYS_TRAP (sysTrapBmpColortableSize);

extern void     BmpGetDimensions (const BitmapType * bitmapP, Coord * widthP, Coord * heightP, UInt16 * rowBytesP)
					SYS_TRAP (sysTrapBmpGetDimensions);

extern UInt8    BmpGetBitDepth (const BitmapType * bitmapP)
					SYS_TRAP (sysTrapBmpGetBitDepth);

extern BitmapType* BmpGetNextBitmap (BitmapType * bitmapP)
					SYS_TRAP (sysTrapBmpGetNextBitmap);

//-----------------------------------------------
// High Density support functions           
//-----------------------------------------------
#if EMULATION_LEVEL == EMULATION_NONE
void 			WinHighDensityDispatch();
#endif

extern BitmapType* BmpGetNextBitmapAnyDensity(BitmapType* bitmapP)
							HIGH_DENSITY_TRAP(HDSelectorBmpGetNextBitmapAnyDensity);

extern UInt8	BmpGetVersion(const BitmapType* bitmapP)
							HIGH_DENSITY_TRAP(HDSelectorBmpGetVersion);

extern BitmapCompressionType	BmpGetCompressionType(const BitmapType* bitmapP)
							HIGH_DENSITY_TRAP(HDSelectorBmpGetCompressionType);

extern UInt16	BmpGetDensity(const BitmapType* bitmapP)
							HIGH_DENSITY_TRAP(HDSelectorBmpGetDensity);

extern Err		BmpSetDensity(BitmapType* bitmapP, UInt16 density)
							HIGH_DENSITY_TRAP(HDSelectorBmpSetDensity);
							
extern Boolean	BmpGetTransparentValue(const BitmapType* bitmapP, UInt32* transparentValueP)
							HIGH_DENSITY_TRAP(HDSelectorBmpGetTransparentValue);
							
extern void		BmpSetTransparentValue(BitmapType* bitmapP, UInt32 transparentValue)
							HIGH_DENSITY_TRAP(HDSelectorBmpSetTransparentValue);
							
extern BitmapTypeV3* BmpCreateBitmapV3(const BitmapType* bitmapP, UInt16 density, const void* bitsP, const ColorTableType* colorTableP)
							HIGH_DENSITY_TRAP(HDSelectorBmpCreateBitmapV3);
							
#ifdef __cplusplus
}
#endif


#endif							// __BITMAP_H__
