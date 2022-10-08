#ifndef _NRG_PARSER_H_
#define _NRG_PARSER_H_

#include "../foo_sdk/foobar2000/SDK/foobar2000.h"
#include "../foo_sdk/foobar2000/helpers/helpers.h"
#include "../foo_sdk/pfc/pfc.h"
// #include "../nrg_img/include/aimg.h"
#include "../nrg_img/include/aimg_types.h"
#include "DbgOut.h"
#include <string>
#include <memory>

class aimg_img_t;

class NrgAudioParser {
public:
	NrgAudioParser(service_ptr_t<file> input, abort_callback & p_abort);
	NrgAudioParser(const char* p_path, abort_callback& p_abort);
	~NrgAudioParser();

	/// The main header parsing function
	/// \return 0 File parsed ok
	/// \return 1 Failed
	int Parse(bool bInfoOnly = false, bool bBreakAtClusters = true);

	/// Set the info tags to the current tags file in memory
	void SetTags(const file_info &info);

	uint64 GetTimecodeScale() { return m_TimecodeScale; };

	/// Returns an adjusted duration of current track
	double GetDuration();

	// 返回当前 Track 的文件大小
	t_uint64 GetTrackSize();

	/// Returns the track index of the first decodable track
	int32 GetFirstAudioTrack();

	double TimecodeToSeconds(uint64 code,unsigned samplerate_hint = 44100);

	uint64 SecondsToTimecode(double seconds);

	/// Set the fb2k info from the nrg file
	/// \param info This will be filled up with tags ;)
	bool SetFB2KInfo(file_info &info, t_uint32 p_subsong);

	/// Get the foobar2000 style format string
//	const char *GetFoobar2000Format(uint16 trackNo, bool bSetupCodecPrivate = true);

	/// Set the current track to read data from, 0 based
	void SetCurrentTrack(uint32 newTrackNo);

	std::string GetCurrentTrackTitle();

	uint32 GetAudioTrackCount();

	int32 GetAvgBitrate();

	/// Seek to a position of current track
	/// \param seconds The absolute position to seek to, in seconds			
	/// \return Current postion offset from where it was requested
	/// If you request to seek to 2.0 and we can only seek to 1.9
	/// the return value would be 100 * m_TimcodeScale
	bool Seek(double seconds);

	// 顺序读取 nrg img，不一定是当前 Track
	bool Read(void** ptr, t_size* bytes);

	// 读取当前 Track 的数据，可能会包含多个 sector
	bool ReadCurrentTrackData(void** ptr, t_size* bytes);

	// 读取当前 Track 的第一帧内容
	bool ReadFirstFrame(void** ptr, t_size* bytes);

protected:
	
	uint32 m_CurrentTrackNo;
	
	uint64 m_CurrentTimecode;
	double m_Duration;
	uint64 m_TimecodeScale;
	
	int32 m_FileDate;

	uint64 m_FileSize;

	std::string m_File_Path;
	service_ptr_t<file> m_FileFoo;

private:
	aimg_img_t* m_Nrg_Img;

	uint8* m_Sector_Buffer;
	uint32 m_Sector_Buff_Size;

	uint8* m_Track_Buffer;
	uint32 m_Track_Buff_Size;

	uint8 m_FirstFrame[4];
};

typedef std::shared_ptr<NrgAudioParser> nrg_parser_ptr;

#endif // _NRG_PARSER_H_
