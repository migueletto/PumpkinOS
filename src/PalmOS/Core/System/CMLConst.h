/******************************************************************************
 *
 * Copyright (c) 1998-2003 PalmSource, Inc. All rights reserved.
 *
 * File: CMLConst.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		CML data structures definition file.
 *
 *****************************************************************************/

#ifndef  __CMLCONST_H__
#define  __CMLCONST_H__

#define	cmlEncodingTypeStrNULL				""

typedef enum {
	cmlCompressionTypeNone,
	cmlCompressionTypeBitPacked,
	cmlCompressionTypeLZ77,
	cmlCompressionTypeBest,
	cmlCompressionTypeLZ77Primer1,
	cmlCompressionTypeMax =	cmlCompressionTypeLZ77Primer1 /* end of valid range */
} CmlCompressionType;
//
#define	cmlCompressionTypeDef				cmlCompressionTypeBitPacked


#define	cmlContentTypeStrTextPlain			"text/plain"
#define	cmlContentTypeStrTextHTML			"text/html"
#define	cmlContentTypeStrImageGIF			"image/gif"
#define	cmlContentTypeStrImageJPEG			"image/jpeg"
#define	cmlContentTypeStrApplicationCml		"application/cml"
#define	cmlContentTypeStrImagePalmOS		"image/palmos"
#define	cmlContentTypeStrBinDefault			"application/octet-stream"

#define	cmlContentTypeTextPlain				0
#define	cmlContentTypeTextHTML				1
#define	cmlContentTypeImageGIF				2
#define	cmlContentTypeImageJPEG				3
#define	cmlContentTypeTextCml				4
#define	cmlContentTypeImagePalmOS			5
#define	cmlContentTypeOther					6

// Tag sizes
#define	cmlCharSize							5
#define	cmlAsciiCharSize					8
#define	cmlShortTagSize						7
#define	cmlLongTagSize						15
#define	cmlShortTagMax						127

// Size in bits of various Tag data types
#define	cmlFlagSize							1
#define	cmlColorSize						8
#define	cmlAlignSize						2
#define	cmlAlignISize						3
#define	cmlClearSize						2
#define	cmlFormatSize						4
#define	cmlListTypeSize						3
#define	cmlImageTypeSize					1
#define	cmlLinkColorTypeSize				2
#define	cmlTextSizeSize						3
#define cmlListItemSize						2
#define cmlFormSize							3
#define cmlInputTextSize					2
#define cmlInputPasswordSize				2
#define cmlInputRadioSize					4
#define cmlInputCheckBoxSize				4
#define cmlInputSubmitSize					2
#define cmlInputResetSize					1
#define cmlInputHiddenSize					2
#define cmlInputTextAreaSize				1
#define cmlSelectSize						2
#define cmlSelectItemCustomSize				2
#define cmlDatePickerSize					1
#define cmlTimePickerSize					1
#define cmlTableSize						7
#define cmlTableCellSize					7
#define cmlHyperlinkSize					10
#define cmlHorizontalRuleSize				5

// Flag values
#define cmlFlagHRCustom						0x01
#define cmlFlagHRAlign						0x06
#define cmlFlagHRNoShade					0x08
#define cmlFlagHRIsPercent					0x10

#define cmlFlagListModType					0x01
#define cmlFlagListModValue					0x02

#define cmlFlagLinkIsButton					0x01
#define cmlFlagLinkHasTitle					0x02
#define cmlFlagLinkInternal					0x04
#define cmlFlagLinkIsFragment				0x08
#define cmlFlagLinkIsSecure					0x10
#define cmlFlagLinkHasHref					0x20
#define	cmlFlagLinkIsSameDoc				0x40
#define cmlFlagLinkIsFakeRemote				0x80
#define cmlFlagLinkIsLocalRef				0x100
#define cmlFlagLinkIsBinary					0x200

#define cmlFlagFormIsStandalone				0x01
#define cmlFlagFormIsSecure					0x02
#define cmlFlagFormIsLocalAction			0x04

#define cmlFlagInputHasName					0x01
#define cmlFlagInputHasValue				0x02
#define cmlFlagInputChecked					0x04
#define cmlFlagInputHasText					0x08
#define cmlFlagInputMultiple				0x02
#define cmlFlagInputSelected				0x01

#define cmlFlagImageEmbedded				0x01
#define cmlFlagImageHasAlign				0x02
#define cmlFlagImageHasBorder				0x04
#define cmlFlagImageHasHSpace				0x08
#define cmlFlagImageHasVSpace				0x10
#define cmlFlagImageHasSrc					0x20
#define cmlFlagImageHasAlt					0x40
#define cmlFlagImageLocalPQA				0x80
#define cmlFlagImageLocalPQF				cmlFlagImageLocalPQA  /* old name; deprecated... */

