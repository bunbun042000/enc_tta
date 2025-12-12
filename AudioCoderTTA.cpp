/*
The ttaplugins-winamp project.
Copyright (C) 2005-2026 Yamagata Fumihiro

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <nsv/enc_if.h>

#include <libtta.h>

#include <malloc.h>
#include "AudioCoderTTA.h"
#include "tta_encoder_exted.h"

TTAint32 CALLBACK write_callback(TTA_io_callback* io, TTAuint8* buffer, TTAuint32 size)
{
	TTA_io_callback_wrapper* iocb = (TTA_io_callback_wrapper*)io;

	if (iocb->remain_data_buffer.data_length > iocb->remain_data_buffer.current_end_pos + size)
	{
		memcpy_s(iocb->remain_data_buffer.buffer + iocb->remain_data_buffer.current_end_pos,
			iocb->remain_data_buffer.data_length - iocb->remain_data_buffer.current_end_pos, buffer, size);
		iocb->remain_data_buffer.current_end_pos += size;
		return size;
	}
	else
	{
		// Do nothing
	}
	return 0;
} // write_callback

TTAint64 CALLBACK seek_callback(TTA_io_callback* io, TTAint64 offset) {
	TTA_io_callback_wrapper* iocb = (TTA_io_callback_wrapper*)io;
	if (iocb->remain_data_buffer.current_end_pos > offset)
	{
		iocb->remain_data_buffer.current_pos = offset;
		return offset;
	}
	else
	{
		// Do nothing
	}
	return 0;
} // seek_callback

AudioCoderTTA::AudioCoderTTA() : AudioCoder()
{
}

AudioCoderTTA::AudioCoderTTA(int nch, int srate, int bps) : AudioCoder()
{

	iocb_wrapper.remain_data_buffer.buffer = nullptr;
	iocb_wrapper.remain_data_buffer.data_length = 0;
	iocb_wrapper.remain_data_buffer.current_pos = 0;
	iocb_wrapper.remain_data_buffer.current_end_pos = 0;

	iocb_wrapper.iocb.read = nullptr;
	iocb_wrapper.iocb.write = &write_callback;
	iocb_wrapper.iocb.seek = &seek_callback;

	//	smp_size = nch * bps >> 3;
	info.nch = nch;
	info.bps = bps;
	info.sps = srate;
	info.format = TTA_FORMAT_SIMPLE;
	info.samples = 0;
	smp_size = nch * ((bps + 7) / 8);
	samplecount = 0;

	// check for supported formats
	if ((info.nch == 0) ||
		(info.nch > MAX_NCH) ||
		(info.bps == 0) ||
		(info.bps > MAX_BPS))
	{
		throw AudioCoderTTA_exception(TTA_FORMAT_ERROR);
	}
	else
	{
		// Do nothing
	}

	if (info.samples == 0)
	{
		info.samples = MAX_SAMPLES;
	}
	else
	{
		// Do nothing
	}

	iocb_wrapper.remain_data_buffer.data_length = PCM_BUFFER_LENGTH * smp_size + 4; // +4 for READ_BUFFER macro

	// allocate memory for PCM buffer
	iocb_wrapper.remain_data_buffer.buffer = (TTAuint8*)_aligned_malloc(iocb_wrapper.remain_data_buffer.data_length, 16); 
	// encoding unit size
	buffer_size = PCM_BUFFER_LENGTH * smp_size;

	try
	{
		TTA = new (&ttaenc_mem) tta_encoder_extend((TTA_io_callback*)&iocb_wrapper);
	}

	catch (tta::tta_exception& ex)
	{
		if (nullptr != TTA)
		{
			reinterpret_cast<tta_encoder_extend*>(TTA)->~tta_encoder_extend();
			TTA = nullptr;
			data_buf_free(&iocb_wrapper.remain_data_buffer);
			throw AudioCoderTTA_exception(TTA_MEMORY_ERROR);
		}
		else
		{
			// Do nothing
		}
	}
	TTA->init_set_info_for_memory(&info, 0);
}

void AudioCoderTTA::data_buf_free(data_buf *databuf)
{
	if (databuf->buffer != nullptr)
	{
		_aligned_free(databuf->buffer);
		databuf->buffer = nullptr;
	}
	else
	{
		// Do nothing
	}
	databuf->current_pos = 0;
	databuf->data_length = 0;
	databuf->current_end_pos = 0;

}

__forceinline int AudioCoderTTA::write_output(TTAuint8 *out, int out_avail, int out_used_total)
{
	int out_used = 0;
	if (iocb_wrapper.remain_data_buffer.current_pos < iocb_wrapper.remain_data_buffer.current_end_pos) // write any header
	{
		int l = min(out_avail - out_used_total, (int)(iocb_wrapper.remain_data_buffer.current_end_pos - iocb_wrapper.remain_data_buffer.current_pos));
		memcpy_s(out + out_used_total, out_avail - out_used_total, iocb_wrapper.remain_data_buffer.buffer + iocb_wrapper.remain_data_buffer.current_pos, l);
		out_used += l;
		iocb_wrapper.remain_data_buffer.current_pos += l;

	}
	else if (iocb_wrapper.remain_data_buffer.current_pos == iocb_wrapper.remain_data_buffer.current_end_pos)
	{
		iocb_wrapper.remain_data_buffer.current_pos = 0;
		iocb_wrapper.remain_data_buffer.current_end_pos = 0;
	}
	else
	{
		// Do nothing
	}

	return out_used;
}

int AudioCoderTTA::Encode(int framepos, void *in0, int in_avail, int *in_used, void *out0, int out_avail)
{
	int ret = -1;
	int out_used_total = 0;
	int out_used = 0;
	*in_used = 0;
	TTAuint8 * in = (TTAuint8*)in0;
	TTAuint8 * out = (TTAuint8*)out0;

	for (;;)
	{
		out_used = write_output(out, out_avail, out_used_total);
		if (out_used)
		{
			out_used_total += out_used;
			if (out_avail == out_used_total)
			{
				break;
			}
			else
			{
				// Do nothing
			}
		}
		else // encode more
		{
			int l = min((int)(buffer_size), in_avail - *in_used);
			if (l > 0 || (lastblock == 1 && in_avail == *in_used))
			{
				samplecount += l / smp_size;
				TTA->process_stream(in + *in_used, l);
				*in_used += l;

				if (lastblock)
				{
					TTA->preliminaryFinish();
					lastblock = 2;
				}
				else
				{
					// Do nothing
				}
			}
			else
			{
				break;
			}
		}
	}
	return out_used_total;
}

AudioCoderTTA::~AudioCoderTTA()
{

	data_buf_free(&iocb_wrapper.remain_data_buffer);

	//	smp_size = nch * bps >> 3;
	info.nch = 0;
	info.bps = 0;
	info.sps = 0;
	info.format = TTA_FORMAT_SIMPLE;
	info.samples = 0;
	samplecount = 0;

	if (nullptr != TTA)
	{
		reinterpret_cast<tta_encoder_extend*>(TTA)->~tta_encoder_extend();
		TTA = nullptr;
	}
	else
	{
		// Do nothing
	}

} // ~AudioCoderTTA

void AudioCoderTTA::PrepareToFinish()
{
	lastblock = 1;
}

void AudioCoderTTA::FinishAudio(const wchar_t *filename)
{
	info.samples = samplecount;

	std::wstring lpTempPathBuffer;
	wchar_t szTempFileName[MAX_PATHLEN];

	DWORD dwBytesWritten = 0;
	DWORD dwBytesRead = 0;
	const size_t BUFSIZE = 65536;
	TTAuint8 *chBuffer;
	chBuffer = new TTAuint8[BUFSIZE];

	if (wcsnlen(filename, MAX_PATHLEN) > MAX_PATH)
	{
		throw AudioCoderTTA_exception(TTA_WRITE_ERROR);
		return;
	}
	else
	{
		// Do nothing
	}

	std::wstring temppath = std::wstring(filename);
	size_t lastposofpath;
	lastposofpath = temppath.rfind(L'\\');

	if (lastposofpath != std::string::npos)
	{
		lpTempPathBuffer = temppath.substr(0, lastposofpath).c_str();
	}
	else
	{
		throw AudioCoderTTA_exception(TTA_WRITE_ERROR);
		return;
	}

	UINT uRetVal = GetTempFileNameW(lpTempPathBuffer.c_str(), // directory for tmp files
		L"enc",     // temp file name prefix 
		0,                // create unique name 
		szTempFileName);  // buffer for name 

	if (uRetVal == 0)
	{
		throw AudioCoderTTA_exception(TTA_WRITE_ERROR);
		return;
	}
	else
	{
		// Do nothing
	}

	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hTempFile = INVALID_HANDLE_VALUE;

	hFile = CreateFileW(filename,               // file name 
		GENERIC_READ,         // open for reading 
		0,                     // do not share 
		nullptr,				   // default security 
		OPEN_EXISTING,         // existing file only 
		FILE_ATTRIBUTE_NORMAL, // normal file 
		nullptr);                 // no template 

	if (hFile == INVALID_HANDLE_VALUE)
	{
		throw AudioCoderTTA_exception(TTA_READ_ERROR);
		return;
	}
	else
	{
		// Do nothing
	}

	BOOL fSuccess = FALSE;

	// Write header
	hTempFile = CreateFileW(szTempFileName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	TTA->init_set_info_for_memory(&info, 0);
	TTA->flushFifo();

	fSuccess = WriteFile(hTempFile, (iocb_wrapper.remain_data_buffer.buffer + iocb_wrapper.remain_data_buffer.current_pos),
		(DWORD)(TTA->getHeaderOffset()), &dwBytesWritten, nullptr);

	if (!fSuccess)
	{
		throw AudioCoderTTA_exception(TTA_WRITE_ERROR);
		return;
	}
	else
	{
		// Do nothing
	}

	// Write seek table
	DWORD dwRetVal = SetFilePointer(hTempFile, (LONG)TTA->getHeaderOffset(), nullptr, FILE_BEGIN);
	iocb_wrapper.remain_data_buffer.current_pos = 0;
	iocb_wrapper.remain_data_buffer.current_end_pos = 0;

	TTA->finalize();

	fSuccess = WriteFile(hTempFile, iocb_wrapper.remain_data_buffer.buffer + iocb_wrapper.remain_data_buffer.current_pos,
		(DWORD)(iocb_wrapper.remain_data_buffer.current_end_pos - iocb_wrapper.remain_data_buffer.current_pos), &dwBytesWritten, nullptr);

	if (!fSuccess)
	{
		throw AudioCoderTTA_exception(TTA_WRITE_ERROR);
		return;
	}
	else
	{
		// Do nothing
	}

	dwRetVal = SetFilePointer(hFile, (LONG)TTA->getHeaderOffset(), nullptr, FILE_BEGIN);

	do
	{
		if (ReadFile(hFile, chBuffer, BUFSIZE, &dwBytesRead, nullptr))
		{
			fSuccess = WriteFile(hTempFile, chBuffer, dwBytesRead, &dwBytesWritten, nullptr);
			if (!fSuccess)
			{
				data_buf_free(&iocb_wrapper.remain_data_buffer);
				throw AudioCoderTTA_exception(TTA_WRITE_ERROR);
				return;
			}
			else
			{
				// Do nothing
			}
		}
		else
		{
			data_buf_free(&iocb_wrapper.remain_data_buffer);
			throw AudioCoderTTA_exception(TTA_READ_ERROR);
			return;
		}
		//  Continues until the whole file is processed.
	} while (dwBytesRead == BUFSIZE);

	if (!CloseHandle(hFile))
	{
		data_buf_free(&iocb_wrapper.remain_data_buffer);
		throw AudioCoderTTA_exception(TTA_FILE_ERROR);
		return;
	}
	else
	{
		// Do nothing
	}

	if (!CloseHandle(hTempFile))
	{
		data_buf_free(&iocb_wrapper.remain_data_buffer);
		throw AudioCoderTTA_exception(TTA_FILE_ERROR);
		return;
	}
	else
	{
		// Do nothing
	}

	CopyFileW(szTempFileName, filename, FALSE);
	DeleteFileW(szTempFileName);

	data_buf_free(&iocb_wrapper.remain_data_buffer);
	delete[] chBuffer;
	chBuffer = nullptr;
}

void AudioCoderTTA::FinishAudio(const char *filename)
{
	wchar_t *wfilename = new wchar_t[MAX_PATHLEN + 1];
	size_t converted = 0;
	mbstowcs_s(&converted, wfilename, MAX_PATHLEN, filename, MAX_PATHLEN);
	FinishAudio(wfilename);
}
