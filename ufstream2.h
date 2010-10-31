#pragma once

#include "unicode.h"
#include "ustring.h"

namespace unicode
{
	class uifstream
	{
	public:
		uifstream(const char* filename, int explicit_encoding=encoding_auto, int default_encoding=encoding_utf8)
		{
			open(filename, explicit_encoding, default_encoding);
		}

		void open(const char* filename, int explicit_encoding=encoding_auto, int default_encoding=encoding_utf8)
		{
			static const unsigned char table_bom_ids[] = {
				0xEF, 0xBB, 0xBF, // UTF-8
				0xFE, 0xFF, // UTF-16 Big Endian
				0xFF, 0xFE, // UTF-16 Little Endian
				0x00, 0x00, 0xFE, 0xFF, // UTF-32 Big Endian
				0xFF, 0xFE, 0x00, 0x00 // UTF-32 Little Endian
			};

			static const int table_bom_lengths[] = {
				3, 2, 2, 4, 4
			};

			// ...

			is.open(filename);
			unsigned char buf[4];
			is.read((char*)buf, sizeof(buf));
			int bom_matched = -1;

			for(int i=0, offset=0; i < (sizeof(table_bom_lengths)/sizeof(int)) && bom_matched == -1; ++i)
			{
				for(int j=0; j < table_bom_lengths[i]; ++j)
				{
					if(buf[j] == table_bom_ids[offset])
					{
						bom_matched = i;
						break;
					}

					++offset;
				}
			}

			if(bom_matched == -1) // If we matched a BOM.
				is.seekg(table_bom_lengths[bom_matched], std::ios::beg); // Seek just beyond it.
			else
				is.seekg(0, std::ios::beg); // Seek to beginning.

			if(explicit_encoding == -1) // Auto detect.
			{
				if(bom_matched == -1)
					encoding = default_encoding;
				else
					encoding = bom_matched;
			}
			else // Explicit encoding.
			{
				encoding = explicit_encoding;

				if(bom_matched != explicit_encoding) // If BOM found doesn't match explicit encoding.
				{
					// Then we must seek back to the beginning and disregard the match.
					is.seekg(0, std::ios::beg);
				}
			}

 		}

		void set_encoding(int new_encoding)
		{
			encoding = new_encoding;
		}

		int get_encoding()
		{
			return encoding;
		}

		const ustring& read_string()
		{
			buf.clear();

			while(utf32_unit c = get())
			{
				buf.append(c);
			}

			return buf;
		}

		const ustring& read(int qty)
		{
			buf.clear();

			for(;qty;--qty)
			{
				buf.append(get());
			}

			return buf;
		}

		utf32_unit get()
		{
			switch(encoding)
			{
			case encoding_utf8:
				{
					int first = is.get(); // Read in 1 byte
					if(first == EOF)
					{
						return 0;
					}

					int byte_qty = table_extra_bytes[first];
					utf8_unit *buf = new utf8_unit[byte_qty+1];
					buf[0] = first;
					is.read((char*)buf+1, byte_qty);

					return utf8_to_utf32(buf, byte_qty);
				}
				break;

			case encoding_utf16:
				{
					utf16_unit tmp_unit;
					for(int i=0; i < 2; ++i)
					{
						int tmp = is.get();
						if(tmp == EOF)
						{
							return 0;
						}

						((char*)&tmp_unit)[i] = tmp;
					}

					return tmp_unit;
				}
				break;

			case encoding_utf32:
				{
					utf32_unit tmp_unit;

					for(int i=0; i < 4; ++i)
					{
						int tmp = is.get();
						if(tmp == EOF)
						{
							return 0;
						}

						((char*)&tmp_unit)[i] = tmp;
					}

					return tmp_unit;
				}
				break;
			}

			return 0;
		}

	private:
		int encoding;
		std::ifstream is;
		static ustring buf;
	};

}