#define cmlFlagTableHasAlign				0x01
#define cmlFlagTableHasWidth				0x02
#define cmlFlagTableHasBorder				0x04
#define cmlFlagTableHasCellSpacing			0x08
#define cmlFlagTableHasCellPadding			0x10
#define cmlFlagTableHasKeepRow				0x20
#define cmlFlagTableHasKeepCol				0x40

#define cmlFlagCellHasHAlign				0x01
#define cmlFlagCellHasVAlign				0x02
#define cmlFlagCellHasColSpan				0x04
#define cmlFlagCellHasRowSpan				0x08
#define cmlFlagCellHasHeight				0x10
#define cmlFlagCellHasWidth					0x20
#define cmlFlagCellNoWrap					0x40

enum CMLTag	
{
	cmlTagAnchor, 	
	cmlTagBGColor, 
	cmlTagTextColor, 
	cmlTagLinkColor, 
	cmlTagTextSize, 			
	cmlTagTextBold,
	cmlTagTextItalic, 
	cmlTagTextUnderline, 
	cmlTagParagraphAlign, 		// 0x08
	cmlTagHorizontalRule, 
	cmlTagH1, 
	cmlTagH2, 
	cmlTagH3, 		
	cmlTagH4, 
	cmlTagH5, 		
	cmlTagH6, 					
	cmlTagBlockQuote, 			// 0x10
	cmlTagHyperlink,
	cmlTagAddress,
	cmlTagTextStrike,	
	cmlTagTextMono,	
	cmlTagTextSub,
	cmlTagTextSup,		
	cmlTagClear,			
	cmlTagHistoryListText,		// 0x18
	cmlTagIsIndex, 
	cmlTagListOrdered, 
	cmlTagListUnordered, 
	cmlTagListDefinition, 
	cmlTagListItemCustom, 	
	cmlTagListItemNormal, 	
	cmlTagListItemTerm, 	
	cmlTagListItemDefinition, 	// 0x20
	cmlTagForm, 				
	cmlTagInputTextLine, 
	cmlTagInputPassword, 
	cmlTagInputRadio, 
	cmlTagInputCheckBox, 
	cmlTagInputSubmit,  		
	cmlTagInputReset, 	
	cmlTagInputHidden, 			// 0x28
	cmlTagInputTextArea,
	cmlTagSelect, 
	cmlTagSelectItemNormal, 
	cmlTagSelectItemCustom, 
	cmlTagInputDatePicker, 
	cmlTagInputTimePicker,		
	cmlTagTable, 
	cmlTagTableRow, 			// 0x30
	cmlTagCaption, 
	cmlTagTableData, 
	cmlTagTableHeader, 	
	cmlTagImage, 				// 0x34
	//cmlTagHorizontalScrollBar,

	
	// Special CML cmlTags.
	cmlTag8BitEncoding = 0x70, 		
	cmlTagCMLEnd, 	
	
	// cmlTags not passed to output
	cmlTagTextFont = 0x100, 	
	cmlTagBody,
	cmlTagBase,
	cmlTagTextBaseFont,
	cmlTagListItem,
	cmlTagInput,			
	cmlTagSelectItem,			
	cmlTagMeta,			
	cmlTagLastInList // Not used as a tag; marks the end of the enum.
};

#define cmlMaxTag cmlTagLastInList
#define cmlTagNull cmlTagLastInList
		//Used internally by CmlEncoder; not sent.

// CML 5 Bit character equates
typedef enum {
	cmlCharEnd,
	cmlCharStart,
	cmlCharEsc,
	cmlCharFormFeed,
	cmlCharLineBreak,
	cmlCharSpace,
	
	// the lower case letters 'a' - 'z' follow....
	cmlCharA
	
	} CMLCharEnum;

// Regular Ascii characters
#define	cmlAsciiCR			0x0D
#define	cmlAsciiLineFeed	0x0A
#define	cmlAsciiFormFeed	0x0C
#define	cmlAsciiSpace		0x20


// List types for ordered and unordered lists
typedef enum	{ 
	cmlListTDisc, cmlListTSquare, cmlListTCircle,
	cmlListT1, cmlListTa, cmlListTA, cmlListTi, cmlListTI 
	} CmlListEnum;
				
// Align types	 
typedef enum		{
	cmlAlignLeft, cmlAlignCenter, cmlAlignRight, cmlAlignNone
	} CmlAlignEnum;
	
typedef enum	{
	cmlVAlignTop, cmlVAlignCenter, cmlVAlignBottom, cmlVAlignNone
	} CmlVAlignEnum;

typedef enum	{
	cmlIAlignLeft, cmlIAlignCenter, cmlIAlignRight, cmlIAlignTop, 
	cmlIAlignMiddle, cmlIAlignBottom, cmlIAlignNone
	} CmlIAlignEnum;
					
typedef enum	{
	cmlClearLeft, cmlClearAll, cmlClearRight, cmlClearLine
	} CmlClearEnum;
	
typedef enum {
	cmlLinkColor, cmlLinkColorVisited, cmlLinkColorActive
	} CmlLinkColorEnum; 

#endif   //__CMLCONST_H__
