/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SslLibAsn1.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 *****************************************************************************/

#ifndef __SslLibAsn1_h__
#define __SslLibAsn1_h__

/* More ASN.1 Information can be found at
 * http://www.itu.int/ITU-T/asn1/
 */

#define asn1Eoc			0
#define asn1Boolean		1
#define asn1Integer		2
#define asn1BitString		3
#define asn1OctetString		4
#define asn1Null		5
#define asn1Object		6
#define asn1ObjectDescriptor	7
#define asn1External		8
#define asn1Real		9
#define asn1Enumerated		10
#define asn1EmbeddedPdv		11
#define asn1Utf8String		12
#define asn1Sequence		16
#define asn1Set			17
#define asn1NumericString	18
#define asn1PrintableString	19
#define asn1T61String		20
#define asn1TeletexString	20
#define asn1VideotexString	21
#define asn1Ia5String		22
#define asn1UtcTime		23
#define asn1GeneralizedTime	24
#define asn1GraphicString	25
#define asn1Iso64String		26
#define asn1VisibleString	26
#define asn1GeneralString	27
#define asn1UniversalString	28
#define asn1BmpString		30

#define asn1ExItemTypeX509	32
#define asn1FldX509Version	 2 /* X509 version number */
#define asn1FldX509SerialNumber	 3 /* Serial number */
#define asn1FldX509IssuerRdn	 4 /* The issuer name blob */
#define asn1FldX509SubjectRdn	 5 /* The subject name blob */
#define asn1FldX509NotBefore	 6 /* NotBefore data blob */
#define asn1FldX509NotAfter	 7 /* NotAfter data blob */
#define asn1FldX509PubKeyBody	 8 /* The pub key stuff */
#define asn1FldX509PubKeyOid	 9 /* The OID for the public key type */
#define asn1FldX509PubKeyParams	10 /* The OID for the public key type */
#define asn1FldX509PubKey	11 /* The public key blob */
#define asn1FldX509SignatureOid	12 /* The OID for the signature algorithm*/
#define asn1FldX509SignatureParams 13
#define asn1FldX509Signature	14 /* The signature blob */
#define asn1FldX509CertIssuerId	15
#define asn1FldX509CertSubjectId 16
#define asn1FldX509IssuerUniqueIdentifier 21
#define asn1FldX509SubjectUniqueIdentifier 22
#define asn1FldX509Extensions	23 /* X509v3 extension blob */

#define asn1ExItemTypeRSA		33
#define asn1FldRsaNumPrimes		0x0001 /* Num in len, not data bytes */
#define asn1FldRsaN			0x0010
#define asn1FldRsaE			0x0011
#define asn1FldRsaD			0x0012
#define asn1FldRsaPrime(n)		(0x0013+((n)*3))
#define asn1FldRsaExp(n)		(0x0014+((n)*3))
#define asn1FldRsaInv(n)		(0x0015+((n)*3))
/* Note that P and Q are swapped */
#define asn1FldRsaQ			asn1FldRsaPrime(0)
#define asn1FldRsaDmq1			asn1FldRsaExp(0)
#define asn1FldRsaP			asn1FldRsaPrime(1)
#define asn1FldRsaDmp1			asn1FldRsaExp(1)
#define asn1FldRsaIqmp			asn1FldRsaInv(1)

#define asn1ExItemTypeRdn		34
/* 4n+0 values are OID's, rn+1 values are the relevent data item
 * The values will be decoded into sequention locations
 */
#define asn1FldRdnOid			4
#define asn1FldRdnValue			5
#define asn1FldRdnOidN(n)		(R_RDN_OID+(n)*4)
#define asn1FldRdnValueN(n)		(R_RDN_OID+(n)*4+1)

#define asn1ExItemTypeX509Ex		35
#define asn1FldX509ExOid	 	1 /* First extension OID */
#define asn1FldX509ExCritical		2 /* First extension critical flag */
#define asn1FldX509ExBytes	 	3 /* First extension bytes */
#define asn1FldX509ExOidN(n) 		(1+(n*3)) /* N'th OID */
#define asn1FldX509ExCriticalN(n)	(2+(n*3)) /* N'th critical flag */
#define asn1FldX509ExBytesN(n)	 	(3+(n*3)) /* N'th data bytes */

