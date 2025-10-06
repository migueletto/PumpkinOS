/*******************************************************************************
 * Copyright (c) 2004-2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 ******************************************************************************/
/**
 * @ingroup Codec
 */

/**
 * @file 	palmOneCodecFormat.h
 * @version 1.0
 *
 * @brief	Public 68K common header file for Codec Plugin Manager API - Codec Formats.
 *
 * This file contains the list of known codecs. The codecs listed below
 * may not be implemented yet. Each format is identified with a 4-byte
 * ID as an attempt to avoid collision and be human readable.
 * <hr>
 */

#ifndef _PALMONECODECFORMAT_H_
#define _PALMONECODECFORMAT_H_

#include <PalmTypes.h>
#include <VFSMgr.h>
#include <FileStream.h>

/**
 * @name Audio Codecs
 */
/*@{*/
#define palmCodecAudioPCM			'PCM1'	/**< Raw PCM Audio Format. */
#define palmCodecAudioAAC			'DAAC'	/**< AAC Audio Format. */
#define palmCodecAudioMP1			'MPG1'	/**< MPEG Layer 1 Audio Format. */
#define palmCodecAudioMP2			'MPG2'	/**< MPEG Layer 2 Audio Format. */
#define palmCodecAudioMP3			'MPG3'	/**< MPEG Layer 3 Audio Format. */
#define palmCodecAudioVORBIS		'OGGV'	/**< OGG Vorbis Audio Format. */
#define palmCodecAudioIMA_ADPCM		'APCM'	/**< IMA ADPCM Audio Format. */
#define palmCodecAudioDVI_ADPCM		'DPCM'	/**< Intel/MS/DVI ADPCM. */
#define palmCodecAudioALAW			'ALAW'	/**< A-Law Audio Format. */
#define palmCodecAudioULAW			'ULAW'	/**< U-Law Audio Format. */
/*@}*/

/**
 * @name Speech Codecs
 */
/*@{*/
#define palmCodecAudioAMR			'AMRS'	/**< GSM-AMR Speech Format. */
#define palmCodecAudioQCELP			'QCLP'	/**< CDMA-QCELP Speech Format. */
#define palmCodecAudioG711			'G711'	/**< G.711 Speech Format. */
#define palmCodecAudioG722			'G722'	/**< G.722 Speech Format. */
#define palmCodecAudioG726			'G726'	/**< G.726 Speech Format. */
#define palmCodecAudioG728			'G728'	/**< G.728 Speech Format. */
/*@}*/

/**
 * @name Video Codecs
 */
/*@{*/
#define palmCodecVideoMPEG4			'MPG4'	/**< MPEG-4 Video Format. */
#define palmCodecVideoMPEG1			'Mpg1'	/**< MPEG-1 Video Format. */
#define palmCodecVideoMPEG2			'Mpg2'	/**< MPEG-2 Video Format. */
#define palmCodecVideoH263			'H263'	/**< H263 Video Format. */
#define palmCodecVideoMJPEG			'MJPG'	/**< Motion JPEG Video Format. */
#define palmCodecVideoDIVX			'DivX'	/**< DivX Video Format. */
#define palmCodecVideoXVID			'XviD'	/**< XviD Video Format. */
/*@}*/

/**
 * @name Text Codecs
 */
/*@{*/
#define palmCodecText3G				'3GTX'	/**< 3G Text Format. */
/*@}*/

/**
 * @name Still Images Codecs
 */
/*@{*/
#define palmCodecImageJPEG			'JPEG'	/**< JPEG Image Format. */
#define palmCodecImageGIF87a		'GIF7'	/**< GIF87a Image Format. */
#define palmCodecImageGIF89a		'GIF9'	/**< GIF89a Image Format. */
#define palmCodecImagePNG			'PNGI'	/**< Portable Network Graphics Image Format. */
#define palmCodecImageWBMP			'WBMP'	/**< Windows Bitmap Image Format. */
#define palmCodecImageTIFF			'TIFF'	/**< TIFF Image Format. */
/*@}*/

/**
 * @name Image File Formats
 */
/*@{*/
#define palmCodecImageJPEGFile		'JPEF'	/**< JPEG Image File Format. */
#define palmCodecImageGIF87aFile	'GI7F'	/**< GIF87a Image File Format. */
#define palmCodecImageGIF89aFile	'GI9F'	/**< GIF89a Image Format. */
#define palmCodecImagePNGFile		'PNGF'	/**< Portable Network Graphics Image File Format. */
#define palmCodecImageWBMPFile		'WBMF'	/**< Windows Bitmap Image File Format. */
#define palmCodecImageTIFFFile		'TFFF'	/**< TIFF Image File Format. */
/*@}*/

