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

// For panic()
#include "pico.h"

// This originally used regular C++ fstream. Now uses a static manifest of
// some blobs in flash (poor man's FS).

// Static file table, ends with sentinel value with null name.
extern const file_entry file_manifest;

namespace gambatte {

class StdFile : public File {
public:
	explicit StdFile(char const *filename)
	: fsize_(0), get_ptr_(0), flash_resource_(0)
	{
		int i;
		for (i = 0; (&file_manifest)[i].name; ++i)
		{
			const char *p = (&file_manifest)[i].name, *q = filename;
			bool match = true;
			do {
				match = match && *p == *q;
			} while (*p++ && *q++);

			if (match) {
				printf("Found \"%s\" @%p\n", (&file_manifest)[i].name, (&file_manifest)[i].resource);
				fsize_ = (&file_manifest)[i].size;
				flash_resource_ = (&file_manifest)[i].resource;
				break;
			}
		}
		if (!(&file_manifest)[i].name) {
			panic("Couldn't find file %s\n", filename);
		}
	}

	virtual void rewind() { get_ptr_ = 0; }
	virtual std::size_t size() const { return fsize_; };
	virtual void read(char *buffer, std::size_t amount)
	{
		if (fsize_ - get_ptr_ < amount)
			amount = fsize_ - get_ptr_;
		memcpy(buffer, flash_resource_, amount);
		get_ptr_ += amount;
	}
	virtual bool fail() const { return flash_resource_ == NULL; }
	// Get pointer to raw flash resource, for zero-copy read-only use
	virtual const char* getraw() const {return flash_resource_;}

private:
	std::size_t fsize_;
	size_t get_ptr_;
	const char *flash_resource_;
};

}

#endif