#define asn1ExItemTypeX509ExData	36
#define asn1FldX509ExBasicConstraintsCa                   1
/* Value is in the length, not the data bytes */
#define asn1FldX509ExBasicConstraintsPathLenConstraint    2

typedef struct Asn1OidBer_st
	{
	UInt16 length;
	UInt8 *data;
	} Asn1OidBer;


/**********************************************/
/* ASN.1 Object identifiers for RDN */
/**********************************************/

#define asn1Len_commonName 5
#define asn1Str_commonName \
	(unsigned char *)"\006\003\125\004\003"
#define asn1Ary_commonName \
	{0x06,0x03,0x55,0x04,0x03}
#define asn1OidBer_commonName \
	{asn1Len_commonName,asn1Str_commonName}

#define asn1Len_surnameName 5
#define asn1Str_surnameName \
	(unsigned char *)"\006\003\125\004\004"
#define asn1Ary_surnameName \
	{0x06,0x03,0x55,0x04,0x04}
#define asn1OidBer_surnameName \
	{asn1Len_surnameName,asn1Str_surnameName}

#define asn1Len_serialNumber 5
#define asn1Str_serialNumber \
	(unsigned char *)"\006\003\125\004\005"
#define asn1Ary_serialNumber \
	{0x06,0x03,0x55,0x04,0x05}
#define asn1OidBer_serialNumber \
	{asn1Len_serialNumber,asn1Str_serialNumber}

#define asn1Len_countryName 5
#define asn1Str_countryName \
	(unsigned char *)"\006\003\125\004\006"
#define asn1Ary_countryName \
	{0x06,0x03,0x55,0x04,0x06}
#define asn1OidBer_countryName \
	{asn1Len_countryName,asn1Str_countryName}

#define asn1Len_localityName 5
#define asn1Str_localityName \
	(unsigned char *)"\006\003\125\004\007"
#define asn1Ary_localityName \
	{0x06,0x03,0x55,0x04,0x07}
#define asn1OidBer_localityName \
	{asn1Len_localityName,asn1Str_localityName}

#define asn1Len_stateOrProvinceName 5
#define asn1Str_stateOrProvinceName \
	(unsigned char *)"\006\003\125\004\010"
#define asn1Ary_stateOrProvinceName \
	{0x06,0x03,0x55,0x04,0x08}
#define asn1OidBer_stateOrProvinceName \
	{asn1Len_stateOrProvinceName,asn1Str_stateOrProvinceName}

#define asn1Len_organizationName 5
#define asn1Str_organizationName \
	(unsigned char *)"\006\003\125\004\012"
#define asn1Ary_organizationName \
	{0x06,0x03,0x55,0x04,0x0a}
#define asn1OidBer_organizationName \
	{asn1Len_organizationName,asn1Str_organizationName}

#define asn1Len_organizationUnitName 5
#define asn1Str_organizationUnitName \
	(unsigned char *)"\006\003\125\004\013"
#define asn1Ary_organizationUnitName \
	{0x06,0x03,0x55,0x04,0x0b}
#define asn1OidBer_organizationUnitName \
	{asn1Len_organizationUnitName,asn1Str_organizationUnitName}

#define asn1Len_title 5
#define asn1Str_title \
	(unsigned char *)"\006\003\125\004\014"
#define asn1Ary_title \
	{0x06,0x03,0x55,0x04,0x0c}
#define asn1OidBer_title \
	{asn1Len_title,asn1Str_title}

#define asn1Len_description 5
#define asn1Str_description \
	(unsigned char *)"\006\003\125\004\015"
#define asn1Ary_description \
	{0x06,0x03,0x55,0x04,0x0d}
#define asn1OidBer_description \
	{asn1Len_description,asn1Str_description}

#define asn1Len_emailAddress 11
#define asn1Str_emailAddress \
	(unsigned char *)"\006\011\052\206\110\206\367\015\001\011\001"
#define asn1Ary_emailAddress \
	{0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x09,0x01}
#define asn1OidBer_emailAddress \
	{asn1Len_emailAddress,asn1Str_emailAddress}