#define palmCodecImageBase			'IM00'	/**< Base code for Image formats. */

/**
 * Image Formats
 */
enum
{
	palmCodecImageARGB32 = palmCodecImageBase, /**< 32 bit color with alpha channel. */
	palmCodecImageRGBA32,			/**< ('IM01') 32 bit color with alpha channel. */
	palmCodecImageRGB32,			/**< ('IM02') 32 bit RGB color without alpha channel. */
	palmCodecImageRGB888,			/**< ('IM03') 24 bit RGB color. */
	palmCodecImageRGB888Planar,		/**< ('IM04') 24 bit RGB Planar color. */
	palmCodecImageRGB565,			/**< ('IM05') 16 bit RGB color. */
	palmCodecImageRGB555,			/**< ('IM06') 16 bit RGB color + don't care. */
	palmCodecImageBGRA32,			/**< ('IM07') 32 bit BGR color with alpha channel. */
	palmCodecImageBGR32,			/**< ('IM08') 32 bit BGR color without alpha channel. */
	palmCodecImageBGR888,			/**< ('IM09') 24 bit BGR color. */
	palmCodecImageBGR565,			/**< ('IM0:') 16 bit BGR color. */
	palmCodecImageBGR555,			/**< ('IM0;') 16 bit BGR color + don't care. */
	palmCodecImageYUV444,			/**< ('IM0<') 4:4:4 YUV format. */
	palmCodecImageYUV422,			/**< ('IM0=') 4:2:2 YUV format. */
	palmCodecImageYUV420,			/**< ('IM0>') 4:2:0 YUV format. */
	palmCodecImageYUV411,			/**< ('IM0?') 4:1:1 YUV format. */
	palmCodecImageYUV211,			/**< ('IM0@') 2:1:1 YUV format. */
	palmCodecImageYUV444Planar,		/**< ('IM0A') 4:4:4 Planar YUV format. */
	palmCodecImageYUV422Planar,		/**< ('IM0B') 4:2:2 Planar YUV format. */
	palmCodecImageYUV420Planar,		/**< ('IM0C') 4:2:0 Planar YUV format. */
	palmCodecImageYUV411Planar,		/**< ('IM0D') 4:1:1 Planar YUV format. */
	palmCodecImageYUV211Planar,		/**< ('IM0E') 2:1:1 Planar YUV format. */
	palmCodecImageYCbCr444,			/**< ('IM0F') 4:4:4 YCbCr format. */
	palmCodecImageYCbCr422,			/**< ('IM0G') 4:2:2 YCbCr format. */
	palmCodecImageYCbCr420,			/**< ('IM0H') 4:2:0 YCbCr format. */
	palmCodecImageYCbCr411,			/**< ('IM0I') 4:1:1 YCbCr format. */
	palmCodecImageYCbCr211,			/**< ('IM0J') 2:1:1 YCbCr format. */
	palmCodecImageYCbCr444Planar,	/**< ('IM0K') 4:4:4 Planar YCbCr format. */
	palmCodecImageYCbCr422Planar,	/**< ('IM0L') 4:2:2 Planar YCbCr format. */
	palmCodecImageYCbCr420Planar,	/**< ('IM0M') 4:2:0 Planar YCbCr format. */
	palmCodecImageYCbCr411Planar,	/**< ('IM0N') 4:1:1 Planar YCbCr format. */
	palmCodecImageYCbCr211Planar,	/**< ('IM0O') 2:1:1 Planar YCbCr format. */
	palmCodecImageRGBIndex16,		/**< ('IM0P') Palettized 16 bit RGB. */
	palmCodecImageRGBIndex8,		/**< ('IM0Q') Palettized 8 bit RGB. */
	palmCodecImageRGBIndex4,		/**< ('IM0R') Palettized 4 bit RGB. */
	palmCodecImageRGBIndex2,		/**< ('IM0S') Palettized 2 bit RGB. */
	palmCodecImageRGBIndex1,		/**< ('IM0T') Palettized 1 bit RGB. */
	palmCodecImageRGBIndex,			/**< ('IM0U') Palettized RGB (unknowm index size). */
	palmCodecImageGRAY8,			/**< ('IM0V') Gray color 8 bit. */
	palmCodecImageGRAY4,			/**< ('IM0W') Gray color 4 bit. */
	palmCodecImageGRAY2,			/**< ('IM0X') Gray color 2 bit. */
	palmCodecImageGRAY1,			/**< ('IM0Y') Gray color 1 bit. */
	palmCodecImageGIFFrame,			/**< ('IM0Z') GIF Frame format. */
	palmCodecImageCMY,				/**< ('IM0[') Cyan Magenta Yellow format. */
	palmCodecImageCMYK,				/**< ('IM0\') Cyan Magenta Yellow BlacK format. */
	palmCodecImageHSL,				/**< ('IM0]') Hue Saturation Lightness format. */
	palmCodecImageHSI,				/**< ('IM0^') Hue Saturation Intensity format. */
	palmCodecImageHSV,				/**< ('IM0_') Hue Saturation Value format. */
	palmCodecImageHCI				/**< ('IM0`') Hue Chroma Intensity format. */
};


