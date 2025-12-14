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

#pragma once
#include <nsv/enc_if.h>
#include <memory.h>
#include <malloc.h>
#include <windows.h>
#include <stdexcept>
#include <stdlib.h>
#include <libtta.h>

#include "tta_encoder_exted.h"

static const int MAX_PATHLEN = 8192;
static const int PCM_BUFFER_LENGTH = 5210;

struct data_buf
{
	size_t	data_length;
	size_t	current_pos;
	size_t	current_end_pos;
	TTAuint8* buffer;
};

struct TTA_io_callback_wrapper
{
	TTA_io_callback iocb{};
	data_buf remain_data_buffer{};
};

/////////////////////// TTA encoder functions /////////////////////////
class AudioCoderTTA : public AudioCoder
{
public:
	AudioCoderTTA();
	AudioCoderTTA(int nch, int srate, int bps);
	int Encode(int framepos, void *in0, int in_avail, int *in_used, void *out0, int out_avail) override; //returns bytes in out
	~AudioCoderTTA();

	/* internal public functions */
	void PrepareToFinish();
	void FinishAudio(const wchar_t *filename);
	void FinishAudio(const char *filename);

protected:
	__forceinline int write_output(TTAuint8* out, int out_avail, int out_used_total);
	void data_buf_free(data_buf* databuf);

	TTA_info info = {};

	int lastblock = 0;
	TTAuint32 samplecount = 0;
	int smp_size = 0;

private:
	alignas(16) TTA_io_callback_wrapper iocb_wrapper ={};
	alignas(tta_encoder_extend) std::byte ttaenc_mem[sizeof(tta_encoder_extend)] = {};
	tta_encoder_extend* TTA = nullptr;

	int buffer_size = 0;

}; // class AudioCoderTTA

//////////////////////// TTA exception class //////////////////////////
class AudioCoderTTA_exception : public std::exception
{
	tta_error err_code;

public:
	AudioCoderTTA_exception(tta_error code) : err_code(code) {}
	tta_error code() const { return err_code; }
}; // class AudioCoderTTA_exception