#define asn1Len_name 5
#define asn1Str_name \
	(unsigned char *)"\006\003\125\004\051"
#define asn1Ary_name \
	{0x06,0x03,0x55,0x04,0x29}
#define asn1OidBer_name \
	{asn1Len_name,asn1Str_name}

#define asn1Len_givenName 5
#define asn1Str_givenName \
	(unsigned char *)"\006\003\125\004\052"
#define asn1Ary_givenName \
	{0x06,0x03,0x55,0x04,0x2a}
#define asn1OidBer_givenName \
	{asn1Len_givenName,asn1Str_givenName}

#define asn1Len_initials 5
#define asn1Str_initials \
	(unsigned char *)"\006\003\125\004\053"
#define asn1Ary_initials \
	{0x06,0x03,0x55,0x04,0x2b}
#define asn1OidBer_initials \
	{asn1Len_initials,asn1Str_initials}

#define asn1Len_uniqueIdentifier 5
#define asn1Str_uniqueIdentifier \
	(unsigned char *)"\006\003\125\004\055"
#define asn1Ary_uniqueIdentifier \
	{0x06,0x03,0x55,0x04,0x2d}
#define asn1OidBer_uniqueIdentifier \
	{asn1Len_uniqueIdentifier,asn1Str_uniqueIdentifier}

#define asn1Len_dnQualifier 5
#define asn1Str_dnQualifier \
	(unsigned char *)"\006\003\125\004\056"
#define asn1Ary_dnQualifier \
	{0x06,0x03,0x55,0x04,0x2e}
#define asn1OidBer_dnQualifier \
	{asn1Len_dnQualifier,asn1Str_dnQualifier}

/**********************************************/
/* ASN.1 Oids assoicated with PKCS1/RSA */
/**********************************************/

#define asn1Len_rsaEncryption 11
#define asn1Str_rsaEncryption \
	(unsigned char *)"\006\011\052\206\110\206\367\015\001\001\001"
#define asn1Ary_rsaEncryption \
	{0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x01}
#define asn1OidBer_rsaEncryption \
	{asn1Len_rsaEncryption,asn1Str_rsaEncryption}

#define asn1Len_md2WithRSAEncryption 11
#define asn1Str_md2WithRSAEncryption \
	(unsigned char *)"\006\011\052\206\110\206\367\015\001\001\002"
#define asn1Ary_md2WithRSAEncryption \
	{0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x02}
#define asn1OidBer_md2WithRSAEncryption \
	{asn1Len_md2WithRSAEncryption,asn1Str_md2WithRSAEncryption}

#define asn1Len_md5WithRSAEncryption 11
#define asn1Str_md5WithRSAEncryption \
	(unsigned char *)"\006\011\052\206\110\206\367\015\001\001\004"
#define asn1Ary_md5WithRSAEncryption \
	{0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x04}
#define asn1OidBer_md5WithRSAEncryption \
	{asn1Len_md5WithRSAEncryption,asn1Str_md5WithRSAEncryption}

#define asn1Len_sha1WithRSAEncryption 11
#define asn1Str_sha1WithRSAEncryption \
	(unsigned char *)"\006\011\052\206\110\206\367\015\001\001\005"
#define asn1Ary_sha1WithRSAEncryption \
	{0x06,0x09,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x01,0x05}
#define asn1OidBer_sha1WithRSAEncryption \
	{asn1Len_sha1WithRSAEncryption,asn1Str_sha1WithRSAEncryption}

#define asn1Len_md5WithRSA 7
#define asn1Str_md5WithRSA \
	(unsigned char *)"\006\005\053\016\003\002\003"
#define asn1Ary_md5WithRSA \
	{0x06,0x05,0x2b,0x0e,0x03,0x02,0x03}
#define asn1OidBer_md5WithRSA \
	{asn1Len_md5WithRSA,asn1Str_md5WithRSA}

#define asn1Len_sha1WithRSA 7
#define asn1Str_sha1WithRSA \
	(unsigned char *)"\006\005\053\016\003\002\035"
#define asn1Ary_sha1WithRSA \
	{0x06,0x05,0x2b,0x0e,0x03,0x02,0x1d}