/**
 * Audio channels
 */
typedef enum {
	palmCodecMONO	= 0x01,	/**< One channel. */
	palmCodecSTEREO			/**< Two channels. */

} PalmAudioChannelType;

/**
 * Basic data types
 */
typedef enum {

	// Compatibility layer for sound manager
	palmCodecINT8			= 0x01,	/**< Signed 8-bit. */
	palmCodecUINT8			= 0x11,	/**< Unsigned 8-bit. */
	palmCodecINT16Big		= 0x02,	/**< Signed 16-bit big-endian. */
	palmCodecINT16Little	= 0x12,	/**< Signed 16-bit little-endian. */
	palmCodecINT32Big		= 0x04,	/**< Signed 32-bit big-endian. */
	palmCodecINT32Little	= 0x14,	/**< Signed 32-bit little-endian. */
	palmCodecFloatBig		= 0x24,	/**< Float big-endian. */
	palmCodecFloatLittle	= 0x34,	/**< Float little-endian. */

	// Extra types
	palmCodecUINT16Big,				/**< Unsigned 16-bit big-endian. */
	palmCodecUINT16Little,			/**< Unsigned 16-bit little-endian. */
	palmCodecINT24,					/**< Signed 24-bit. */
	palmCodecUINT24,				/**< Unsigned 24-bit. */
	palmCodecUINT32Big,				/**< Unsigned 32-bit big-endian. */
	palmCodecUINT32Little			/**< Unsigned 32-bit little-endian. */

} PalmBasicType;

/**
 * Endianess
 */
typedef enum {
	palmCodecBIG_ENDIAN,	/**< Big endian. */
	palmCodecLITTLE_ENDIAN	/**< Little endian. */

} PalmEndianType;


/**
 * File types
 */
typedef enum {
	palmCodecVFSFile,		/**< VFS File. */
	palmCodecStreamFile		/**< Stream File. */

} PalmFileType;

/**
 * BMP compression
 */
typedef enum {
	palmCodecBMP_None,		/**< No compression. */
	palmCodecBMP_RLE4,		/**< RLE 4bit compression. */
	palmCodecBMP_RLE8		/**< RLE 8bit compression. */

} PalmBMPCompressionType;

/**
 * AAC profiles
 */
typedef enum {
	palmCodecAACMainProfile,  /**< Main Profile */
	palmCodecAACLowComplexity, /**< Low Complexity Profile */
	palmCodecAACScaleableSamplingRate /**< SSR Profile*/

} PalmAACProfileType;

/**
 * AMR modes
 */
typedef enum {
	palmCodecAMR475,		/**<  4.75 kBits/second. */
	palmCodecAMR515,		/**<  5.15 kBits/second. */
	palmCodecAMR59,			/**<  5.9  kBits/second. */
	palmCodecAMRM67,		/**<  6.7  kBits/second. */
	palmCodecAMR74,			/**<  7.4  kBits/second. */
	palmCodecAMR795,		/**<  7.95 kBits/second. */
	palmCodecAMR102,		/**< 10.2  kBits/second. */
	palmCodecAMR122			/**< 12.2  kBits/second. */

} PalmAMRMode;

/**
 * AMR transmission settings
 */
typedef enum {
	palmCodecAMRDtxDisable,	/**< Disable discontinuous transmission mode. */
	palmCodecAMRDtxEnable	/**< Enable discontinuous transmission mode. */

} PalmAMRDtxMode;

/**
 * QCELP rates
 */
typedef enum {
	palmCodecQCELPSilent,	/**<   0 Bits per packet. */
	palmCodecQCELPEighth,	/**<  20 Bits per packet. */
	palmCodecQCELPQuarter,	/**<  54 Bits per packet. */
	palmCodecQCELPHalf,		/**< 124 Bits per packet. */
	palmCodecQCELPFull		/**< 266 Bits per packet. */

} PalmQCELPRate;

