/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: CPMLibCommon.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 *****************************************************************************/

#ifndef _CPMLIB_COMMON_H
#define _CPMLIB_COMMON_H

#include <ErrorBase.h>
 
/** @def cpmCreator 
 * Used for both the database that contains the
 * Cryptographic Provider Manager Library and it's preferences
 * database.  
 */
#define cpmCreator 'cpml'       /* Our CPM Library creator */

/** @def cpmFtrCreator
 * Our feature creator for use with the FtrGet() call. 
 */
#define cpmFtrCreator cpmCreator

 /** @def cpmFtrNumVersion
  * This feature can be obtained to get the current version of the CPM
  * Library.
  *
  * <pre>
  * 0xMMmfsbbb, where MM is major version, m is minor version
  * f is bug fix, s is stage: 3-release,2-beta,1-alpha,0-development,
  * bbb is build number for non-releases 
  * V1.12b3   would be: 0x01122003
  * V2.00a2   would be: 0x02001002
  * V1.01     would be: 0x01013000
  * </pre>
  */
#define cpmFtrNumVersion 0

/** @def cpmErrAlreadyOpen
 * CPM Library is already open. Usually returned from a CPMLibOpen
 * indicating that the library is already open.
 */
#define cpmErrAlreadyOpen         (cpmErrorClass | 1)
/** @def cpmErrNotOpen
 * CPM Library not open. Usually returned from a CPM Library call
 * indciating that the CPM library is not yet open.
 */
#define cpmErrNotOpen             (cpmErrorClass | 2)
/** @def cpmErrStillOpen
 * CPM Library is still open after a CPMLibClose call.
 */
#define cpmErrStillOpen           (cpmErrorClass | 3)
/** @def cpmErrNoProviders
 * CPM Library is not aware of any providers. With no providers the
 * CPM Library has no functionality.
 */
#define cpmErrNoProviders         (cpmErrorClass | 4)
/** @def cpmErrNoBaseProvider
 * CPM Library cannot load the Base provider.
 */
#define cpmErrNoBaseProvider  (cpmErrorClass | 5)
/** @def cpmErrProviderNotFound
 * CPM Library cannot find the specified provider.
 */
#define cpmErrProviderNotFound    (cpmErrorClass | 6)
/** @def cpmErrParamErr
 * A CPM library call was made with an invalid parameter.
 */
#define cpmErrParamErr            (cpmErrorClass | 10)
/** @def cpmErrOutOfResources
 * The CPM Library is out of resources (memory, static heap, etc.)
 */
#define cpmErrOutOfResources      (cpmErrorClass | 11)
/** @def cpmErrOutOfMemory
 * The CPM Library is out of dynamic heap.
 */
#define cpmErrOutOfMemory         (cpmErrorClass | 12)
/** @def cpmErrBufTooSmall
 * A buffer passed into the CPM Library was too small.
 */
#define cpmErrBufTooSmall         (cpmErrorClass | 13)
/** @def cpmErrBadData
 * Data passed into the CPM Library was no good.
 */
#define cpmErrBadData             (cpmErrorClass | 14)

/** @def cpmErrUnimplemented
 * A CPM Library function is not implemented. 
 */
#define cpmErrUnimplemented       (cpmErrorClass | 15)
/** @def cpmErrUnsupported
 * A CPM Library function is unsupported in the current version.
 */
#define cpmErrUnsupported         (cpmErrorClass | 16)
/** @def cpmErrNoGlobals
 * The CPM globals could not be found for this operation
 */
#define cpmErrNoGlobals         (cpmErrorClass | 17)
/** @def cpmErrKeyExists
 * The key you are trying to import already seems to exist
 */
#define cpmErrKeyExists     (cpmErrorClass | 18)
/** @def cpmErrKeyNotFound
 * The key you are trying to export doesnt seem to exist
 */
#define cpmErrKeyNotFound   (cpmErrorClass | 19)

