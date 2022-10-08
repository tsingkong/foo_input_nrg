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
#ifndef aimg_types_h
#define aimg_types_h

#if defined(__GNUC__)
# define EMPTY_ARRAY_SIZE
# define GNUC_PACKED   __attribute__((packed))
# define PRAGMA_BEGIN_PACKED
# define PRAGMA_END_PACKED
# include <endian.h>
# if __BYTE_ORDER == __BIG_ENDIAN
#  define HOST_BIG_ENDIAN
# endif
# include <byteswap.h>
#else   /* !__GNUC__ */
# define EMPTY_ARRAY_SIZE 1
# define GNUC_PACKED
# define PRAGMA_BEGIN_PACKED _Pragma("pack(1)")
# define PRAGMA_END_PACKED   _Pragma("pack()")
# include <stdint.h>
# include "byteswap.h"
#endif  /* __GNUC__ */


/* integer conversion */
#ifdef HOST_BIG_ENDIAN

#define aimg_icvt16(X) (X)
#define aimg_icvt32(X) (X)
#define aimg_icvt64(X) (X)

#else /* !HOST_BIG_ENDIAN */

#define aimg_icvt16(X) bswap_16(X)
#define aimg_icvt32(X) bswap_32(X)
#define aimg_icvt64(X) bswap_64(X)

#endif /* HOST_BIG_ENDIAN */

typedef unsigned long long   uint64;
typedef unsigned long        uint32;
typedef unsigned short       uint16;
typedef unsigned char        uint8;
typedef unsigned char        byte;

typedef long long   int64;
typedef long        int32;
typedef short       int16;
typedef char        int8;

#endif
