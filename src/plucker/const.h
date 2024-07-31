/*
 * $Id: const.h,v 1.53 2004/04/18 15:34:47 prussar Exp $
 *
 * Viewer - a part of Plucker, the free off-line HTML viewer for PalmOS
 * Copyright (c) 1998-2002, Mark Ian Lillywhite and Michael Nordstrom
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

/* Default spacing between paragraphs */
#define DEFAULT_PARAGRAPH_SPACING   2

/* Default size of horizontal rules */
#define DEFAULT_HRULE_SIZE          2

/* Max length of search pattern */
#define MAX_PATTERN_LEN             40

/* Max number of search terms */
#define MAX_SEARCH_TERMS            10

/* Document version */
#define ViewerDocumentVersion       1

/* Meta Document version */
#define MetaDocumentVersion         7

/* Document list version */
#define PlkrDocListVersion          4

/* Creator ID */
#define ViewerAppID                 'Plkr'

/* Skins resource type */
#define SkinResourceType            'Skin'

/* User font resource type */
#define UserFontResourceType        'Font'

/* Type for Plucker documents */
#define ViewerDocumentType          'Data'

/* Type for meta documents */
#define MetaDocumentType            'Meta'

/* Type for Uncompress buffer */
#define UncompressType              'Ucmp'

/* Type for Cache database */
#define CacheDBType                 'Cach'

/* Type for document list */
#define PlkrDocListType             'List'

/* Type for keyboard map db */
#define PlkrKeyboardMapType         'Keyb'

/* Type for keyboard map db */
#define PlkrVFSFontCacheType        'VFnC'

/* Type for transliteration database */
#define XlitDBType                  'Xlit'

/* Type for armlet resources */
#define ArmletResourceType          'armc'

/* Type for I-mode icon db */
#define PlkrImodeType               'Imod'

/* Type for internal MemoDB */
#define PlkrMemoDBType              'Memo'

/* Length of OwnerID buffer */
#define OWNER_ID_HASH_LEN           40

/* Acer S50/S60 manufacturer ID */
#define acerS50OrS60ManufacturerID  'acer'

/* Acer S50/S60 device ID */
#define acerS50OrS60DeviceID        'coco'

/* E0 E1 is a single character in Big5, GB2312, EUC-JP, EUC-KR */
#define testDoubleByteBig5GB2312EUCJPKR   0xE0E1

/* 82 BE is a single character in Shift-JIS */
#define testDoubleByteShiftJIS            0x82BE

/* 3B 22 is a single character in JIS and Kuten */
#define testDoubleByteJISKuten            0x3B22

/* Preference ID -- use 0 for test version */
#define ViewerPrefID                4

/* version number of the preferences */
#define ViewerVersion               3

/* Preference ID for search patterns */
#define ViewerPrefSearchStringID    3

/* Preference ID for DIA settings */
#define ViewerPrefDIA               5

/* Old preference ID */
#define ViewerOldPrefID             2