#define asn1OidBer_sha1WithRSA \
	{asn1Len_sha1WithRSA,asn1Str_sha1WithRSA}


/**********************************************/
/* ASN.1 Oids associated with X509 extensions */
/**********************************************/

#define asn1Len_subjectDirectoryAttributes 5
#define asn1Str_subjectDirectoryAttributes \
	(unsigned char *)"\006\003\125\035\011"
#define asn1Ary_subjectDirectoryAttributes \
	{0x06,0x03,0x55,0x1d,0x09}
#define asn1OidBer_subjectDirectoryAttributes \
	{asn1Len_subjectDirectoryAttributes,asn1Str_subjectDirectoryAttributes}

#define asn1Len_subjectKeyIdentifier 5
#define asn1Str_subjectKeyIdentifier \
	(unsigned char *)"\006\003\125\035\016"
#define asn1Ary_subjectKeyIdentifier \
	{0x06,0x03,0x55,0x1d,0x0e}
#define asn1OidBer_subjectKeyIdentifier \
	{asn1Len_subjectKeyIdentifier,asn1Str_subjectKeyIdentifier}

#define asn1Len_keyUsage 5
#define asn1Str_keyUsage \
	(unsigned char *)"\006\003\125\035\017"
#define asn1Ary_keyUsage \
	{0x06,0x03,0x55,0x1d,0x0f}
#define asn1OidBer_keyUsage \
	{asn1Len_keyUsage,asn1Str_keyUsage}

#define asn1Len_privateKeyUsagePeriod 5
#define asn1Str_privateKeyUsagePeriod \
	(unsigned char *)"\006\003\125\035\020"
#define asn1Ary_privateKeyUsagePeriod \
	{0x06,0x03,0x55,0x1d,0x10}
#define asn1OidBer_privateKeyUsagePeriod \
	{asn1Len_privateKeyUsagePeriod,asn1Str_privateKeyUsagePeriod}

#define asn1Len_subjectAltName 5
#define asn1Str_subjectAltName \
	(unsigned char *)"\006\003\125\035\021"
#define asn1Ary_subjectAltName \
	{0x06,0x03,0x55,0x1d,0x11}
#define asn1OidBer_subjectAltName \
	{asn1Len_subjectAltName,asn1Str_subjectAltName}

#define asn1Len_issuerAltName 5
#define asn1Str_issuerAltName \
	(unsigned char *)"\006\003\125\035\022"
#define asn1Ary_issuerAltName \
	{0x06,0x03,0x55,0x1d,0x12}
#define asn1OidBer_issuerAltName \
	{asn1Len_issuerAltName,asn1Str_issuerAltName}

#define asn1Len_basicConstraints 5
#define asn1Str_basicConstraints \
	(unsigned char *)"\006\003\125\035\023"
#define asn1Ary_basicConstraints \
	{0x06,0x03,0x55,0x1d,0x13}
#define asn1OidBer_basicConstraints \
	{asn1Len_basicConstraints,asn1Str_basicConstraints}

#define asn1Len_cRLNumber 5
#define asn1Str_cRLNumber \
	(unsigned char *)"\006\003\125\035\024"
#define asn1Ary_cRLNumber \
	{0x06,0x03,0x55,0x1d,0x14}
#define asn1OidBer_cRLNumber \
	{asn1Len_cRLNumber,asn1Str_cRLNumber}

#define asn1Len_reasonCode 5
#define asn1Str_reasonCode \
	(unsigned char *)"\006\003\125\035\025"
#define asn1Ary_reasonCode \
	{0x06,0x03,0x55,0x1d,0x15}
#define asn1OidBer_reasonCode \
	{asn1Len_reasonCode,asn1Str_reasonCode}

#define asn1Len_instructionCode 5
#define asn1Str_instructionCode \
	(unsigned char *)"\006\003\125\035\027"
#define asn1Ary_instructionCode \
	{0x06,0x03,0x55,0x1d,0x17}
#define asn1OidBer_instructionCode \
	{asn1Len_instructionCode,asn1Str_instructionCode}

#define asn1Len_invalidityDate 5
#define asn1Str_invalidityDate \
	(unsigned char *)"\006\003\125\035\030"
