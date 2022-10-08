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
#ifndef aimg_h
#define aimg_h
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif
    typedef struct aimg_img_t       aimg_img_t;
    typedef struct aimg_track_t aimg_track_t;
    typedef struct aimg_istream_t   aimg_istream_t;
    typedef struct aimg_ostream_t   aimg_ostream_t;

    typedef struct aimg_text_t      aimg_text_t;

    typedef struct msf_t            msf_t;
    typedef struct aimg_cfg_t
    {
        const char* plugin_dir;
        const char* language;
        int          log_level;
    }aimg_cfg_t;

    /*==================================================================*/
    /* Plugins */
    typedef struct aimg_plugin_info
    {
        const char* info;
        const char* ext;
    }aimg_plugin_info;

    typedef enum {
        AIMG_PLUGIN_IMAGE,
        AIMG_PLUGIN_ISTREAM,
        AIMG_PLUGIN_OSTREAM,
    }aimg_plugin_type;

    const aimg_plugin_info* aimg_plugin_get_info(aimg_plugin_type type, int idx);

    /*==================================================================*/
    /* Image operations */

    typedef enum aimg_ttype_t {
        AIMG_TRACK_LEAD_IN,
        AIMG_TRACK_AUDIO,
        AIMG_TRACK_DATA,
        AIMG_TRACK_LEAD_OUT,
        AIMG_TRACK_ERROR,
    }aimg_ttype_t;

    enum aimg_track_flags {
        AIMG_TRACK_FLAG_NONE = 0x00,
        AIMG_TRACK_FLAG_PRE = 0x01,
        AIMG_TRACK_FLAG_DCP = 0x02,
        AIMG_TRACK_FLAG_DATA = 0x04,
        AIMG_TRACK_FLAG_4CH = 0x08,
        AIMG_TRACK_FLAG_SCMS = 0x10,
    };

#define aimg_image_open(FILE) aimg_image_open_with_stream(FILE, NULL)

    aimg_img_t*
        aimg_image_open_with_stream(const char* const file, aimg_istream_t* const stream);

    void
        aimg_image_close(aimg_img_t* const);

    int
        aimg_image_first_track(const aimg_img_t* const);

    int
        aimg_image_last_track(const aimg_img_t* const);

    aimg_text_t*
        aimg_image_text(const aimg_img_t* const image);

    int
        aimg_image_track_is_audio(const aimg_img_t* const, int track);

    void
        aimg_image_select_track(aimg_img_t* const, int track, int select);

    int
        aimg_image_track_is_selected(const aimg_img_t* const, int track);

    void
        aimg_image_select_all_tracks(aimg_img_t* const, int select);

    aimg_ttype_t
        aimg_image_track_type(const aimg_img_t* const, int track);

    const char*
        aimg_image_track_type_str(const aimg_img_t* const, int track);

    aimg_text_t*
        aimg_image_track_text(const aimg_img_t* const image, int const track);

    size_t
        aimg_image_track_sector_size(const aimg_img_t* const, int track);

    long
        aimg_image_track_length(const aimg_img_t* const, int track);

    long
        aimg_image_track_first_sector(const aimg_img_t* const, int track);

    long
        aimg_image_track_last_sector(const aimg_img_t* const, int track);

    /*==================================================================*/
    /* track operations */
    aimg_track_t*
        aimg_track_open(aimg_img_t* const, int track);

    void
        aimg_track_close(aimg_track_t* const);

    aimg_ttype_t
        aimg_track_type(const aimg_track_t* const);

    const char*
        aimg_track_type_str(const aimg_track_t* const);

    size_t
        aimg_track_sector_size(const aimg_track_t* const);

    long
        aimg_track_length(const aimg_track_t* const);

    long
        aimg_track_first_sector(const aimg_track_t* const);

    long
        aimg_track_last_sector(const aimg_track_t* const);

    long
        aimg_track_seek_sector(aimg_track_t* const, long const);

    aimg_text_t*
        aimg_track_text(aimg_track_t* const);

    int
        aimg_track_flags(const aimg_track_t* const);

    size_t
        aimg_track_read(aimg_track_t* const, void* const buf);

    void
        aimg_track_select(aimg_track_t* const, int select);

    int
        aimg_track_is_selected(const aimg_track_t* const);

    /*==================================================================*/
    /* input stream operations */
    aimg_istream_t*
        aimg_istream_open(const char* const file);

    void
        aimg_istream_close(aimg_istream_t* const);

    off_t
        aimg_istream_length(aimg_istream_t* const);

    off_t
        aimg_istream_seek(aimg_istream_t* const, off_t const offset);

    size_t
        aimg_istream_read(aimg_istream_t* const, void* const buf, size_t const length);

    /*==================================================================*/
    /* output stream operations */
    aimg_ostream_t*
        aimg_ostream_create(const char* const file, const char* const ext);

    aimg_ostream_t*
        aimg_ostream_create_with_text(const char* const temp,
            const aimg_text_t* const text,
            const char* const ext);
    void
        aimg_ostream_close(aimg_ostream_t* const);

    void
        aimg_ostream_set_text(aimg_ostream_t* const, const aimg_text_t* const);

    size_t
        aimg_ostream_write(aimg_ostream_t* const, const void* const buf, size_t const length);

    int
        aimg_ostream_write_sector(aimg_ostream_t* const, void* buf);

    void
        aimg_ostream_set_text(aimg_ostream_t* const,
            const aimg_text_t* const text);

    /*==================================================================*/
    /* utils */
    struct msf_t
    {
        unsigned int m;
        unsigned int s;
        unsigned int f;
    };

    long            aimg_msf_to_sector(const msf_t* const msf);
    void            aimg_sector_to_msf(long const sector, msf_t* const msf);
    int             aimg_msf_parse(msf_t* const msf, const char* p);
    int             aimg_msf_sprintf(char* buf, const msf_t* const msf);

    long            aimg_frame_to_sector(long const frame);
    long            aimg_size_to_sector(long const frame);
    long            aimg_sector_to_frame(long const sector);
    long            aimg_sector_to_size(long const sector);


    /*==================================================================*/
    /* text */
    struct aimg_text_t
    {
        unsigned char track_num;
        unsigned char tracks_count;
        char* artist;
        char* album;
        char* title;
        char* genre;
        char* year;
        char* comment;
    };

    void aimg_text_clean(aimg_text_t* const text);
    void aimg_text_set_artist(aimg_text_t* const text, const char* str);
    void aimg_text_set_album(aimg_text_t* const text, const char* str);
    void aimg_text_set_title(aimg_text_t* const text, const char* str);
    void aimg_text_set_genre(aimg_text_t* const text, const char* str);
    void aimg_text_set_year(aimg_text_t* const text, const char* str);
    void aimg_text_set_comment(aimg_text_t* const text, const char* str);
    void aimg_text_copy_over(aimg_text_t* const dst, const aimg_text_t* const src);
#ifdef __cplusplus
}  //end extern "C"
#endif
#endif