#define LOG_EMERG       0       /* system is unusable */
#define LOG_ALERT       1       /* action must be taken immediately */
#define LOG_CRIT        2       /* critical conditions */
#define LOG_ERR         3       /* error conditions */
#define LOG_WARNING     4       /* warning conditions */
#define LOG_NOTICE      5       /* normal but significant condition */
#define LOG_INFO        6       /* informational */
#define LOG_DEBUG       7       /* debug-level messages */

/** @def APF_MP
 * multiple part operations supported (Init, Update, FInal) else just
 * single part operation
 */
#define APF_MP               0x00000001 

/** @def APF_HW
 * provider implemented in hardware (SmartCard)
 */
#define APF_HW               0x00000002 

/** @def APF_KEYGEN
 * provider supports key generation, import and export
 */
#define APF_KEYGEN           0x00000004

/** @def APF_KEYPAIRGEN
 * provider supports key pair generation, import and export
 */
#define APF_KEYPAIRGEN       0x00000010

/** @def APF_KEYDERIVE
 * provider supports key derivation, import and export
 */
#define APF_KEYDERIVE        0x00000020

/** @def APF_HASH
 * provider supportes message digests
 */
#define APF_HASH             0x00000040

/** @def APF_CIPHER
 * provider supports encryption/decryption, import and export
 */
#define APF_CIPHER           0x00000080

/** @def APF_SIGN
 * provider supports signing
 */
#define APF_SIGN             0x00000100

/** @def APF_VERIFY
 * provider supports verification
 */
#define APF_VERIFY           0x00000200


/** @def IMPORT_EXPORT_TYPE_RAW
 * a raw form of import/export as defined by the provider 
 */
#define IMPORT_EXPORT_TYPE_RAW 0

/** @def IMPORT_EXPORT_TYPE_DER
 * a standardized ASN.1 DER encoding
 */
#define IMPORT_EXPORT_TYPE_DER 1

/** @def IMPORT_EXPORT_TYPE_XML
 * a standardized XML encoding 
 */
#define IMPORT_EXPORT_TYPE_XML 2 

/** @enum APHashEnum
 * Algorithm provider hash type enumeration
 */ 
typedef UInt32 APHashEnum;

#define   apHashTypeUnspecified             0x00L
#define   apHashTypeNone                    0x01L
#define   apHashTypeMD2                     0x02L
#define   apHashTypeMD5                     0x03L
#define   apHashTypeSHA1                    0x04L
#define   apHashTypeHAVAL                   0x05L
#define   apHashTypeRIPEMD160               0x06L
#define   apHashTypeTiger                   0x07L
#define   apHashTypePanama                  0x08L
/* SHA-2 */
#define   apHashTypeSHA256                  0x09L
#define   apHashTypeSHA384                  0x0aL
#define   apHashTypeSHA512                  0x0bL



/** @enum APAlgorithmEnum
 * Algorithm provider key type enumeration
 */
typedef UInt32 APAlgorithmEnum;

#define   apAlgorithmTypeUnspecified        0x00L
  /* block ciphers */
#define   apSymmetricTypeDES                0x01L
#define   apSymmetricTypeRC2                0x02L
#define   apSymmetricTypeRC4                0x03L
#define   apSymmetricTypeRC5                0x04L
#define   apSymmetricTypeRC6                0x05L
#define   apSymmetricTypeDESX_XDX3          0x06L
#define   apSymmetricType3DES_EDE2          0x07L
#define   apSymmetricType3DES_EDE3          0x08L
#define   apSymmetricTypeIDEA               0x09L
#define   apSymmetricTypeDiamond2           0x0aL
#define   apSymmetricTypeBlowfish           0x0bL
#define   apSymmetricTypeTEA                0x0cL
#define   apSymmetricTypeSAFER              0x0dL
#define   apSymmetricType3WAY               0x0eL
#define   apSymmetricTypeGOST               0x0fL
#define   apSymmetricTypeSHARK              0x10L
#define   apSymmetricTypeCAST128            0x11L
#define   apSymmetricTypeSquare             0x12L
#define   apSymmetricTypeSkipjack           0x13L
  /* stream ciphers */
