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
#ifndef aimg_i_h
#define aimg_i_h

#include "aimg_types.h"
#include "aimg_plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

struct aimg_img_t
{
    aimg_img_op *        op;
    int                  tfirst;
    int                  tlast;
    long                 lfirst;
    long                 llast;
    aimg_text_t          text;
    aimg_track_t *       tracks[100];
};

aimg_img_t * aimg_image_alloc(aimg_img_op * const op, size_t size);

struct aimg_track_t
{
    aimg_ttype_t     type;
    int              flags;
    size_t           s_size;
    long             lfirst;
    long             llast;
    int              selected;
    aimg_text_t      text;
    aimg_istream_t * stream;
};

aimg_track_t * aimg_track_alloc(size_t size);

struct aimg_istream_t
{
    aimg_istream_op * op;
    off_t             length;
};
aimg_istream_t * aimg_istream_alloc(aimg_istream_op * const op, size_t size);

struct aimg_ostream_t
{
    const aimg_ostream_op * op;
};
aimg_ostream_t * aimg_ostream_alloc(aimg_ostream_op * const op, size_t size);

enum {
    AIMG_FRAMESIZE_RAW   = 2352,
    AIMG_FRAMESIZE_MODE1 = 2048,
    AIMG_FRAMESIZE_MODE2 = 2336,
    AIMG_FRAMESAMPLES    = AIMG_FRAMESIZE_RAW / 2,
    AIMG_MAX_SECTOR      = 450150,
    AIMG_FRAMES_PER_SEC  = 75,
    AIMG_MCN_SIZE        = 13,
    AIMG_ISRC_SIZE       = 12
};

#define AIMG_FRAMES_PER_MIN (AIMG_FRAMES_PER_SEC*60)

/* log functions */
enum{
    Log_Fatal,
    Log_Error,
    Log_Warning,
    Log_Info,
    Log_Debug,
};

void   aimg_log_set_level  (int level);
int    aimg_log_check_level(int level);
void   aimg_log  (int level, const char * module, const char * const format, ...);
/* plugins */
aimg_img_t     * aimg_plugin_img_open  (const char * const file, const char * ext, aimg_istream_t * const stream);
aimg_img_t     * aimg_plugin_img_parse (void * const data, const size_t size, const char * ext, aimg_istream_t * const stream);

aimg_istream_t * aimg_plugin_istream_open(const char * const file, const char * ext);
aimg_ostream_t * aimg_plugin_ostream_open(const char * const file, const char * ext);

#ifdef __cplusplus
}  //end extern "C"
#endif
#endif