/**
 * QCELP encoder settings
 */
typedef enum {
	palmCodecQCELPReducedDisabled,	/**<  Disable reduced rate encoding. */
	palmCodecQCELPReducedEnabled	/**<  Enable reduced rate encoding. */

} PalmQCELPReducedRate;

/**
 * MPEG4 encoding algorithm
 */
typedef enum {
	palmCodecMPEG4MVFAST,	/**< Motion Vector Field Adaptation Search Technique. */
	palmCodecMPEG4SEA		/**< Successive Elimination Algorithm. */

} PalmMPEG4Algorithm;


/**
 * Custom controls / API selectors
 */
typedef enum {
	palmCodecCtlGetVersion,					/**< Get the codec version. (UInt32 in SysMakeROM style) */
	palmCodecCtlGetAuthor,					/**< Get the codec author. (Char[64]) */
	palmCodecCtlGetDate,					/**< Get the codec date. (Char[10] like "09/02/2003") */
	palmCodecCtlGetComments,				/**< Get comments about the codec. (Char[512]) */
	palmCodecCtlLastBuffer,					/**< Next EncodeDecode call will get the last buffer. */
	palmCodecCtlCustomBase	= 0x80000000	/**< Custom custom control base. */

} PalmCodecControlType;


/**
 * @brief RAW image format parameters
 */
typedef struct {
	UInt32	width;				/**< Width of the image. */
	UInt32	height;				/**< Height of the image. */
	UInt32	rowByte;			/**< Rowbyte of the image. */
	UInt32	endianess;			/**< Big Endian, Little Endian. */

} PalmImageParamType;

#define MAX_PLANE_COUNT		5	/**< Maximum number of image planes */

/**
 * @brief Planar image structure, passed during EncodeDecode
 */
typedef struct {
	UInt32	width;				/**< Width of the image. */
	UInt32	height;				/**< Height of the image. */

	// Plane info
	void	*planeP[MAX_PLANE_COUNT];	/**< Image planes. */
	UInt32	planeWidth[MAX_PLANE_COUNT];/**< Width of each plane. */
	UInt32	planeCount;					/**< Number of planes used. */

} PalmImagePlanarType;

/**
 * @brief PCM format parameters
 */
typedef struct {
	UInt32	sampleRate;			/**< Sample rate (44100, 32000...). */
	UInt32	sampleType;			/**< Sample type (use the one one from Sound Manager). */
	UInt32	sampleWidth;		/**< Sample width (mono/stereo). */

} PalmAudioPCMParamType;

/**
 * @brief IMA-ADPCM format parameters
 */
typedef struct {
	UInt32	sampleRate;			/**< Sample rate (44100, 32000...). */
	UInt32	sampleWidth;		/**< Sample width (mono/stereo). */
	UInt32	blockSize;			/**< Block Size. */

} PalmAudioADPCMParamType;

/**
 * @brief AAC format parameters
 */
typedef struct {
	UInt32	bitRate;			/**< Bits per second. */
	UInt32	sampleRate;			/**< Sample rate (44100, 32000...). */
	UInt32	sampleWidth;		/**< Sample width (mono/stereo). */
	UInt32	profile;			/**< Profile used if any. */

} PalmAudioAACParamType;

/*
 * @brief AMR format encoding parameters
 */
typedef struct {
	UInt32	mode;				/**< Encoding mode (i.e. kBits/second). */
	UInt32	dtxMode;			/**< Discontinuous transmission mode. */

} PalmAudioAMRParamType;

/*
 * @brief QCELP format encoding parameters
 */
typedef struct {
	UInt32	maxRate;			/**< Maximum rate. */
	UInt32	minRate;			/**< Minimum rate. */
	UInt32	reducedRate;		/**< Reduced rate flag. */

} PalmAudioQCELPParamType;

/*
 * @brief QCELP format decoding parameter
 */
typedef struct {
	UInt32	headerSize;			/**< size of RIFF header. */
	UInt32	headerP;			/**< RIFF header. */

} PalmAudioQCELPDecParamType;

/**
 * @brief MPEG-1 audio decoding parameters
 */
typedef struct {
	UInt32 avgBytesPerSec;
	UInt32 bitsPerSample;
	UInt32 channels;
	UInt32 sampleRate;

} PalmAudioMP2ParamType;

/**
 * @brief File format parameters
 */
