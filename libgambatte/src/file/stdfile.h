/***************************************************************************
Copyright (C) 2007 by Nach
http://nsrt.edgeemu.com

Copyright (C) 2007-2011 by sinamas <sinamas at users.sourceforge.net>
sinamas@users.sourceforge.net

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License version 2 for more details.

You should have received a copy of the GNU General Public License
version 2 along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA.
***************************************************************************/
#ifndef GAMBATTE_STD_FILE_H
#define GAMBATTE_STD_FILE_H

#include "file.h"
#include <string.h>

#if PICO_ON_DEVICE
// For panic()
#include "pico.h"
#include <cstring>
#else
#include <fstream>
#endif

// On-device, we have flash-resident blobs for ROMs etc, and these are used
// in-place (no room to copy into RAM).
//
// Off-device we still use standard C++ file IO, but we take (and leak!) a
// memory copy internally so that the "in-place" pointer can be passed around
// like it is on-device.

#if PICO_ON_DEVICE
// Static file table, ends with sentinel value with null name.
extern const file_entry file_manifest;
#endif

namespace gambatte {

class StdFile : public File {
public:
	explicit StdFile(char const *filename):
#if PICO_ON_DEVICE
	get_ptr_(0),
#else
	stream_(filename, std::ios::in | std::ios::binary),
#endif
	fsize_(0), flash_resource_(0)
	{
#if PICO_ON_DEVICE
		int i;
		for (i = 0; (&file_manifest)[i].name; ++i)
		{
			if (!strcmp((&file_manifest)[i].name, filename)) {
				printf("Found \"%s\" @%p\n", (&file_manifest)[i].name, (&file_manifest)[i].resource);
				fsize_ = (&file_manifest)[i].size;
				flash_resource_ = (&file_manifest)[i].resource;
				break;
			}
		}
		if (!(&file_manifest)[i].name) {
			panic("Couldn't find file %s\n", filename);
		}
#else
		if (stream_) {
			stream_.seekg(0, std::ios::end);
			fsize_ = stream_.tellg();
			stream_.seekg(0, std::ios::beg);
			// Create a memory copy for "zero-copy" use elsewhere. Leak the memory.
			char *buf = new char[fsize_];
			stream_.read(buf, fsize_);
			stream_.seekg(0, std::ios::beg);
			flash_resource_ = buf;
		}
#endif
	}

	virtual void rewind() {
#if PICO_ON_DEVICE
		get_ptr_ = 0;
#else
		stream_.seekg(0, std::ios::beg);
#endif
	}
	virtual std::size_t size() const { return fsize_; };
	virtual void read(char *buffer, std::size_t amount)
	{
#if PICO_ON_DEVICE
		if (fsize_ - get_ptr_ < amount)
			amount = fsize_ - get_ptr_;
		memcpy(buffer, flash_resource_, amount);
		get_ptr_ += amount;
#else
		stream_.read(buffer, amount);
#endif
	}
	virtual bool fail() const {
#if PICO_ON_DEVICE
		return flash_resource_ == NULL;
#else
		return stream_.fail();
#endif
	}

	// Get pointer to raw flash resource, for zero-copy read-only use
	virtual const char* getraw() const {return flash_resource_;}

private:
#if PICO_ON_DEVICE
	size_t get_ptr_;
#else
	std::ifstream stream_;
	std::size_t fsize_;
	const char *flash_resource_;
#endif
};

}

#endif