#define   apSymmetricTypePanama             0x14L
#define   apSymmetricTypeARC4               0x15L
#define   apSymmetricTypeSEAL               0x16L
#define   apSymmetricTypeWAKE               0x17L
#define   apSymmetricTypeSapphire           0x18L
#define   apSymmetricTypeBBS                0x19L
  /* AES block ciphers */
#define   apSymmetricTypeRijndael           0x2aL
#define   apSymmetricTypeCAST256            0x2bL
#define   apSymmetricTypeTwofish            0x2cL
#define   apSymmetricTypeMARS               0x2dL
#define   apSymmetricTypeSerpent            0x2eL
  /* asymmetric key types */
#define   apAsymmetricTypeRSA               0x2fL
#define   apAsymmetricTypeDSA               0x30L
#define   apAsymmetricTypeElgamal           0x31L
#define   apAsymmetricTypeNR                0x32L /* Nyberg-Rueppel */
#define   apAsymmetricTypeBlumGoldwasser    0x33L
#define   apAsymmetricTypeRabin             0x34L
#define   apAsymmetricTypeRW                0x35L /* Rabin-Williams */
#define   apAsymmetricTypeLUC               0x36L
#define   apAsymmetricTypeLUCELG            0x37L
  /* elliptic curve */
#define   apAsymmetricTypeECDSA             0x38L
#define   apAsymmetricTypeECNR              0x39L
#define   apAsymmetricTypeECIES             0x3aL
#define   apAsymmetricTypeECDHC             0x3bL
#define   apAsymmetricTypeECMQVC            0x3cL
  /* key agreement */
#define   apKeyAgreementTypeDH              0x3dL
#define   apKeyAgreementTypeDH2             0x3eL /* Unified Diffie-Hellman */    
#define   apKeyAgreementTypeMQV             0x3fL /* Menezes-Qu-Vanstone */
#define   apKeyAgreementTypeLUCDIF          0x40L
#define   apKeyAgreementTypeXTRDH           0x41L


/** @enum APModeEnum
 * Modes of operation for symmetric encryption/decryption
 */
typedef UInt32 APModeEnum;

#define   apModeTypeUnspecified             0x00L
#define   apModeTypeNone                    0x01L
#define   apModeTypeECB                     0x02L
#define   apModeTypeCBC                     0x03L
#define   apModeTypeCBC_CTS                 0x04L
#define   apModeTypeCFB                     0x05L
#define   apModeTypeOFB                     0x06L
#define   apModeCounter                     0x07L

/** @enum APKeyUsageEnum
 * Key Usage
 */
typedef UInt32 APKeyUsageEnum;

#define   apKeyUsageUnspecified             0x00L
#define   apKeyUsageAll                     0x01L
#define   apKeyUsageSigning                 0x02L
#define   apKeyUsageEncryption              0x03L
#define   apKeyUsageCertificateSigning      0x04L
#define   apKeyUsageKeyEncrypting           0x05L
#define   apKeyUsageDataEncrypting          0x06L
#define   apKeyUsageMessageIntegrity        0x07L


/** @enum APKeyClassEnum
 * Key Class
 */
typedef UInt32 APKeyClassEnum;

#define   apKeyClassUnspecified             0x00L
#define   apKeyClassSymmetric               0x01L
#define   apKeyClassPublic                  0x02L
#define   apKeyClassPrivate                 0x03L

/** @enum APPaddingEnum
 * Padding type 
 */
typedef UInt32 APPaddingEnum;

#define   apPaddingTypeUnspecified          0x00L
#define   apPaddingTypeNone                 0x01L
#define   apPaddingTypePKCS1Type1           0x02L
#define   apPaddingTypePKCS1Type2           0x03L
#define   apPaddingTypePKCS5                0x04L
#define   apPaddingTypeOAEP                 0x05L
#define   apPaddingTypeSSLv23               0x06L

/** @enum ApKeyDerivationEnum
 * Key Derivation Type
 */
