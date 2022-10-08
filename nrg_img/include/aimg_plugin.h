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
#ifndef aimg_plugin_H
#define aimg_plugin_H
#include "aimg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define QUOTATE(X) #X

typedef int (aimg_plugin_initialize)(void * const handle);

typedef struct aimg_img_op
{
    const char    *info;
    const char    *ext;
    aimg_img_t * (*open) (const char * const, aimg_istream_t * const stream);
    aimg_img_t * (*parse)(char * const, const size_t, aimg_istream_t * const stream);
    void         (*close)(aimg_img_t * const);
}aimg_img_op;

typedef struct aimg_istream_op
{
    const char        *info;
    const char        *ext;
    aimg_istream_t * (*open_stream) (const char * const);
    void             (*close_stream)(aimg_istream_t * const);
    /* origin: 
        SEEK_CUR    1
        SEEK_END    2
        SEEK_SET    0
        AIMG_SEEK_TRACK 3
    */
    off_t            (*seek_stream) (aimg_istream_t * const, off_t const pos, int origin);
    size_t           (*read_stream) (aimg_istream_t * const, void * const buf, size_t length);
}aimg_istream_op;

typedef struct aimg_ostream_op
{
    const char        *info;
    const char        *ext;
    aimg_ostream_t * (*open) (const char * const);
    void             (*close)(aimg_ostream_t * const);
    size_t           (*write)(aimg_ostream_t * const ,
                              const void * const, size_t const);
    void             (*set_text)(aimg_ostream_t * const ,
                                 const aimg_text_t * const text);
}aimg_ostream_op;

int aimg_register_plugin(void * handle, int type, void* op);

#define AIMG_PLUGIN_CONSTRUCTOR_NAME    "aimg_plugin_init"
#define AIMG_PLUGIN_CONSTRUCTOR(handle)  int aimg_plugin_init (void * const handle)

#ifdef __cplusplus
}  //end extern "C"
#endif
#endif