#define asn1Ary_invalidityDate \
	{0x06,0x03,0x55,0x1d,0x18}
#define asn1OidBer_invalidityDate \
	{asn1Len_invalidityDate,asn1Str_invalidityDate}

#define asn1Len_deltaCRLIndicator 5
#define asn1Str_deltaCRLIndicator \
	(unsigned char *)"\006\003\125\035\033"
#define asn1Ary_deltaCRLIndicator \
	{0x06,0x03,0x55,0x1d,0x1b}
#define asn1OidBer_deltaCRLIndicator \
	{asn1Len_deltaCRLIndicator,asn1Str_deltaCRLIndicator}

#define asn1Len_issuingDistributionPoint 5
#define asn1Str_issuingDistributionPoint \
	(unsigned char *)"\006\003\125\035\034"
#define asn1Ary_issuingDistributionPoint \
	{0x06,0x03,0x55,0x1d,0x1c}
#define asn1OidBer_issuingDistributionPoint \
	{asn1Len_issuingDistributionPoint,asn1Str_issuingDistributionPoint}

#define asn1Len_certificateIssuer 5
#define asn1Str_certificateIssuer \
	(unsigned char *)"\006\003\125\035\035"
#define asn1Ary_certificateIssuer \
	{0x06,0x03,0x55,0x1d,0x1d}
#define asn1OidBer_certificateIssuer \
	{asn1Len_certificateIssuer,asn1Str_certificateIssuer}

#define asn1Len_nameConstraints 5
#define asn1Str_nameConstraints \
	(unsigned char *)"\006\003\125\035\036"
#define asn1Ary_nameConstraints \
	{0x06,0x03,0x55,0x1d,0x1e}
#define asn1OidBer_nameConstraints \
	{asn1Len_nameConstraints,asn1Str_nameConstraints}

#define asn1Len_cRLDistributionPoints 5
#define asn1Str_cRLDistributionPoints \
	(unsigned char *)"\006\003\125\035\037"
#define asn1Ary_cRLDistributionPoints \
	{0x06,0x03,0x55,0x1d,0x1f}
#define asn1OidBer_cRLDistributionPoints \
	{asn1Len_cRLDistributionPoints,asn1Str_cRLDistributionPoints}

#define asn1Len_certificatePolicies 5
#define asn1Str_certificatePolicies \
	(unsigned char *)"\006\003\125\035\040"
#define asn1Ary_certificatePolicies \
	{0x06,0x03,0x55,0x1d,0x20}
#define asn1OidBer_certificatePolicies \
	{asn1Len_certificatePolicies,asn1Str_certificatePolicies}

#define asn1Len_policyMappings 5
#define asn1Str_policyMappings \
	(unsigned char *)"\006\003\125\035\041"
#define asn1Ary_policyMappings \
	{0x06,0x03,0x55,0x1d,0x21}
#define asn1OidBer_policyMappings \
	{asn1Len_policyMappings,asn1Str_policyMappings}

#define asn1Len_authorityKeyIdentifier 5
#define asn1Str_authorityKeyIdentifier \
	(unsigned char *)"\006\003\125\035\043"
#define asn1Ary_authorityKeyIdentifier \
	{0x06,0x03,0x55,0x1d,0x23}
#define asn1OidBer_authorityKeyIdentifier \
	{asn1Len_authorityKeyIdentifier,asn1Str_authorityKeyIdentifier}

#define asn1Len_policyConstraints 5
#define asn1Str_policyConstraints \
	(unsigned char *)"\006\003\125\035\044"
#define asn1Ary_policyConstraints \
	{0x06,0x03,0x55,0x1d,0x24}
#define asn1OidBer_policyConstraints \
	{asn1Len_policyConstraints,asn1Str_policyConstraints}

#define asn1Len_extKeyUsage 5
#define asn1Str_extKeyUsage \
	(unsigned char *)"\006\003\125\035\045"
#define asn1Ary_extKeyUsage \
	{0x06,0x03,0x55,0x1d,0x25}
#define asn1OidBer_extKeyUsage \
	{asn1Len_extKeyUsage,asn1Str_extKeyUsage}

