#include "palette.h"

const RGBColorType defaultPalette[MAX_PAL] = {
  { 0x00,0xff,0xff,0xff },
  { 0x01,0xff,0xcc,0xff },
  { 0x02,0xff,0x99,0xff },
  { 0x03,0xff,0x66,0xff },
  { 0x04,0xff,0x33,0xff },
  { 0x05,0xff,0x00,0xff },
  { 0x06,0xff,0xff,0xcc },
  { 0x07,0xff,0xcc,0xcc },
  { 0x08,0xff,0x99,0xcc },
  { 0x09,0xff,0x66,0xcc },
  { 0x0a,0xff,0x33,0xcc },
  { 0x0b,0xff,0x00,0xcc },
  { 0x0c,0xff,0xff,0x99 },
  { 0x0d,0xff,0xcc,0x99 },
  { 0x0e,0xff,0x99,0x99 },
  { 0x0f,0xff,0x66,0x99 },
  { 0x10,0xff,0x33,0x99 },
  { 0x11,0xff,0x00,0x99 },
  { 0x12,0xcc,0xff,0xff },
  { 0x13,0xcc,0xcc,0xff },
  { 0x14,0xcc,0x99,0xff },
  { 0x15,0xcc,0x66,0xff },
  { 0x16,0xcc,0x33,0xff },
  { 0x17,0xcc,0x00,0xff },
  { 0x18,0xcc,0xff,0xcc },
  { 0x19,0xcc,0xcc,0xcc },
  { 0x1a,0xcc,0x99,0xcc },
  { 0x1b,0xcc,0x66,0xcc },
  { 0x1c,0xcc,0x33,0xcc },
  { 0x1d,0xcc,0x00,0xcc },
  { 0x1e,0xcc,0xff,0x99 },
  { 0x1f,0xcc,0xcc,0x99 },
  { 0x20,0xcc,0x99,0x99 },
  { 0x21,0xcc,0x66,0x99 },
  { 0x22,0xcc,0x33,0x99 },
  { 0x23,0xcc,0x00,0x99 },
  { 0x24,0x99,0xff,0xff },
  { 0x25,0x99,0xcc,0xff },
  { 0x26,0x99,0x99,0xff },
  { 0x27,0x99,0x66,0xff },
  { 0x28,0x99,0x33,0xff },
  { 0x29,0x99,0x00,0xff },
  { 0x2a,0x99,0xff,0xcc },
  { 0x2b,0x99,0xcc,0xcc },
  { 0x2c,0x99,0x99,0xcc },
  { 0x2d,0x99,0x66,0xcc },
  { 0x2e,0x99,0x33,0xcc },
  { 0x2f,0x99,0x00,0xcc },
  { 0x30,0x99,0xff,0x99 },
  { 0x31,0x99,0xcc,0x99 },
  { 0x32,0x99,0x99,0x99 },
  { 0x33,0x99,0x66,0x99 },
  { 0x34,0x99,0x33,0x99 },
  { 0x35,0x99,0x00,0x99 },
  { 0x36,0x66,0xff,0xff },
  { 0x37,0x66,0xcc,0xff },
  { 0x38,0x66,0x99,0xff },
  { 0x39,0x66,0x66,0xff },
  { 0x3a,0x66,0x33,0xff },
  { 0x3b,0x66,0x00,0xff },
  { 0x3c,0x66,0xff,0xcc },
  { 0x3d,0x66,0xcc,0xcc },
  { 0x3e,0x66,0x99,0xcc },
  { 0x3f,0x66,0x66,0xcc },
  { 0x40,0x66,0x33,0xcc },
  { 0x41,0x66,0x00,0xcc },
  { 0x42,0x66,0xff,0x99 },
  { 0x43,0x66,0xcc,0x99 },
  { 0x44,0x66,0x99,0x99 },
  { 0x45,0x66,0x66,0x99 },
  { 0x46,0x66,0x33,0x99 },
  { 0x47,0x66,0x00,0x99 },
  { 0x48,0x33,0xff,0xff },
  { 0x49,0x33,0xcc,0xff },
  { 0x4a,0x33,0x99,0xff },
  { 0x4b,0x33,0x66,0xff },
  { 0x4c,0x33,0x33,0xff },
  { 0x4d,0x33,0x00,0xff },
  { 0x4e,0x33,0xff,0xcc },
  { 0x4f,0x33,0xcc,0xcc },
  { 0x50,0x33,0x99,0xcc },
  { 0x51,0x33,0x66,0xcc },
  { 0x52,0x33,0x33,0xcc },
  { 0x53,0x33,0x00,0xcc },
  { 0x54,0x33,0xff,0x99 },
  { 0x55,0x33,0xcc,0x99 },
  { 0x56,0x33,0x99,0x99 },
  { 0x57,0x33,0x66,0x99 },
  { 0x58,0x33,0x33,0x99 },
  { 0x59,0x33,0x00,0x99 },
  { 0x5a,0x00,0xff,0xff },
  { 0x5b,0x00,0xcc,0xff },
  { 0x5c,0x00,0x99,0xff },
  { 0x5d,0x00,0x66,0xff },
  { 0x5e,0x00,0x33,0xff },
  { 0x5f,0x00,0x00,0xff },
  { 0x60,0x00,0xff,0xcc },
  { 0x61,0x00,0xcc,0xcc },
  { 0x62,0x00,0x99,0xcc },
  { 0x63,0x00,0x66,0xcc },
  { 0x64,0x00,0x33,0xcc },
  { 0x65,0x00,0x00,0xcc },
  { 0x66,0x00,0xff,0x99 },
  { 0x67,0x00,0xcc,0x99 },
  { 0x68,0x00,0x99,0x99 },
  { 0x69,0x00,0x66,0x99 },
  { 0x6a,0x00,0x33,0x99 },
  { 0x6b,0x00,0x00,0x99 },
  { 0x6c,0xff,0xff,0x66 },
  { 0x6d,0xff,0xcc,0x66 },
  { 0x6e,0xff,0x99,0x66 },
  { 0x6f,0xff,0x66,0x66 },
  { 0x70,0xff,0x33,0x66 },
  { 0x71,0xff,0x00,0x66 },
  { 0x72,0xff,0xff,0x33 },
  { 0x73,0xff,0xcc,0x33 },
  { 0x74,0xff,0x99,0x33 },
  { 0x75,0xff,0x66,0x33 },
  { 0x76,0xff,0x33,0x33 },
  { 0x77,0xff,0x00,0x33 },
  { 0x78,0xff,0xff,0x00 },
  { 0x79,0xff,0xcc,0x00 },
  { 0x7a,0xff,0x99,0x00 },
  { 0x7b,0xff,0x66,0x00 },
  { 0x7c,0xff,0x33,0x00 },
  { 0x7d,0xff,0x00,0x00 },
  { 0x7e,0xcc,0xff,0x66 },
  { 0x7f,0xcc,0xcc,0x66 },
  { 0x80,0xcc,0x99,0x66 },
  { 0x81,0xcc,0x66,0x66 },
  { 0x82,0xcc,0x33,0x66 },
  { 0x83,0xcc,0x00,0x66 },
  { 0x84,0xcc,0xff,0x33 },
  { 0x85,0xcc,0xcc,0x33 },
  { 0x86,0xcc,0x99,0x33 },
  { 0x87,0xcc,0x66,0x33 },
  { 0x88,0xcc,0x33,0x33 },
  { 0x89,0xcc,0x00,0x33 },
  { 0x8a,0xcc,0xff,0x00 },
  { 0x8b,0xcc,0xcc,0x00 },
  { 0x8c,0xcc,0x99,0x00 },
  { 0x8d,0xcc,0x66,0x00 },
  { 0x8e,0xcc,0x33,0x00 },
  { 0x8f,0xcc,0x00,0x00 },
  { 0x90,0x99,0xff,0x66 },
  { 0x91,0x99,0xcc,0x66 },
  { 0x92,0x99,0x99,0x66 },
  { 0x93,0x99,0x66,0x66 },
  { 0x94,0x99,0x33,0x66 },
  { 0x95,0x99,0x00,0x66 },
  { 0x96,0x99,0xff,0x33 },
  { 0x97,0x99,0xcc,0x33 },
  { 0x98,0x99,0x99,0x33 },
  { 0x99,0x99,0x66,0x33 },
  { 0x9a,0x99,0x33,0x33 },
  { 0x9b,0x99,0x00,0x33 },
  { 0x9c,0x99,0xff,0x00 },
  { 0x9d,0x99,0xcc,0x00 },
  { 0x9e,0x99,0x99,0x00 },
  { 0x9f,0x99,0x66,0x00 },
  { 0xa0,0x99,0x33,0x00 },
  { 0xa1,0x99,0x00,0x00 },
  { 0xa2,0x66,0xff,0x66 },
  { 0xa3,0x66,0xcc,0x66 },
  { 0xa4,0x66,0x99,0x66 },
  { 0xa5,0x66,0x66,0x66 },
  { 0xa6,0x66,0x33,0x66 },
  { 0xa7,0x66,0x00,0x66 },
  { 0xa8,0x66,0xff,0x33 },
  { 0xa9,0x66,0xcc,0x33 },
  { 0xaa,0x66,0x99,0x33 },
  { 0xab,0x66,0x66,0x33 },
  { 0xac,0x66,0x33,0x33 },
  { 0xad,0x66,0x00,0x33 },
  { 0xae,0x66,0xff,0x00 },
  { 0xaf,0x66,0xcc,0x00 },
  { 0xb0,0x66,0x99,0x00 },
  { 0xb1,0x66,0x66,0x00 },
  { 0xb2,0x66,0x33,0x00 },
  { 0xb3,0x66,0x00,0x00 },
  { 0xb4,0x33,0xff,0x66 },
  { 0xb5,0x33,0xcc,0x66 },
  { 0xb6,0x33,0x99,0x66 },
  { 0xb7,0x33,0x66,0x66 },
  { 0xb8,0x33,0x33,0x66 },
  { 0xb9,0x33,0x00,0x66 },
  { 0xba,0x33,0xff,0x33 },
  { 0xbb,0x33,0xcc,0x33 },
  { 0xbc,0x33,0x99,0x33 },
  { 0xbd,0x33,0x66,0x33 },
  { 0xbe,0x33,0x33,0x33 },
  { 0xbf,0x33,0x00,0x33 },
  { 0xc0,0x33,0xff,0x00 },
  { 0xc1,0x33,0xcc,0x00 },
  { 0xc2,0x33,0x99,0x00 },
  { 0xc3,0x33,0x66,0x00 },
  { 0xc4,0x33,0x33,0x00 },
  { 0xc5,0x33,0x00,0x00 },
  { 0xc6,0x00,0xff,0x66 },
  { 0xc7,0x00,0xcc,0x66 },
  { 0xc8,0x00,0x99,0x66 },
  { 0xc9,0x00,0x66,0x66 },
  { 0xca,0x00,0x33,0x66 },
  { 0xcb,0x00,0x00,0x66 },
  { 0xcc,0x00,0xff,0x33 },
  { 0xcd,0x00,0xcc,0x33 },
  { 0xce,0x00,0x99,0x33 },
  { 0xcf,0x00,0x66,0x33 },
  { 0xd0,0x00,0x33,0x33 },
  { 0xd1,0x00,0x00,0x33 },
  { 0xd2,0x00,0xff,0x00 },
  { 0xd3,0x00,0xcc,0x00 },
  { 0xd4,0x00,0x99,0x00 },
  { 0xd5,0x00,0x66,0x00 },
  { 0xd6,0x00,0x33,0x00 },
  { 0xd7,0x11,0x11,0x11 },
  { 0xd8,0x22,0x22,0x22 },
  { 0xd9,0x44,0x44,0x44 },
  { 0xda,0x55,0x55,0x55 },
  { 0xdb,0x77,0x77,0x77 },
  { 0xdc,0x88,0x88,0x88 },
  { 0xdd,0xaa,0xaa,0xaa },
  { 0xde,0xbb,0xbb,0xbb },
  { 0xdf,0xdd,0xdd,0xdd },
  { 0xe0,0xee,0xee,0xee },
  { 0xe1,0xc0,0xc0,0xc0 },
  { 0xe2,0x80,0x00,0x00 },
  { 0xe3,0x80,0x00,0x80 },
  { 0xe4,0x00,0x80,0x00 },
  { 0xe5,0x00,0x80,0x80 },
  { 0xe6,0x00,0x00,0x00 },
  { 0xe7,0x00,0x00,0x00 },
  { 0xe8,0x00,0x00,0x00 },
  { 0xe9,0x00,0x00,0x00 },
  { 0xea,0x00,0x00,0x00 },
  { 0xeb,0x00,0x00,0x00 },
  { 0xec,0x00,0x00,0x00 },
  { 0xed,0x00,0x00,0x00 },
  { 0xee,0x00,0x00,0x00 },
  { 0xef,0x00,0x00,0x00 },
  { 0xf0,0x00,0x00,0x00 },
  { 0xf1,0x00,0x00,0x00 },
  { 0xf2,0x00,0x00,0x00 },
  { 0xf3,0x00,0x00,0x00 },
  { 0xf4,0x00,0x00,0x00 },
  { 0xf5,0x00,0x00,0x00 },
  { 0xf6,0x00,0x00,0x00 },
  { 0xf7,0x00,0x00,0x00 },
  { 0xf8,0x00,0x00,0x00 },
  { 0xf9,0x00,0x00,0x00 },
  { 0xfa,0x00,0x00,0x00 },
  { 0xfb,0x00,0x00,0x00 },
  { 0xfc,0x00,0x00,0x00 },
  { 0xfd,0x00,0x00,0x00 },
  { 0xfe,0x00,0x00,0x00 },
  { 0xff,0x00,0x00,0x00 }
};