typedef struct {
	UInt32		fileLocationType;	/**< File location type. */
	FileRef		vfsFileRef;			/**< File Reference for VFS type. */
	FileHand	streamFileHandle;	/**< File Handle for File Stream type. */

} PalmFileFormatParamType;


/**
 * @brief BMP format parameters
 */
typedef struct {
	UInt32	xPixelsPerMeter;	/**< Horizontal resolution. */
	UInt32	yPixelsPerMeter;	/**< Vertical resolution. */
	UInt32	colorUsed;			/**< Number of color indices actually used. */
	UInt32	colorImportant;		/**< Number of color indices actually required. 0 = all colors required. */
	UInt32	compression;		/**< Bitmap compression. */

} PalmImageBMPParamType;


/**
 * @brief JPEG format encoding parameters
 */
typedef struct {
	UInt32	quality;			/**< JPEG quality between 0 and 100. */
	UInt32 	restartInterval;	/**< Number of restart intervals. */
	UInt32	JPEGMode;			/**< Baseline/Progressive... */
	UInt32	subSampling;		/**< SubSampling YCbCr (444, 422, 411). */

} PalmImageJPEGEncodeParamType;

/**
 * @brief GIF frame parameters
 *
 * Each GIF frame contains more info than a simple palette and image buffer.
 */
typedef struct {
	UInt32	leftPosition;			/**< Left position of the image. */
	UInt32	topPosition;			/**< Top posistion of the image. */
	UInt32	imageWidth;				/**< Image width. */
	UInt32	imageHeight;			/**< Image height. */
	UInt32	localColorBits;			/**< Number of bits for the local color table. */
	UInt32	hasLocalColorTable;		/**< 1 if has local color table, 0 otherwise. */
	UInt32	isInterlaced;			/**< 1 if interlaced, 0 otherwise. */
	UInt32	hasTransparentColor;	/**< 1 if has transparency, 0 otherwise. */
	UInt32	transparentColorIndex;	/**< Transparency color index. */
	UInt32	disposalMethod;			/**< Disposal method. (Refer to GIF standard)*/
	UInt32	delayTime;				/**< Delay for the current image. */
	UInt8	localColorTable[256*3];	/**< Local color table. */
	UInt32	imageDataSize;			/**< Size of the image buffer. */
	UInt8	*imageDataP;			/**< Raw Image. */

} PalmImageGIFFrameParamType;

/**
 * @brief Index image parameters
 */
typedef struct {
	UInt32	width;				/**< Width of the image. */
	UInt32	height;				/**< Height of the image. */
	UInt32	rowByte;			/**< Rowbyte of the image. */
	UInt32	endianess;			/**< Big Endian, Little Endian. */
	UInt32	colorBits;			/**< Number of bits for the color table. */
	UInt8	*colorTable;		/**< Color table. Should have the size 4*2^(colorBits). */
	UInt8	*imageDataP;		/**< Raw Image data. */

} PalmImageINDEXParamType;

/**
 * @brief MPEG-1 video decoding parameters
 */
typedef struct {
	UInt32	width;			/**< Width. */
	UInt32	height;			/**< Height. */
	UInt32	skipBFrames;	/**< B Frame are very time consuming. If True skip B-Frames. */

} PalmVideoMPEG1DecodeParamType;

/**
 * @brief MPEG-4 video format encoding parameters
 */
typedef struct {
	UInt32	profile;			/**< Profile used if any. */
	UInt32	volWidth;			/**< Video Object Layer width. */
	UInt32	volHeight;			/**< Video Object Layer height. */
	UInt32	keyFrame;			/**< Number of P-Frames between two I-Frames. */
	UInt32	frameRate;			/**< Number of frames per second. */
	UInt32	bitRate;			/**< Bit rate target. */
	UInt32	algorithm;			/**< MV_FAST, SEA... */
	UInt32	IVOPQuantization;	/**< Quantization step in I-VOP frames. */
	UInt32	VOVOLVersionID;		/**< Version ID for VO & VOL header. */
	UInt32	searchRange;		/**< Motion search range (in pixels). */

} PalmVideoMPEG4EncodeParamType;

/**
 * @brief MPEG-4 video format decoding parameters
 */
typedef struct {
	UInt32	profile;			/**< Profile used if any. */
	UInt32	volWidth;			/**< Video Object Layer width. */
	UInt32	volHeight;			/**< Video Object Layer width. */
} PalmVideoMPEG4DecodeParamType;


#endif  // _PALMONECODECFORMAT_H_
