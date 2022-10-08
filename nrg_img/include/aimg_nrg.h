/***************************************************************************
 *   Copyright (C) 2009 by Danya Filatov                                   *
 *   danya25@free.fr                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef aimg_nrg_h
#define aimg_nrg_h
#include "aimg_i.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NRG_VERSION_V55_SHIFT 12
#define NRG_VERSION_V50_SHIFT 8
#define NRG_FOOTER_MAX_SIZE   4096

/* Nero Footer tags (BIG ENDIAN) */
#ifdef HOST_BIG_ENDIAN
typedef enum {
  CDTX_ID  = 0x43445458,   /* CD TEXT */
  CUEX_ID  = 0x43554558,  /* Nero version 5.5.x-6.x */
  CUES_ID  = 0x43554553,  /* Nero pre version 5.5.x-6.x */
  DAOX_ID  = 0x44414f58,  /* Nero version 5.5.x-6.x */
  DAOI_ID  = 0x44414f49,
  END1_ID  = 0x454e4421,
  ETN2_ID  = 0x45544e32,
  ETNF_ID  = 0x45544e46,
  NER5_ID  = 0x4e455235,  /* Nero version 5.5.x */
  NERO_ID  = 0x4e45524f,  /* Nero pre 5.5.x */
  SINF_ID  = 0x53494e46,  /* Session information */
  MTYP_ID  = 0x4d545950,  /* Disc Media type? */
} nero_id_t;

#else

typedef enum {
  CDTX_ID  = 0x58544443,   /* CD TEXT */
  CUEX_ID  = 0x58455543,  /* Nero version 5.5.x-6.x */
  CUES_ID  = 0x53455543,  /* Nero pre version 5.5.x-6.x */
  DAOX_ID  = 0x584f4144,  /* Nero version 5.5.x-6.x */
  DAOI_ID  = 0x494f4144,
  END1_ID  = 0x21444e45,
  ETN2_ID  = 0x324e5445,
  ETNF_ID  = 0x464e5445,
  NER5_ID  = 0x3552454e,  /* Nero version 5.5.x */
  NERO_ID  = 0x4f52454e,  /* Nero pre 5.5.x */
  SINF_ID  = 0x464e4953,  /* Session information */
  MTYP_ID  = 0x5059544d,  /* Disc Media type? */
} nero_id_t;
#endif

PRAGMA_BEGIN_PACKED

typedef union {
  struct {
    uint32 __x          GNUC_PACKED;
    uint32 ID           GNUC_PACKED;
    uint32 footer_ofs   GNUC_PACKED;
  } v50;
  struct {
    uint32 ID           GNUC_PACKED;
    uint64 footer_ofs   GNUC_PACKED;
  } v55;
} nrg_version_t;

typedef struct {
  uint32 id                    GNUC_PACKED;
  uint32 len                   GNUC_PACKED;
  uint8  data[EMPTY_ARRAY_SIZE];
} nrg_chunk_t;

typedef struct {
    unsigned char type:4;
    unsigned char flags:4;

    uint8    track;

    unsigned char addr:4;
    unsigned char ctrl:4;
    uint8    res;
    uint32   lsn        GNUC_PACKED;
} nrg_cuex_array_t;

typedef struct {
    uint8     isrc [AIMG_ISRC_SIZE];
    uint16    flags     GNUC_PACKED;
    uint8     type;
    uint8    _unknown[3];
    uint64    pregap    GNUC_PACKED;
    uint64    begin     GNUC_PACKED;
    uint64    end       GNUC_PACKED;
} nrg_daox_track_t;

typedef struct {
  uint32 _unknown1           GNUC_PACKED;
  uint8  mcn[AIMG_MCN_SIZE];
  uint32 _unknown2           GNUC_PACKED;
  uint8  tcount;
  nrg_daox_track_t tracks[];
} nrg_daox_chunk_t;

// The format is explained in part in Annex J of(MMC - 3), and in part by Sony’s documentation cdtext.zip.
typedef struct {
    /* The first byte of each pack contains the pack type. See table:categories for a list of pack types. 
      0x80: Title
      0x81: Performers
      0x82: Songwriters 
      0x83: Composers
      0x84: Arrangers 
      0x85: Message Area
      0x86: Disc Identification (in text and binary)
      0x87: Genre Identification (in text and binary)
      0x88: Table of Contents (in binary)
      0x89: Second Table of Contents (in binary)
      0x8d: Closed Information
      0x8e: UPC/EAN code of the album and ISRC code of each track
      0x8f: Block Size Information (binary)

    Pack Types 0x8a to 0x8c although not specified are reserved for potential future use.
    Pack Types 0x86, 0x87, 0x88, 0x89, 0x8d (Disc Identification, Genre Identification, Table of Contents, Second Table of Contents and Closed Information respectively) apply to the whole disc, and cannot be attached to individual tracks.
    Pack Types 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, and 0x8e (Performers, Songwriters, Composers, Arrangers, and Message Area respectively) have to be attributed to each track if they are present for the whole disc.
    Pack Type  0x8f (Block Size Information) describes the overall content of a block and in part of all other blocks.
    */
    uint8 pack_type;
    /*
    The second byte often gives the track number of the pack. However, 
    a zero track value indicates that the information pertains to the whole album. 
    Higher numbers are valid for track-oriented packs (types 0x80 to 0x85, and 0x8e). 
    In these pack types, there should be one text pack for the disc and one for each track. 
    With TOC packs (types 0x88 and 0x89), the second byte is a track number too. 
    With type 0x8f, the second byte counts the record parts from 0 to 2. 
    */
    uint8 trackNo;
    // The third byte is a sequential counter. 
    uint8 packNo;
    /*
    The fourth byte is the Block Number and Character Position Indicator. It consists of three bit fields:
    bits 0-3
        Character position. Either the number of characters which the current text inherited from the previous pack, or 15 if the current text started before the previous pack. 
    bits 4-6
        Block Number (groups text packs in language blocks) 
    bit 7
        Is 0 if single byte characters, 1 if double-byte characters. 
    */
    uint8 charInherit : 4;
    uint8 blockNo : 3;
    uint8 multiByte : 1;
    uint8 text[12];
    uint16 crc;
} nrg_cdtx_chunk_t;

PRAGMA_END_PACKED

aimg_istream_t * nrg_stream_new  (int fd);

typedef struct aimg_nrg_t
{
    aimg_img_t       base;
    uint32           version;
    aimg_istream_t* stream;
}aimg_nrg_t;

// 用 path 或 stream 打开 nrg 文件
aimg_img_t* nrg_open(const char* const path, aimg_istream_t* const stream);
aimg_track_t* nrg_track_new(aimg_nrg_t* const a, int tidx, int type);
void         nrg_close(aimg_img_t* const a);

#ifdef __cplusplus
}  //end extern "C"
#endif
#endif
