
#include "nrg_parser.h"
#include "../nrg_img/include/aimg.h"
#include "../nrg_img/include/aimg_nrg.h"
#include "nrg_foo_stream.h"

#include <string>
using namespace std;

bool starts_with(const string &s, const char *start) {
  return strncmp(s.c_str(), start, strlen(start)) == 0;
};

uint64 NrgAudioParser::SecondsToTimecode(double seconds)
{
	return (uint64)floor(seconds * 1000000000);
};

double NrgAudioParser::TimecodeToSeconds(uint64 code,unsigned samplerate_hint)
{
	return ((double)(int64)code / 1000000000);
};

NrgAudioParser::NrgAudioParser(const char* p_path, abort_callback & p_abort)
{
	m_FileDate = 0;
	m_Duration = 0;
	m_CurrentTimecode = 0;
	m_FileSize = -1;
	m_File_Path = p_path;
	if (m_File_Path.substr(0, 7).compare("file://") == 0)
	{
		m_File_Path = m_File_Path.substr(7, m_File_Path.size());
	}
	m_CurrentTrackNo = 0;
	m_Nrg_Img = NULL;
	m_Sector_Buffer = m_Track_Buffer = NULL;
	m_Sector_Buff_Size = m_Track_Buff_Size = 0;
};

NrgAudioParser::NrgAudioParser(service_ptr_t<file> input, abort_callback& p_abort)
{
	m_FileDate = 0;
	m_Duration = 0;
	m_CurrentTimecode = 0;
	m_FileSize = input->get_size(p_abort);
	m_FileFoo = input;
	m_File_Path = "";
	m_CurrentTrackNo = 0;
	m_Nrg_Img = NULL;
	m_Sector_Buffer = m_Track_Buffer = NULL;
	m_Sector_Buff_Size = m_Track_Buff_Size = 0;
};

NrgAudioParser::~NrgAudioParser() {
	if (m_Sector_Buffer != NULL)
	{
		delete[]m_Sector_Buffer;
	}
	if (m_Track_Buffer != NULL)
	{
		delete[]m_Track_Buffer;
	}
};

int NrgAudioParser::Parse(bool bInfoOnly, bool bBreakAtClusters) 
{
	if (m_Nrg_Img != NULL)
	{
		return 0;
	}
	try {
		if (m_File_Path.empty()) {
			aimg_istream_t* aimg_stream = nrg_foo_stream_new(m_FileFoo);
			m_Nrg_Img = nrg_open(NULL, aimg_stream);
		}
		else {
			m_Nrg_Img = nrg_open(m_File_Path.c_str(), NULL);
		}
	} catch (std::exception &) {
		return 1;
	} catch (...) {
		return 1;
	}

	return 0;
};

void NrgAudioParser::SetTags(const file_info &info)
{

};

// in second
double NrgAudioParser::GetDuration() { 
	aimg_track_t* track = m_Nrg_Img->tracks[m_CurrentTrackNo];
	long sector_count = aimg_track_length(track);
	long sector_size = aimg_track_sector_size(track);
	// 32Bits
	m_Duration = (double)sector_count* sector_size / 4 / 44100;
	return m_Duration;
};

// 返回当前 Track 的文件大小
t_uint64 NrgAudioParser::GetTrackSize() {
	aimg_track_t* track = m_Nrg_Img->tracks[m_CurrentTrackNo];
	long sector_count = aimg_track_length(track);
	long sector_size = aimg_track_sector_size(track);
	return sector_count * sector_size;
}

std::string NrgAudioParser::GetCurrentTrackTitle() {
	if (m_Nrg_Img->tracks[m_CurrentTrackNo]->text.title == NULL) {
		return "Track " + std::to_string(m_CurrentTrackNo);
	}
	return m_Nrg_Img->tracks[m_CurrentTrackNo]->text.title;
}

int32 NrgAudioParser::GetFirstAudioTrack()
{
	if (m_Nrg_Img == NULL)
		return -1;
	return m_Nrg_Img->tfirst;
}

