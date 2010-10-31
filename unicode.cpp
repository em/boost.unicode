#include "unicode.h"

namespace unicode
{
	int character_decode(const utf8_unit* source)
	{
		int ret = 0;

		int byte_qty = table_extra_bytes[*source];

		switch(byte_qty)
		{
			case 5: ret += *source++; ret <<= 6;
			case 4: ret += *source++; ret <<= 6;
			case 3: ret += *source++; ret <<= 6;
			case 2: ret += *source++; ret <<= 6;
			case 1: ret += *source++; ret <<= 6;
			case 0: ret += *source++;
		}

		ret -= table_offset[byte_qty];

		if(ret >= (utf32_unit)0xD800 && ret <= (utf32_unit)0xDFFF)
		{
			ret = (utf32_unit)0x0000FFFD;
		}

		return ret;
	}

}