/**
 * foo_input_nrg - nrg decoder for foobar2000
*/

/* include foobar sdk */
#include "../foo_sdk/foobar2000/SDK/foobar2000.h"

#include <commctrl.h>
#include <Shlwapi.h>
#include <time.h>
#include "nrg_parser.h"
#include "resource.h"
#include "DbgOut.h"

class input_nrg : public input_stubs {
private:
	nrg_parser_ptr m_parser;
	service_ptr_t<packet_decoder> m_decoder;
	t_input_open_reason m_reason;

	int m_TrackNo;

	unsigned m_expected_sample_rate;
	unsigned m_expected_channels;
	unsigned m_expected_bitspersample;

	uint64 m_length, m_position;

public:
	service_ptr_t<file> m_file;
	pfc::array_t<t_uint8> m_buffer;
	
public:
	input_nrg()
	{
		hprintf(L"Nrg: input_nrg()\n");
		m_parser = NULL;
		m_decoder = NULL;

		m_TrackNo = 0;
		m_position = 0;
		m_length = 0;
		m_expected_channels = 6;
		m_expected_sample_rate = 44100;
		m_expected_bitspersample = 32;
	}

	~input_nrg()
	{
		hprintf(L"Nrg: ~input_nrg()\n");
        hprintf(L"--destruction-- position=%d\n", m_position);
		cleanup();
	}

	/**
	 * API function called by foobar to open a file. It's called on get info or playback start. It's safest place
	 * to place total_frame_count extraction code.
	 * 
	 * @param p_filehint	file object, may be null untill opened.
	 * @param p_path		path to file
	 * @param p_reason		reason why the file was opened
	 * @param p_abort		abort callback
	 * @since				1.0.0
	 */
	void open(service_ptr_t<file> p_filehint,const char * p_path,t_input_open_reason p_reason,abort_callback & p_abort) {
		hprintf(L"Nrg: open() p_reason=%d\n", p_reason);

		/* write access is called for retagging purposes. we do not support that, so throw an error */
		if (p_reason == input_open_info_write) throw exception_io_unsupported_format();

		cleanup();

		/* store file object */
		m_file = p_filehint;

		/* if file is null, open it - handled by the helper */
		input_open_file_helper(m_file,p_path,p_reason,p_abort);

		m_reason = p_reason;
		hprintf(L"file opened:%S", p_path);

		m_parser = nrg_parser_ptr(new NrgAudioParser(m_file, p_abort));
		if (m_parser->Parse(!(m_reason & input_open_decode))) {
			console::error("Nrg: Invalid Nrg file.");
			cleanup();
			throw exception_io_unsupported_format();
		}

		m_TrackNo = m_parser->GetFirstAudioTrack();
		if (m_TrackNo == -1) {
			console::error("Nrg: no decodable streams found.");
			cleanup();
			throw exception_io_unsupported_format();
		}
	}

	unsigned int get_subsong_count() {
		int count = m_parser->GetAudioTrackCount();
		hprintf(L"Nrg: get_subsong_count() chapters=%d, AudioTrackCount=%d\n", count);
		return count;
	}

	//! See: input_info_reader::get_subsong(). Valid after open() with any reason.
	t_uint32 get_subsong(unsigned p_index)
	{
		hprintf(L"Nrg: get_subsong() p_index=%d\n", p_index);
		return p_index;
	}

	//! See: input_info_reader::get_info(). Valid after open() with any reason.
	/**
	 * API function called by foobar to get information of properties dialog.
	 * @param p_info		object to store the info in
	 * @param p_abort		abort callback
	 */
	void get_info(t_uint32 p_subsong, file_info& p_info, abort_callback& p_abort)
	{
		hprintf(L"Nrg: get_info() = %d\n", p_subsong);
		if (m_reason != input_open_decode || m_decoder == NULL) {
			set_current_track(p_subsong);
			initialize_decorder(p_abort);
		}
		p_info.info_set_int("channels", m_expected_channels);
		p_info.info_set_int("samplerate", m_expected_sample_rate);

		m_decoder->get_info(p_info);

		m_expected_channels = (unsigned)p_info.info_get_int("channels");
		m_expected_sample_rate = (unsigned)p_info.info_get_int("samplerate");
		m_expected_bitspersample = (unsigned)p_info.info_get_int("bitspersample");

		m_decoder->set_stream_property(packet_decoder::property_channels, m_expected_channels, 0, 0);
		m_decoder->set_stream_property(packet_decoder::property_samplerate, m_expected_sample_rate, 0, 0);
		m_decoder->set_stream_property(packet_decoder::property_bitspersample, m_expected_bitspersample, 0, 0);

		// p_info.info_set_int("bitspersample", m_expected_bitspersample);
		
		m_parser->SetFB2KInfo(p_info, p_subsong);

		hprintf(L"Nrg: m_parser->GetDuration(): %f\n", m_parser->GetDuration());
		p_info.set_length(m_parser->GetDuration());
		if (p_info.info_get_bitrate() == 0) {
			p_info.info_set_bitrate(m_parser->GetAvgBitrate());
		}
	}