typedef UInt32 APKeyDerivationEnum;

#define   apKeyDerivationUnspecified        0x00L
#define   apKeyDerivationTypePKCS5v1        0x01L
#define   apKeyDerivationTypePKCS5v2        0x02L
#define   apKeyDerivationTypePKCS12         0x03L
#define   apKeyDerivationTypePKIX           0x04L
#define   apKeyDerivationTypeTLS            0x05L

/** @enum APKeyDerivationUsageEnum
 * KeyDerivation usage type 
 */
typedef UInt32 APKeyDerivationUsageEnum;

#define  apKeyDerivationUsageUnspecified    0x00L
#define  apKeyDerivationUsageEncryption     0x01L
#define  apKeyDerivationUsageMAC            0x02L
#define  apKeyDerivationUsageIV             0x03L

/** @struct CPMInfoStruct
 * 
 * Structure to hold information about the CPM Library as it is known
 * by the currently running system.
 */
struct CPMInfoStruct {
  /*! number of instances of this library */
  UInt8 numInstances;           
  /*! number of providers this library knows about */
  UInt8 numProviders;           
  /*! is the default provider known? */
  Boolean defaultProviderPresent; 
};

typedef struct CPMInfoStruct CPMInfoType, *CPMInfoPtr;

/** @struct APProviderInfoStruct
 * 
 * Structure to hold information about a particular provider as it is
 * known by the current instantiation of the CPM library.
 */
struct APProviderInfoStruct {
  /*! name of the provider */
  char name[32];                
  /*! other textual information */
  char other[64];            
  /*! flags to indicate the functionality supported */
  UInt32 flags;                 
  /*! number of algorithms supported */
  UInt8 numAlgorithms;          
  /*! is this a hardware provider? */
  Boolean bHardware;            
};

typedef struct APProviderInfoStruct APProviderInfoType, *APProviderInfoPtr;

/** @struct APProviderContextStruct
 * Provider specific information
 */
struct APProviderContextStruct {
  /*! the provider handling this operation */
  UInt32 providerID;            
  /*! provider specific infomation about this operation */
  void *localContext;           
};

typedef struct APProviderContextStruct APProviderContextType, *APProviderContextPtr;

/** @struct APHashInfoStruct
 * 
 * Structure to hold information about a particular hashing operation
 * including generic information about hashing operations and specific
 * information about the provider's concept of the hashing operation.
 * 
 * The application is responsible for allocating memory and
 * deallocating memory for this structure. 
 */
struct APHashInfoStruct {
  /*! the providerContext of this InfoType */
  APProviderContextType providerContext;
  /*! the type of the hash; APHashType */
  APHashEnum type;             
  /*! the length of the hash in bytes */
  UInt32 length;               
};

typedef struct APHashInfoStruct APHashInfoType, *APHashInfoPtr;

/** @struct APKeyInfoStruct
 * 
 * Structure to hold information about a particular key including
 * generic information about all keys and specific information about the
 * provider's concept of the key. 
 *
 * The application is always repsonsible for allocating and freeing
 * the APKeyInfoType. The application must also call CPMLibReleaseKeyInfo
 * before freeing the APKeyInfoType to allow the CPM and provider to
 * cleanup.
 *
 * If the application does not care or wants default settings for the
 * APKeyInfoType, the application would allocate the APKeyInfoType and
 * zero the structure. Upon return from either CPMLibImportKey or
 * CPMLibGenerateKey the APKeyInfoType structure would be filled in by
 * the CPM and provider with the appropriate information.
 *
 * If the application does care about the settings for the
 * APKeyInfoType, the application would allocate the APKeyInfoType and
 * set the fields appropriately before passing it in to
 * CPMLibGenerateKey. 
 */