uint32 NrgAudioParser::GetAudioTrackCount()
{
	if (m_Nrg_Img == NULL)
		return 0;
	
	return m_Nrg_Img->tlast - m_Nrg_Img->tfirst + 1;
}

bool NrgAudioParser::SetFB2KInfo(file_info &info, t_uint32 p_subsong)
{
	p_subsong += m_Nrg_Img->tfirst;
	// Last chance,
	if ((p_subsong >= m_Nrg_Img->tfirst) && (p_subsong <= m_Nrg_Img->tlast))
	{
		// UTF-8?
		info.meta_set("TITLE", m_Nrg_Img->tracks[p_subsong]->text.title);
		char trackNumberString[32];
		sprintf(trackNumberString, "%d", p_subsong - m_Nrg_Img->tfirst + 1);
		info.meta_set("TRACKNUMBER", trackNumberString);
		info.meta_set("ALBUM", m_Nrg_Img->text.title);
	}

	return true;
};

void NrgAudioParser::SetCurrentTrack(uint32 newTrackNo)
{
	m_CurrentTrackNo = newTrackNo + m_Nrg_Img->tfirst;
};

// kbps
int32 NrgAudioParser::GetAvgBitrate() 
{ 
	double ret = 44100 * 32 / 1000;
	return static_cast<int32>(ret);
};

bool NrgAudioParser::Seek(double seconds)
{
	aimg_track_t* track = m_Nrg_Img->tracks[m_CurrentTrackNo];
	long nSector = aimg_track_first_sector(track);
	long sector_size = aimg_track_sector_size(track);
	off_t off = seconds * 44100 * 4 + nSector * sector_size;
	off = aimg_istream_seek(track->stream, off);
	return true;
};

bool NrgAudioParser::Read(void** ptr, t_size* bytes)
{
	aimg_track_t* track = m_Nrg_Img->tracks[m_CurrentTrackNo];
	long sector_size = aimg_track_sector_size(track);
	if (sector_size > m_Sector_Buff_Size)
	{
		if (m_Sector_Buffer != NULL)
		{
			delete[]m_Sector_Buffer;
		}
		m_Sector_Buffer = new uint8[sector_size];
		m_Sector_Buff_Size = sector_size;
	}
	if (m_Sector_Buffer != NULL)
	{
		memset(m_Sector_Buffer, 0, m_Sector_Buff_Size);
	}
	*bytes = aimg_istream_read(track->stream, m_Sector_Buffer, m_Sector_Buff_Size);
	if (*bytes <= 0)
		return false;
	*ptr = m_Sector_Buffer;
	return true;
}

// 读取当前 Track 的数据，可能会包含多个 sector
bool NrgAudioParser::ReadCurrentTrackData(void** ptr, t_size* bytes)
{
	unsigned frames_to_skip = 0;
	double time_to_skip = 0;
	Seek(0);
	t_size to_read = *bytes;
	if (to_read > m_Track_Buff_Size)
	{
		if (m_Track_Buffer != NULL)
		{
			delete[]m_Track_Buffer;
		}
		m_Track_Buffer = new uint8[to_read];
		m_Track_Buff_Size = to_read;
	}
	if (m_Track_Buffer != NULL)
	{
		memset(m_Track_Buffer, 0, m_Track_Buff_Size);
	}

	t_size read_ok = 0;
	while (read_ok < to_read) {
		t_size read = to_read - read_ok;
		void* pSectorBuffer = NULL;
		if (!Read(&pSectorBuffer, &read)) {
			break;
		}
		memcpy(m_Track_Buffer + read_ok, pSectorBuffer, read);
		read_ok += read;
	}
	*bytes = read_ok;
	*ptr = m_Track_Buffer;
	return true;
}

bool NrgAudioParser::ReadFirstFrame(void** ptr, t_size* bytes)
{
	Seek(0);
	aimg_track_t* track = m_Nrg_Img->tracks[m_CurrentTrackNo];
	memset(m_FirstFrame, 0, sizeof(m_FirstFrame));
	*bytes = aimg_istream_read(track->stream, m_FirstFrame, 4);
	if (*bytes <= 0)
		return false;
	*ptr = m_FirstFrame;
	return true;
}