	t_filestats get_file_stats(abort_callback & p_abort) {
		hprintf(L"Nrg: get_file_stats()\n");
		return m_file->get_stats(p_abort);
	}

	void decode_initialize(t_uint32 p_subsong, unsigned p_flags, abort_callback & p_abort) {
		hprintf(L"Nrg: decode_initialize() = %d\n", p_subsong);
		set_current_track(p_subsong);
		initialize_decorder(p_abort);
		
		m_length = duration_to_samples(m_parser->GetDuration());
		if (decode_can_seek()) {
			decode_seek(0, p_abort);
		}
	}
	
	/**
	 * API function called by foobar to get next chunk of audio.
	 *
	 * @param p_chunk		buffer in which we store decoded audio
	 * @param p_abort		abort callback
	 * @see					m_block_size
	 * @since				1.1.0
	 */
	bool decode_run(audio_chunk & p_chunk, abort_callback & p_abort) {
		//hprintf(L"Nrg: decode_run(): m_skip_samples=%d, m_skip_frames=%d, m_frame_remaining=%d, m_frame=%p\n", (int)m_skip_samples, (int)m_skip_frames, m_frame_remaining, m_frame);
		if (m_decoder == NULL)
		{
			hprintf(L"Matroska: attempting to decode without a loaded decoder.\n");
			return false;
		}
		
		if (m_position >= m_length)
		{
            hprintf(L"Nrg: decode_run() return false: m_position=%d\n", m_position);
			return false;
		}

		void* ptr = NULL;
		t_size bytes = 0;
		// unsigned samplerate = 44100;
		unsigned nchannel = 6;
		unsigned bps = 32;	// 32 bits
		unsigned channel_config = audio_chunk::g_guess_channel_config(nchannel);
		bool ret = m_parser->Read(&ptr, &bytes);
		if (!ret)
		{
			return false;
		}

		// A temp decoding buffer
		audio_chunk_i m_tempchunk;
		m_tempchunk.reset();
		try {
			hprintf(L"Matroska: decode_run() start decode()\n");
			m_decoder->decode(ptr, bytes, m_tempchunk, p_abort);
		}
		catch (...) {
			hprintf(L"Matroska: decode_run() return false: decode error\n");
			cleanup();
			return false;
		}

		if (m_tempchunk.is_empty())
		{
			// if (duration > 0)
			{
				m_tempchunk.set_srate(m_expected_sample_rate);
				m_tempchunk.set_channels(m_expected_channels);
				// m_tempchunk.pad_with_silence((t_size)duration);
				console::warning("Matroska: decoder returned empty chunk from a non-empty Matroska frame.");
			}
		}
		else if (!m_tempchunk.is_valid())
		{
			console::error("Matroska: decoder produced invalid chunk.");
			cleanup();
			hprintf(L"Matroska: decode_run() return false: invalid chunk\n");
			return false;
		}

		unsigned samplerate = m_tempchunk.get_srate();
		unsigned channels = m_tempchunk.get_channels();
		t_size decoded_sample_count = m_tempchunk.get_sample_count();

		// !!!!!!!!!!!!!!! HACK ALERT !!!!!!!!!!!!!!!!!!!!!!!!
		t_size duration = decoded_sample_count;
		// !!!!!!!!!!!!!!! HACK ALERT !!!!!!!!!!!!!!!!!!!!!!!!

		p_chunk.set_data(m_tempchunk.get_data(),(t_size)duration, channels, samplerate);
		//! Helper, sets chunk data to contents of specified buffer, using default win32/wav conventions for signed/unsigned switch.
		// p_chunk.set_data((const audio_sample*)ptr, (t_size)bytes/4, nchannel, samplerate);
		// p_chunk.set_data_fixedpoint(ptr, bytes, samplerate, nchannel, bps, channel_config);
		return true;
	}
	
	//! See: input_decoder::seek(). Valid after decode_initialize().
	
	void decode_seek(double p_seconds, abort_callback & p_abort) {
		hprintf(L"Nrg: decode_seek() = %f\n", p_seconds);

		if (m_decoder == NULL)
		{
			console::error("Matroska: attempting to seek while not open.");
			cleanup();
			throw exception_io_object_not_seekable();
		}

		{
			double srate = (double)m_expected_sample_rate;
			p_seconds = floor(p_seconds * srate + 0.5) / srate;
		}

		m_position = (uint64)(m_expected_sample_rate * p_seconds + 0.5);

		if (!m_parser->Seek(p_seconds))
			return;
		m_decoder->reset_after_seek();
	}
	
	//! See: input_decoder::can_seek(). Valid after decode_initialize().
	bool decode_can_seek() {
		hprintf(L"Nrg: decode_can_seek()\n");
		return m_file->can_seek();
	}
	
	//! See: input_decoder::get_dynamic_info(). Valid after decode_initialize().
	bool decode_get_dynamic_info(file_info & p_out, double & p_timestamp_delta) {
		//hprintf(L"Nrg: decode_get_dynamic_info()\n");
		bool ret = false;
		return ret;
	}
	