#define asn1Len_microsoftIndividualCodeSigning 12
#define asn1Str_microsoftIndividualCodeSigning \
	(unsigned char *)"\006\012\053\006\001\004\001\202\067\002\001\025"
#define asn1Ary_microsoftIndividualCodeSigning \
	{0x06,0x0A,0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x02,0x01,0x15}
#define asn1OidBer_microsoftIndividualCodeSigning \
	{asn1Len_microsoftIndividualCodeSigning,asn1Str_microsoftIndividualCodeSigning}

#define asn1Len_microsoftCommercialCodeSigning 12
#define asn1Str_microsoftCommercialCodeSigning \
	(unsigned char *)"\006\012\053\006\001\004\001\202\067\002\001\026"
#define asn1Ary_microsoftCommercialCodeSigning \
	{0x06,0x0A,0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x02,0x01,0x16}
#define asn1OidBer_microsoftCommercialCodeSigning \
	{asn1Len_microsoftCommercialCodeSigning,asn1Str_microsoftCommercialCodeSigning}

#define asn1Len_microsoftTrustListSigning 12
#define asn1Str_microsoftTrustListSigning \
	(unsigned char *)"\006\012\053\006\001\004\001\202\067\012\003\001"
#define asn1Ary_microsoftTrustListSigning \
	{0x06,0x0A,0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x0a,0x03,0x01}
#define asn1OidBer_microsoftTrustListSigning \
	{asn1Len_microsoftTrustListSigning,asn1Str_microsoftTrustListSigning}

#define asn1Len_microsoftServerGatedCrypto 12
#define asn1Str_microsoftServerGatedCrypto \
	(unsigned char *)"\006\012\053\006\001\004\001\202\067\012\003\003"
#define asn1Ary_microsoftServerGatedCrypto \
	{0x06,0x0A,0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x0a,0x03,0x03}
#define asn1OidBer_microsoftServerGatedCrypto \
	{asn1Len_microsoftServerGatedCrypto,asn1Str_microsoftServerGatedCrypto}

#define asn1Len_microsoftEncryptedFileSystem 12
#define asn1Str_microsoftEncryptedFileSystem \
	(unsigned char *)"\006\012\053\006\001\004\001\202\067\012\003\004"
#define asn1Ary_microsoftEncryptedFileSystem \
	{0x06,0x0A,0x2b,0x06,0x01,0x04,0x01,0x82,0x37,0x0a,0x03,0x04}
#define asn1OidBer_microsoftEncryptedFileSystem \
	{asn1Len_microsoftEncryptedFileSystem,asn1Str_microsoftEncryptedFileSystem}

#define asn1Len_netscapeServerGatedCrypto 11
#define asn1Str_netscapeServerGatedCrypto \
	(unsigned char *)"\006\011\140\206\110\001\206\370\102\004\001"
#define asn1Ary_netscapeServerGatedCrypto \
	{0x06,0x09,0x60,0x86,0x48,0x01,0x86,0xf8,0x42,0x04,0x01}
#define asn1OidBer_netscapeServerGatedCrypto \
	{asn1Len_netscapeServerGatedCrypto,asn1Str_netscapeServerGatedCrypto}

/**********************************************/
/* ASN.1 Oids associated with digests */
/**********************************************/

#define asn1Len_MD2 10
#define asn1Str_MD2 \
	(unsigned char *)"\006\010\052\206\110\206\367\015\002\002"
#define asn1Ary_MD2 \
	{0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x02}
#define asn1OidBer_MD2 \
	{asn1Len_MD2,asn1Str_MD2}

#define asn1Len_MD5 10
#define asn1Str_MD5 \
	(unsigned char *)"\006\010\052\206\110\206\367\015\002\005"
#define asn1Ary_MD5 \
	{0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05}
#define asn1OidBer_MD5 \
	{asn1Len_MD5,asn1Str_MD5}

#define asn1Len_SHA1 7
#define asn1Str_SHA1 \
	(unsigned char *)"\006\005\053\016\003\002\032"
#define asn1Ary_SHA1 \
	{0x06,0x05,0x2b,0x0e,0x03,0x02,0x1a}
#define asn1OidBer_SHA1 \
	{asn1Len_SHA1,asn1Str_SHA1}

#endif
