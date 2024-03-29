#include "foobar2000.h"

void t_replaygain_config::reset()
{
	m_source_mode = source_mode_none;
	m_processing_mode = processing_mode_none;
	m_preamp_without_rg = 0;
	m_preamp_with_rg = 0;
}

audio_sample t_replaygain_config::query_scale(const file_info & info) const
{
	return query_scale(info.get_replaygain());
}

audio_sample t_replaygain_config::query_scale(const replaygain_info & info) const {
	const audio_sample peak_margin = 1.0;//used to be 0.999 but it must not trigger on lossless

	audio_sample peak = peak_margin;
	audio_sample gain = 0;

	bool have_rg_gain = false, have_rg_peak = false;

	if (m_source_mode == source_mode_track || m_source_mode == source_mode_album)
	{
		if (m_source_mode == source_mode_track)
		{
			if (info.is_track_gain_present()) {gain = info.m_track_gain; have_rg_gain = true; }
			else if (info.is_album_gain_present()) {gain = info.m_album_gain; have_rg_gain = true; }
			if (info.is_track_peak_present()) {peak = info.m_track_peak; have_rg_peak = true; }
			else if (info.is_album_peak_present()) {peak = info.m_album_peak; have_rg_peak = true; }
		}
		else
		{
			if (info.is_album_gain_present()) {gain = info.m_album_gain; have_rg_gain = true; }
			else if (info.is_track_gain_present()) {gain = info.m_track_gain; have_rg_gain = true; }
			if (info.is_album_peak_present()) {peak = info.m_album_peak; have_rg_peak = true; }
			else if (info.is_track_peak_present()) {peak = info.m_track_peak; have_rg_peak = true; }
		}
	}

	gain += have_rg_gain ? m_preamp_with_rg : m_preamp_without_rg;

	audio_sample scale = 1.0;

	if (m_processing_mode == processing_mode_gain || m_processing_mode == processing_mode_gain_and_peak)
	{
		scale *= audio_math::gain_to_scale(gain);
	}

	if ((m_processing_mode == processing_mode_peak || m_processing_mode == processing_mode_gain_and_peak) && have_rg_peak)
	{
		if (scale * peak > peak_margin)
			scale = (audio_sample)(peak_margin / peak);
	}

	return scale;
}

audio_sample t_replaygain_config::query_scale(const metadb_handle_ptr & p_object) const
{
	return query_scale(p_object->get_async_info_ref()->info());
}

audio_sample replaygain_manager::core_settings_query_scale(const file_info & p_info)
{
	t_replaygain_config temp;
	get_core_settings(temp);
	return temp.query_scale(p_info);
}

audio_sample replaygain_manager::core_settings_query_scale(const metadb_handle_ptr & info)
{
	t_replaygain_config temp;
	get_core_settings(temp);
	return temp.query_scale(info);
}

//enum t_source_mode {source_mode_none,source_mode_track,source_mode_album};
//enum t_processing_mode {processing_mode_none,processing_mode_gain,processing_mode_gain_and_peak,processing_mode_peak};

static const char* querysign(int val) {
	return val < 0 ? "-" : val>0 ? "+" : "\xc2\xb1";
}

static pfc::string_fixed_t<128> format_dbdelta(double p_val) {
	pfc::string_fixed_t<128> ret;
	int val = (int)pfc::rint32(p_val * 10);
	ret << querysign(val) << (abs(val) / 10) << "." << (abs(val) % 10) << "dB";
	return ret;
}

void t_replaygain_config::format_name(pfc::string_base & p_out) const
{
	switch(m_processing_mode)
	{
	case processing_mode_none:
		p_out = "None.";
		break;
	case processing_mode_gain:
		switch(m_source_mode)
		{
		case source_mode_none:
			if (m_preamp_without_rg == 0) p_out = "None."; 
			else p_out = PFC_string_formatter() << "Preamp : " << format_dbdelta(m_preamp_without_rg);
			break;
		case source_mode_track:
			{
				pfc::string_formatter fmt;
				fmt << "Apply track gain";
				if (m_preamp_without_rg != 0 || m_preamp_with_rg != 0) fmt << ", with preamp";
				fmt << ".";
				p_out = fmt;
			}
			break;
		case source_mode_album:
			{
				pfc::string_formatter fmt;
				fmt << "Apply album gain";
				if (m_preamp_without_rg != 0 || m_preamp_with_rg != 0) fmt << ", with preamp";
				fmt << ".";
				p_out = fmt;
			}
			break;
		};
		break;
	case processing_mode_gain_and_peak:
		switch(m_source_mode)
		{
		case source_mode_none:
			if (m_preamp_without_rg >= 0) p_out = "None.";
			else p_out = PFC_string_formatter() << "Preamp : " << format_dbdelta(m_preamp_without_rg);
			break;
		case source_mode_track:
			{
				pfc::string_formatter fmt;
				fmt << "Apply track gain";
				if (m_preamp_without_rg != 0 || m_preamp_with_rg != 0) fmt << ", with preamp";
				fmt << ", prevent clipping according to track peak.";
				p_out = fmt;
			}
			break;
		case source_mode_album:
			{
				pfc::string_formatter fmt;
				fmt << "Apply album gain";
				if (m_preamp_without_rg != 0 || m_preamp_with_rg != 0) fmt << ", with preamp";
				fmt << ", prevent clipping according to album peak.";
				p_out = fmt;
			}
			break;
		};
		break;
	case processing_mode_peak:
		switch(m_source_mode)
		{
		case source_mode_none:
			p_out = "None.";
			break;
		case source_mode_track:
			p_out = "Prevent clipping according to track peak.";
			break;
		case source_mode_album:
			p_out = "Prevent clipping according to album peak.";
			break;
		};
		break;
	}
}

bool t_replaygain_config::is_active() const
{
	switch(m_processing_mode)
	{
	case processing_mode_none:
		return false;
	case processing_mode_gain:
		switch(m_source_mode)
		{
		case source_mode_none:
			return m_preamp_without_rg != 0;
		case source_mode_track:
			return true;
		case source_mode_album:
			return true;
		};
		return false;
	case processing_mode_gain_and_peak:
		switch(m_source_mode)
		{
		case source_mode_none:
			return m_preamp_without_rg < 0;
		case source_mode_track:
			return true;
		case source_mode_album:
			return true;
		};
		return false;
	case processing_mode_peak:
		switch(m_source_mode)
		{
		case source_mode_none:
			return false;
		case source_mode_track:
			return true;
		case source_mode_album:
			return true;
		};
		return false;
	default:
		return false;
	}
}


replaygain_scanner::ptr replaygain_scanner_entry::instantiate( uint32_t flags ) {
	replaygain_scanner_entry_v2::ptr p2;
	if ( p2 &= this ) return p2->instantiate( flags );
	else return instantiate();
}