	//! See: input_decoder::get_dynamic_info_track(). Valid after decode_initialize().
	bool decode_get_dynamic_info_track(file_info & p_out, double & p_timestamp_delta) {
		//hprintf(L"Nrg: decode_get_dynamic_info_track()\n");
		return false;
	}
	
	//! See: input_decoder::on_idle(). Valid after decode_initialize().
	void decode_on_idle(abort_callback & p_abort) {
		m_file->on_idle(p_abort);
	}

	void initialize_decorder(abort_callback& p_abort) {
		bool p_decode = false;
		if (m_reason == input_open_decode) {
			p_decode = true;
		}
		hprintf(L"Matroska: initialize_decoder(): m_TrackNo=%d\n", m_TrackNo);
		{
			packet_decoder::matroska_setup setup;
			pfc::string8 codec_temp("A_DTS");
			setup.codec_id = codec_temp;
			setup.sample_rate = (unsigned)m_expected_sample_rate;
			setup.sample_rate_output = (unsigned)m_expected_sample_rate;
			setup.channels = (unsigned)m_expected_channels;
			setup.codec_private_size = 0;
			setup.codec_private = NULL;

			packet_decoder::g_open(m_decoder, p_decode, packet_decoder::owner_matroska, 0, &setup, sizeof(setup), p_abort);
			if (m_decoder == NULL)
			{
				console::error(uStringPrintf("Matroska: unable to find a \"%s\" packet decoder object.", (const char*)codec_temp));
				cleanup();
				throw exception_io_data();
			}
			else {
				hprintf(L"Matroska: using '%S' decoder.\n", (const char*)codec_temp);
			}
		}

		if (!p_decode && m_decoder->analyze_first_frame_supported()) {
			void* ptr = NULL;
			t_size bytes = 0;
			bool ret = m_parser->ReadFirstFrame(&ptr, &bytes);
			if (ret)
			{
				m_decoder->analyze_first_frame(ptr, bytes, p_abort);
				m_parser->Seek(0);
			}
		}
	}

	//! See: input_info_writer::set_info(). Valid after open() with input_open_info_write reason.
	void retag_set_info(t_uint32 p_subsong,const file_info & p_info,abort_callback & p_abort) {
		hprintf(L"Nrg: retag_set_info()\n");

		if (m_parser == NULL) throw exception_io_unsupported_format();
	}
	
	//! See: input_info_writer::commit(). Valid after open() with input_open_info_write reason.
	void retag_commit(abort_callback & p_abort) {
		hprintf(L"Nrg: retag_commit()\n");
	}

	void remove_tags(abort_callback& abort) {
		hprintf(L"Nrg: remove_tags()\n");
	}

	//! See: input_entry::is_our_content_type().
	/* identify nrg by content type */
	static bool g_is_our_content_type(const char * p_content_type) {
		bool ret = false;
		hprintf(L"Nrg: g_is_our_content_type() p_content_type=%S\n", p_content_type);
		return ret;
	}
	
	//! See: input_entry::is_our_path().
	/* identify nrg by file extension */
	static bool g_is_our_path(const char * p_path,const char * p_extension) {
		bool ret = stricmp_utf8(p_extension,"nrg") == 0;
		hprintf(L"Nrg: g_is_our_path() p_path=%S p_extension=%S\n", p_path, p_extension);
		return ret;
	}

	//! See: input_entry::get_guid().
	static const GUID g_get_guid() {
		// {EE6CE130-A5A5-4D1C-8314-2701DF15E7D8}
		static const GUID foo_input_nrg_GUID =
		{ 0xee6ce130, 0xa5a5, 0x4d1c, { 0x83, 0x14, 0x27, 0x1, 0xdf, 0x15, 0xe7, 0xd8 } };
		return foo_input_nrg_GUID;
	}
	
	//! See: input_entry::get_name().
	static const char * g_get_name() { return "nrg decoder"; }

protected:
	void cleanup()
	{
	}

	int64 duration_to_samples(double val)
	{
        return audio_math::time_to_samples(val, m_expected_sample_rate);
	}

	double samples_to_duration(int64 val)
	{
        return audio_math::samples_to_time(val, m_expected_sample_rate);
	}

	void set_current_track(unsigned int p_subsong) {
		m_TrackNo = p_subsong;
		m_parser->SetCurrentTrack(m_TrackNo);
		m_parser->Seek(0);
		hprintf(L"Nrg: set_current_track(): m_TrackNo=%d\n", m_TrackNo);
	}
};

/**
 * plugin factory 
 * @{
 */
static input_factory_t<input_nrg> g_input_nrg_factory;
DECLARE_COMPONENT_VERSION(
	"NRG input",
	"1.0.0",
	"https://github.com/tsingkong/foo_input_nrg/; 2022: TSiNGKONG"
);

DECLARE_FILE_TYPE("Nero Disk Image files","*.NRG");
/**
 * @}
 */