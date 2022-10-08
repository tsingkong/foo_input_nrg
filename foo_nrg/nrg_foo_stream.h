#ifndef NRG_FOO_STREAM_H
#define NRG_FOO_STREAM_H

#include "../nrg_img/include/aimg.h"
#include "../nrg_img/include/aimg_nrg.h"
#include "../foo_sdk/foobar2000/SDK/foobar2000.h"

aimg_istream_t* nrg_foo_stream_new(const service_ptr_t< file >& p_reader);

#endif