struct APKeyInfoStruct {
  /*! the providerContext of this InfoType */
  APProviderContextType providerContext; 
  /*! the type of this key - APAlgorithmType */
  APAlgorithmEnum type;         
  /*! key usage - APKeyUsageType */
  APKeyUsageEnum usage;         
  /*! key class - APKeyClassType */
  APKeyClassEnum keyclass;      
  /*! e.g. 8 for DES */ 
  UInt32 length;
  /*! e.g. 7 for DES */
  UInt32 actualLength; 
  /*! is this key exportable? - 0 false, 1 true */
  UInt16 exportable;          
  /*! is this key permanent */                                  
  UInt16 ephemeral;   
};

typedef struct APKeyInfoStruct APKeyInfoType, *APKeyInfoPtr;
  
/** @struct APCipherInfoStruct
 * 
 * Structure to hold information about a particular operation's
 * context including generic information about contexts and specific
 * information about the provider's concept of the context.
 *
 * The application is always repsonsible for allocating and freeing
 * the APCipherInfoType. The application must also call
 * CPMLibReleaseContext before freeing the APCipherInfoType to allow
 * the CPM and provider(s) to cleanup.
 *
 * If the application does not care or wants default settings for the
 * APCipherInfoType the application would allocate the
 * APCipherInfoType and zero the structure. Upon return from either
 * CPMLib(Encrypt|Decrypt)[Init] or CPMLibImportCipherInfo the
 * APCipherInfoType structure would be filled in by the CPM and
 * provider with the appropriate information.
 *
 * If the application does care about the settings for the
 * APCipherInfoType, the application would allocate the
 * APCipherInfoType and set the fields appropriately before passing
 * it in to CPMLib(Encrypt|Decrypt)[Init].  
 */
struct APCipherInfoStruct {
  /*! the providerContext of this InfoType */
  APProviderContextType providerContext; 
  /*! cipher type - APAlgorithmEnum */
  APAlgorithmEnum type;       
  /*! type padding for this cipher - APPaddingEnum */
  APPaddingEnum padding;       
  /*! mode of this cipher */
  APModeEnum mode;  
  /*! initialization vector as specified by the caller */
  UInt8 *iv;                    
  /*! sizeof initialization vector */
  UInt32 ivLength;      
  /*! provider specific algorithm parameters */
  void *algorithmParams;        
};

typedef struct APCipherInfoStruct APCipherInfoType, *APCipherInfoPtr;

/** @struct APVerifyInfoStruct
 * 
 * Structure to hold information about a particular operation's
 * verification context including generic information about verification
 * contexts and specific information about the provider's concept of
 * the verification context.
 *
 * The application is always repsonsible for allocating and freeing
 * the APVerifyInfoType and the associated APCipherInfoType and
 * APHashInfoType. The application must call
 * CPMLibReleaseCipherInfo on the APCipherInfoType to allow the CPM and
 * provider(s) to cleanup the APCipherInfoType. The application must call 
 * CPMLibReleaseHashInfo on the APHashInfoType to allow the CPM and provider(s)
 * to cleanup the APHashInfoType.
 *
 * If the application does not care or wants default settings for
 * either the APCipherInfoType or the APHashInfoType, the application
 * would allocate the APCipherInfoType and the APHashInfoType and
 * zero the structures. Upon return from CPMLibVerify[Init] the
 * APCipherInfoType structure and APHashInfoType structure would be
 * filled in by the CPM and provider(s) with the appropriate information.
 *
 * If the application does care about the settings for either the
 * APCipherInfoType or the APHashInfoType, the application would
 * allocate the APCipherInfoType and the APHashInfoType and set the
 * fields appropriately before passing it in to CPMLibVerify[Init]. The
 * application may set the fields in one or the other or both. 
 */
struct APVerifyInfoStruct {
  /*! the providerContext of this InfoType */
  APProviderContextType providerContext;
  /*! hash to use */
  APHashInfoType *hashInfoP;    
  /*! cipher to use */
  APCipherInfoType *cipherInfoP; 
};

typedef struct APVerifyInfoStruct APVerifyInfoType, *APVerifyInfoPtr;

typedef UInt32 VerifyResultType, *VerifyResultPtr;
#endif /* _CPMLIB_COMMON_H */
