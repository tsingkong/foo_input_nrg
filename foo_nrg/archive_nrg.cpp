#define MY_VERSION "1.0"

#include "../foo_sdk/foobar2000/SDK/foobar2000.h"
#include "nrg_parser.h"
#include "nrg_foo_stream.h"

static void handle_error( const char * str )
{
	if ( str ) throw exception_io_data( str );
}

class archive_nrg : public archive_impl
{
public:
	virtual bool supports_content_types()
	{
		return false;
	}

	virtual const char * get_archive_type()
	{
		return "nrg";
	}

	virtual t_filestats get_stats_in_archive( const char * p_archive, const char * p_file, abort_callback & p_abort )
	{
		service_ptr_t< file > m_file;
		filesystem::g_open( m_file, p_archive, filesystem::open_mode_read, p_abort );

		NrgAudioParser parser(m_file, p_abort);
		if (parser.Parse(true, true) != 0) {
			console::error("Nrg: Invalid Nrg file.");
			throw exception_io_unsupported_format();
		}

		int p_subsong = atoi(p_file);	// 1,2,3,...
		if (p_subsong <= 0) {
			console::error("p_subsong <= 0");
			throw exception_io_unsupported_format();
		}

		parser.SetCurrentTrack(p_subsong - 1);

		t_filestats ret;
		ret.m_size = parser.GetTrackSize();
		ret.m_timestamp = m_file->get_timestamp( p_abort );
		return ret;
	}

	virtual void open_archive( service_ptr_t< file > & p_out, const char * p_archive, const char * p_file, abort_callback & p_abort )
	{
		service_ptr_t< file > m_file;
		filesystem::g_open( m_file, p_archive, filesystem::open_mode_read, p_abort );
		t_filetimestamp timestamp = m_file->get_timestamp( p_abort );
		
		NrgAudioParser parser(m_file, p_abort);
		if (parser.Parse(true, true) != 0) {
			console::error("Nrg: Invalid Nrg file.");
			throw exception_io_unsupported_format();
		}

		int p_subsong = atoi(p_file);	// 1,2,3,...
		if (p_subsong <= 0) {
			console::error("p_subsong <= 0");
			throw exception_io_unsupported_format();
		}

		parser.SetCurrentTrack(p_subsong - 1);

		t_size size = parser.GetTrackSize();
		void* data;
		parser.ReadCurrentTrackData(&data, &size);
		p_out = new service_impl_t<reader_membuffer_simple>( data, size, timestamp, m_file->is_remote() );
	}

	virtual void archive_list( const char * path, const service_ptr_t< file > & p_reader, archive_callback & p_out, bool p_want_readers )
	{
		if ( stricmp_utf8( pfc::string_extension( path ), "nrg" ) )
			throw exception_io_data();

		service_ptr_t< file > m_file = p_reader;
		if ( m_file.is_empty() )
			filesystem::g_open( m_file, path, filesystem::open_mode_read, p_out );
		else
			m_file->reopen( p_out );

		t_filetimestamp timestamp = m_file->get_timestamp( p_out );

		abort_callback_dummy p_abort;
		NrgAudioParser parser(m_file, p_abort);
		if (parser.Parse(true, true) != 0) {
			console::error("Nrg: Invalid Nrg file.");
			throw exception_io_unsupported_format();
		}

		pfc::string8_fastalloc m_path;
		service_ptr_t<file> m_out_file;
		t_filestats m_stats;
		m_stats.m_timestamp = timestamp;

		for (int i = 0; i < parser.GetAudioTrackCount(); i++) {
			parser.SetCurrentTrack(i);
			std::string sTitle = parser.GetCurrentTrackTitle();
			sTitle = std::to_string(i + 1) + "." + sTitle + ".dts";
			make_unpack_path(m_path, path, sTitle.c_str());
			m_stats.m_size = parser.GetTrackSize();
			if (p_want_readers)
			{
				t_size size = parser.GetTrackSize();
				void* data;
				parser.ReadCurrentTrackData(&data, &size);
				m_out_file = new service_impl_t<reader_membuffer_simple>(data, size, timestamp, m_file->is_remote());
			}
			if (!p_out.on_entry(this, m_path, m_stats, m_out_file)) break;
		}
	}

	virtual bool is_our_archive(const char* path) {
		int path_length = strlen(path);
		if (path_length < 4) {
			return false;
		}
		bool ret = stricmp_utf8(&(path[path_length-4]), ".nrg") == 0;
		console::info("is_our_archive");
		return ret;
	}
};

class unpacker_nrg : public unpacker
{
public:
	virtual void open( service_ptr_t< file > & p_out, const service_ptr_t< file > & p_source, abort_callback & p_abort )
	{
		if ( p_source.is_empty() ) throw exception_io_data();

		t_filetimestamp timestamp = p_source->get_timestamp(p_abort);

		NrgAudioParser parser(p_source, p_abort);
		if (parser.Parse(true, true) != 0) {
			console::error("Nrg: Invalid Nrg file.");
			throw exception_io_unsupported_format();
		}

		for (int i = 0; i < parser.GetAudioTrackCount(); ++i) {
			parser.SetCurrentTrack(i);
			t_size size = parser.GetTrackSize();
			void* data;
			parser.ReadCurrentTrackData(&data, &size);
			p_out = new service_impl_t<reader_membuffer_simple>(data, size, p_source->get_timestamp(p_abort), p_source->is_remote());
			return;
		}
		throw exception_io_data();
	}
};

static archive_factory_t < archive_nrg >  g_archive_nrg_factory;
static unpacker_factory_t< unpacker_nrg > g_unpacker_nrg_factory;

DECLARE_COMPONENT_VERSION("NRG Reader", MY_VERSION, "TSiNGKONG");

DECLARE_FILE_TYPE_EX("nrg", "Nero Disk Image files", "Nero Disk Image files");

VALIDATE_COMPONENT_FILENAME("foo_archive_nrg.dll");
