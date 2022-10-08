#include "nrg_foo_stream.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void             nrg_foo_stream_close(aimg_istream_t * const stream);
static size_t           nrg_foo_stream_read (aimg_istream_t * const stream,
                                         void * const buf, size_t const length);
static off_t            nrg_foo_stream_seek (aimg_istream_t * const, off_t const pos, int origin);

typedef struct nrg_foo_stream_t
{
    aimg_istream_t   base;
    service_ptr_t< file > p_reader;
}nrg_foo_stream_t;

static aimg_istream_op _opt = {
    "NRG foo stream plugin",
    NULL,
    NULL,
    nrg_foo_stream_close,
    nrg_foo_stream_seek,
    nrg_foo_stream_read
};

aimg_istream_t * nrg_foo_stream_new  (const service_ptr_t< file >& p_reader)
{
    nrg_foo_stream_t * s = (nrg_foo_stream_t*)aimg_istream_alloc(&_opt, sizeof(nrg_foo_stream_t));
    if(s){
        s->p_reader = p_reader;
    }
    return (aimg_istream_t *)s;
}

static void             nrg_foo_stream_close(aimg_istream_t * const stream)
{
    nrg_foo_stream_t * s = (nrg_foo_stream_t *)stream;
    s->p_reader.release();
    free(s);
}

static off_t            nrg_foo_stream_seek (aimg_istream_t * const stream, off_t const pos, int origin)
{
    /*
    enum t_seek_mode {
        //! Seek relative to beginning of file (same as seeking to absolute offset).
        seek_from_beginning = 0,
        //! Seek relative to current position.
        seek_from_current = 1,
        //! Seek relative to end of file.
        seek_from_eof = 2,
    };
    */
    /* origin:
        SEEK_CUR    1
        SEEK_END    2
        SEEK_SET    0
        AIMG_SEEK_TRACK 3
    */
    abort_callback_dummy p_abort;
    nrg_foo_stream_t * s = (nrg_foo_stream_t *)stream;
    if (3 == origin) {
        s->p_reader->seek_ex(pos + (AIMG_FRAMESIZE_RAW * 150), foobar2000_io::file::t_seek_mode::seek_from_beginning, p_abort);
        off_t o = s->p_reader->get_position(p_abort);
        if (o > (AIMG_FRAMESIZE_RAW * 150)) {
            return o - (AIMG_FRAMESIZE_RAW * 150);
        }
        return -1;
    }
    else {
        s->p_reader->seek_ex(pos, (foobar2000_io::file::t_seek_mode)origin, p_abort);
        return s->p_reader->get_position(p_abort);
    }
}

static size_t           nrg_foo_stream_read (aimg_istream_t * const stream,
                                         void * const buf, size_t const length)
{
    abort_callback_dummy p_abort;
    nrg_foo_stream_t * s = (nrg_foo_stream_t *)stream;
    return s->p_reader->read(buf, length, p_abort);
}
