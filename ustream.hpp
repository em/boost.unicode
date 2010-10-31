// (c) Copyright Emery De Nuccio 2007
// Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNICODE_USTREAM_HPP
#define BOOST_UNICODE_USTREAM_HPP

#include <iostream>
#include <fstream>
#include <list>
#include <sstream>
#include <boost/cstdint.hpp>
#include <boost/detail/endian.hpp>
#include "ustring.hpp"

namespace unicode
{
	//================================================================================
	// Encoding/Decoding
	//================================================================================

	using std::ios_base;

	class basic_encoder
	{
	protected:
		std::ostream* os;

	public:
		basic_encoder(std::ostream* _os) : os(_os) {}
		std::ostream* ostream()
		{
			return os;
		}

		virtual void encode(boost::int_fast32_t ch) = 0;
	};

	class basic_decoder
	{
	protected:
		std::istream* is;

	public:
		basic_decoder(std::istream* _is) : is(_is) {}

		std::istream* istream()
		{
			return is;
		}

		virtual boost::int_fast32_t decode() = 0;
		virtual bool prevg() = 0;
		virtual bool nextg() = 0;
		virtual bool seekg(int off, std::ios_base::seekdir dir=std::ios_base::beg)
		{
			switch(dir)
			{
			case std::ios_base::beg:
				{
					is->seekg(0, std::ios_base::beg);
				}
				break;

			case std::ios_base::end:
				{
					is->seekg(0, std::ios_base::end);
				}
				break;
			}

			if(off < 0)
			{
				while(off++)
				{
					prevg();
				}
			}
			else
			{
				while(off--)
				{
					nextg();
				}
			}

			return true;
		}

	};

	class basic_codec : public basic_decoder, public basic_encoder
	{
	private:
	public:
	};

	template<class decoder_type, class encoder_type>
	class specific_codec : public basic_codec
	{
	public:
		decoder_type d;
		encoder_type e;

		specific_codec(std::iostream* _ios) : decoder_type(_ios), encoder_type(_ios) {}
	};

/*
	class basic_codec : public basic_decoder, public basic_encoder
	{
	public:
		virtual boost::int_fast32_t decode() = 0;
		virtual bool prevg() = 0;
		virtual bool nextg() = 0;
		virtual void encode(boost::int_fast32_t ch) = 0;
	};

	template<class decoder_type, class encoder_type>
	class specific_codec : private basic_codec, virtual public decoder_type, virtual public encoder_type
	{
	public:
		specific_codec(std::iostream* _ios) : decoder_type(_ios), encoder_type(_ios) {}
	};

	typedef std::pair<basic_decoder, basic_encoder> codec;

*/
	//--------------------------------------------------------------------------------
	// Basic Multi-Byte-Unit Encoder/Decoder:
	// These are for dealing with endianness in encodings with a multi-byte unit (like UTF-16).
	//--------------------------------------------------------------------------------

	// Basic Multi-Byte-Unit Decoder
	template<class unit_type, int byte_order>
	class basic_mbu_decoder : public basic_decoder
	{
	public:
		basic_mbu_decoder(std::istream* _is) : basic_decoder(_is) {}

		unit_type get_unit()
		{
			unit_type tmp;

			for(int i=0; i < sizeof(unit_type); ++i)
			{
				if(byte_order == BOOST_BYTE_ORDER)
					((boost::uint8_t*)&tmp)[i] = is->get();
				else // If the source is a different endian than the system,
					((boost::uint8_t*)&tmp)[sizeof(unit_type)-i-1] = is->get(); // fill tmp backwards.
			}

			return tmp;
		}
	};

	// Basic Multi-Byte-Unit Encoder
	template<class unit_type, int byte_order>
	class basic_mbu_encoder : public basic_encoder
	{
	public:
		basic_mbu_encoder(std::ostream* _os) : basic_encoder(_os) {}

		void put_unit(boost::int_fast32_t ch)
		{
			for(int i=0; i < sizeof(unit_type); ++i)
			{
				if(byte_order == BOOST_BYTE_ORDER)
					os->put(((boost::uint8_t*)&ch)[i]);
				else // If the source is a different endian than the system,
					os->put(((boost::uint8_t*)&ch)[sizeof(unit_type)-i-1]); // read ch backwards.
			}
		}
	};

	//--------------------------------------------------------------------------------
	// UTF-32 Encoder/Decoder:
	//--------------------------------------------------------------------------------

	// UTF-32 Decoder
	template<int byte_order>
	class utf32_decoder : public basic_mbu_decoder<boost::uint32_t, byte_order>
	{
	public:
		utf32_decoder(std::istream* _is) : basic_mbu_decoder<boost::uint32_t, byte_order>(_is) {}

		// Read until one while character is decoded.
		boost::int_fast32_t decode()
		{
			int pk = is->peek();

			if(pk == EOF)
				return EOF;

			return get_unit();
		}

		// Skip a character in an input stream.
		bool nextg()
		{
			is->seekg(std::ios::cur, 4);
		}

		// Go back a character in an input stream.
		bool prevg()
		{
			is->seekg(std::ios::cur, -4);
		}

		bool seekg(int off, std::ios_base::seekdir dir=std::ios_base::beg)
		{
			is->seekg(4*off, dir);
		}
	};

	// UTF-32 Encoder
	template<int byte_order>
	class utf32_encoder : public basic_mbu_encoder<boost::uint32_t, byte_order>
	{
	public:
		utf32_encoder(std::ostream* _os) : basic_mbu_encoder<boost::uint32_t, byte_order>(_os) {}

		// Encode a character and write it to the stream.
		void encode(boost::int_fast32_t ch)
		{
			put_unit(ch);
		}
	};

	typedef utf32_decoder<1234> utf32le_decoder; // Little endian UTF-32 Decoder
	typedef utf32_encoder<1234> utf32le_encoder; // Little endian UTF-32 Encoder
	typedef utf32_decoder<4321> utf32be_decoder; // Big endian UTF-32 Decoder
	typedef utf32_encoder<4321> utf32be_encoder; // Big endian UTF-32 Encoder

	//--------------------------------------------------------------------------------
	// UTF-16 Encoder/Decoder
	//--------------------------------------------------------------------------------

	// UTF-16 Decoder
	template<int byte_order>
	class utf16_decoder : public basic_mbu_decoder<boost::uint16_t, byte_order>
	{
	public:
		utf16_decoder(std::istream* _is) : basic_mbu_decoder<boost::uint16_t, byte_order>(_is) {}

		// Read until one whole character is decoded.
		boost::int_fast32_t decode()
		{
			int pk = is->peek();

			if(pk == EOF)
				return EOF;

			boost::uint16_t ch = get_unit();

			if( istream().good() && (ch >= 0xD800 && ch <= 0xDBFF) ) // If ch is the first unit in a surrogate pair...
			{
				boost::uint16_t ch2 = get_unit(); // Get the following unit.

				if( istream().good() && (ch2 >= 0xDC00 && ch2 <= 0xDFFF) ) // If the following unit appropriately completes the surrogate pair...
				{
					ch = (boost::uint16_t)((ch - (boost::uint16_t)0xD800) << 10) + (ch2 - (boost::uint16_t)0xDC00) + 0x0010000; // Magic
				}
				else
				{
					throw "Unpaired surrogate.";
				}
			}

			return ch;
		}


		bool nextg() // Move get-pointer forward one full character.
		{
			return decode() == EOF;

			/*
			boost::uint16_t ch = get_unit();

			if( istream().good() && (ch >= 0xD800 && ch <= 0xDBFF) )
			{
				boost::uint16_t ch2 = get_unit();

				if( istream().good() && (ch2 < 0xDC00 || ch2 > 0xDFFF) )
				{
					throw "Unpaired surrogate.";
				}
			}*/
		}

		bool prevg() // Move get-pointer back one full character.
		{
			is->seekg(std::ios::cur, -2); // Previous unit.
			boost::uint16_t ch = get_unit();

			if(ch < 0xDC00 || ch > 0xDFFF) // If this is the second unit in a surrogate pair...
			{
				// Go back another unit to the first part of the surrogate pair.
				is->seekg(std::ios::cur, -2); 
				boost::uint16_t ch = get_unit();

				if(ch < 0xD800 || ch > 0xDBFF)
				{
				}
			}

			return true;
		}
	};

	// UTF-16 Encoder
	template<int byte_order>
	class utf16_encoder : public basic_mbu_encoder<boost::uint16_t, byte_order>
	{
	public:
		utf16_encoder(std::ostream* _os) : basic_mbu_encoder<boost::uint16_t, byte_order>(_os) {}

		// Encode a character and write it to the stream.
		void encode(boost::int_fast32_t ch)
		{
			if(ch <= 0xFFFF)
			{
				put_unit(ch);
			}
			else
			{
				ch -= 0x0010000UL;

				put_unit( (boost::uint16_t)(ch >> 10) + 0xD800 );
				put_unit( (boost::uint16_t)(ch & 0x3FFUL) + 0xDC00 );
			}
		}
	};

	typedef utf16_decoder<1234> utf16le_decoder; // Little endian UTF-16 Decoder
	typedef utf16_encoder<1234> utf16le_encoder; // Little endian UTF-16 Encoder
	typedef utf16_decoder<4321> utf16be_decoder; // Big endian UTF-16 Decoder
	typedef utf16_encoder<4321> utf16be_encoder; // Big endian UTF-16 Encoder



	//--------------------------------------------------------------------------------
	// UTF-8 Encoder/Decoder
	//--------------------------------------------------------------------------------

	// Lookup table with character sizes given the first byte of the character as an index.
	const boost::int_fast8_t utf8_character_sizes_lookup[256] = {
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
		3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,6,6
	};

	// A magic lookup table. *ooo* *ahh*
	// From http://www.unicode.org/Public/PROGRAMS/CVTUTF/ConvertUTF.c
	const boost::int_fast32_t utf8_magic_offsets_lookup[6] = {
		0x00000000UL, 0x00003080UL, 0x000E2080UL, 
		0x03C82080UL, 0xFA082080UL, 0x82082080UL
	};

	// UTF-8 Encoder
	class utf8_encoder : public basic_encoder
	{
	public:
		utf8_encoder(std::ostream* _os) : basic_encoder(_os) {}

		// Encode a character and write it to the stream.
		void encode(boost::int_fast32_t ch)
		{
			if(ch < 0x80)
			{
				os->put((boost::uint8_t)ch);
			}
			else if(ch < 0x800)
			{
				os->put((boost::uint8_t)(0xC0 | ch >> 6));
				os->put((boost::uint8_t)(0x80 | ch & 0x3F));
			}
			else if(ch < 0x10000)
			{
				os->put((boost::uint8_t)(0xE0 | ch >> 12));
				os->put((boost::uint8_t)(0x80 | ch >> 6 & 0x3F));
				os->put((boost::uint8_t)(0x80 | ch & 0x3F));
			}
			else if(ch < 0x200000)
			{
				os->put((boost::uint8_t)(0xF0 | ch >> 18));
				os->put((boost::uint8_t)(0x80 | ch >> 12 & 0x3F));
				os->put((boost::uint8_t)(0x80 | ch >> 6 & 0x3F));
				os->put((boost::uint8_t)(0x80 | ch & 0x3F));
			}
		}
	};

	// UTF-8 Decoder
	class utf8_decoder : public basic_decoder
	{
	public:
		utf8_decoder(std::istream* _is) : basic_decoder(_is) {}

		// Read until one character is decoded.
		boost::int_fast32_t decode()
		{
			int pk = is->peek();

			if(pk == EOF)
				return EOF;

			int ret = 0;
			int len = utf8_character_sizes_lookup[pk];

			switch(len)
			{
				case 6: ret += is->get(); ret <<= 6;
				case 5: ret += is->get(); ret <<= 6;
				case 4: ret += is->get(); ret <<= 6;
				case 3: ret += is->get(); ret <<= 6;
				case 2: ret += is->get(); ret <<= 6;
				case 1: ret += is->get();
			}

			ret -= utf8_magic_offsets_lookup[len-1];

			if(ret >=0xD800 && ret <= 0xDFFF)
			{
				ret = 0x0000FFFD;
			}

			return ret;
		}

		// Skip a character in an input stream.
		bool nextg()
		{
			int c = is->get();
			is->seekg(std::ios::cur, utf8_character_sizes_lookup[c]);

			return true;
		}

		// Go back a character in an input stream.
		bool prevg()
		{
			int tmp;

			do
			{
				is->unget();
				tmp = is->peek();
			}
			while(tmp > 0x7F && tmp < 0xC0); // All bytes in a character after the first byte are in this range.

			return true;
		}
	};


	// Codecs for each combination decoder and encoder.
	typedef specific_codec<utf8_decoder, utf8_encoder> utf8_codec;
	typedef specific_codec<utf16le_decoder, utf16le_encoder> utf16le_codec;
	typedef specific_codec<utf16be_decoder, utf16be_encoder> utf16be_codec;
	typedef specific_codec<utf32le_decoder, utf32le_encoder> utf32le_codec;
	typedef specific_codec<utf32be_decoder, utf32be_encoder> utf32be_codec;


	//================================================================================
	// Stream Stuff
	//================================================================================



	// Unicode input-only generic stream
	template<class decoder_type>
	class uistream
	{
	protected:
		decoder_type* dec;
		boost::uintmax_t gpos;
		int gcnt;

	public:
		uistream(decoder_type* _dec) : dec(_dec), gpos(0)
		{
		}

		decoder_type* decoder(decoder_type* nd)
		{
			decoder_type* tmp = dec;
			dec = nd;
			return tmp;
		}

		decoder_type* decoder()
		{
			return dec;
		}

		boost::int_fast32_t get()
		{
			boost::int_fast32_t ret = dec->decode();
			++gpos;
			gcnt = 1;
			return ret;
		}

		uistream& get(boost::int_fast32_t& c)
		{
			c = get();
			++gpos;
			gcnt = 1;
			return *this;
		}

		uistream& ignore(int qty=1, boost::int_fast32_t delim=EOF)
		{
			while(qty-- || get() == delim);
		}

		uistream& unget()
		{
			dec->prevg();
			--gpos;
			gcnt = 0;
			return *this;
		}

		boost::int_fast32_t peek()
		{
			boost::int_fast32_t ret = get();
			unget();
			return ret;
		}

		uistream& seekg(int off, std::ios_base::seekdir dir=std::ios_base::beg)
		{
			dec->seekg(off, dir);
			return *this;
		}

		boost::uintmax_t tellg()
		{
			return gpos;
		}

		int gcount()
		{
			return gcnt;
		}

		uistream& getline(ustring<decoder_type>& s, int qty, boost::int_fast32_t delim='\n')
		{
			while(qty--)
			{
				boost::int_fast32_t ch = get();

				if(ch == delim)
					break;

				s.append(ch);
			}
		}
	};

	template<class encoder_type>
	class uostream
	{
	protected:
		encoder_type* enc;
		boost::uintmax_t ppos;

	public:
		uostream(encoder_type* _enc) : enc(_enc), ppos(0)
		{
		}

		encoder_type* encoder(encoder_type* nd)
		{
			encoder_type* tmp = enc;
			enc = nd;
			return tmp;
		}

		encoder_type* encoder()
		{
			return enc;
		}

		uostream& put(boost::int_fast32_t ch)
		{
			enc->encode(ch);
			return *this;
		}

		boost::uintmax_t tellp()
		{
			return ppos;
		}
	};

	template<class codec_type>
	class ustream : public uistream<codec_type>, public uostream<codec_type>
	{
	protected:
		codec_type* cod;

	public:
		ustream(codec_type* _c) : cod(_c), uistream<codec_type>(_c), uostream<codec_type>(_c)
		{
		}

		codec_type* codec(codec_type* nc)
		{
			codec_type* tmp = cod;
			cod = nc;
			return tmp;
		}

		codec_type* codec()
		{
			return cod;
		}

		ustream& seekp(int off, std::ios_base::seekdir dir=std::ios_base::beg)
		{
			/*
			while(get() != EOF)
			{
				encoder()->ostream()->seekp(
					decoder()->istream()->gcount(),
					std::ios_base::cur);

				unget();
			}
			*/

			return *this;
		}
	};


	//================================================================================
	// Specific stream classes are wrappers that take a decoder and/or encoder as a
	// template argument to be created as an internal members of the class.
	//================================================================================

	// Specified-decoding Unicode Input Stream
	template<class decoder_type>
	class specific_uistream : public uistream<decoder_type>
	{
	protected:
		decoder_type sd;

	public:
		specific_uistream(std::istream* _is) : sd(_is), uistream<decoder_type>(&sd) {}
	};

	// Specific-encoding Unicode Output Stream
	template<class encoder_type>
	class specific_uostream : public uostream<encoder_type>
	{
	protected:
		encoder_type se;

	public:
		specific_uostream(std::ostream* _os) : se(_os), uostream<encoder_type>(&se) {}
	};

	// Specific-encoding(s) Unicode I/O Stream
	template<class codec_type>
	class specific_ustream : public ustream<codec_type>
	{
	protected:
		codec_type sc;

	public:
		specific_ustream(std::iostream* _ios) : sc(_ios), ustream<codec_type>(&sc) {}
	};

	// All of the specific Unicode streams:
	typedef specific_uistream<utf8_decoder> utf8_uistream; // UTF-8 Input Stream
	typedef specific_uostream<utf8_encoder> utf8_uostream; // UTF-8 Output Stream
	typedef specific_ustream<utf8_codec> utf8_ustream; // UTF-8 I/O Stream
	typedef specific_uistream<utf16le_decoder> utf16le_uistream; // UTF-16 Little-Endian Input Stream
	typedef specific_uostream<utf16le_encoder> utf16le_uostream; // UTF-16 Little-Endian Output Stream
	typedef specific_ustream<utf16le_codec> utf16le_ustream; // UTF-16 Little-Endian I/O Stream
	typedef specific_uistream<utf16be_decoder> utf16be_uistream; // UTF-16 Big-Endian Input Stream
	typedef specific_uostream<utf16be_encoder> utf16be_uostream; // UTF-16 Big-Endian Output Stream
	typedef specific_ustream<utf16be_codec> utf16be_ustream; // UTF-16 Big-Endian I/O Stream
	typedef specific_uistream<utf32le_decoder> utf32le_uistream; // UTF-32 Little-Endian Input Stream
	typedef specific_uostream<utf32le_encoder> utf32le_uostream; // UTF-32 Little-Endian Output Stream
	typedef specific_ustream<utf32le_codec> utf32le_ustream; // UTF-32 Little-Endian I/O Stream
	typedef specific_uistream<utf32be_decoder> utf32be_uistream; // UTF-32 Big-Endian Input Stream
	typedef specific_uostream<utf32be_encoder> utf32be_uostream; // UTF-32 Big-Endian Output Stream
	typedef specific_ustream<utf32be_codec> utf32be_ustream; // UTF-32 Big-Endian I/O Stream


	//================================================================================
	// This overlays file specific features to a Unicode stream class.
	//================================================================================

	template<class specific_ustream_type, class fstream_type>
	class specific_ufstream : public specific_ustream_type
	{
	private:
		fstream_type fs;

	public:
		specific_ufstream() : specific_ustream_type(&fs)
		{
		}

		specific_ufstream(const char* filename) : specific_ustream_type(&fs)
		{
			open(filename);
		}

		void open(const char* filename)
		{
			fs.open(filename, std::ios_base::binary);
		}

		void is_open()
		{
			return fs.is_open();
		}
	};

	// All of the specific Unicode *file* streams:
	typedef specific_ufstream<utf8_uistream, std::ifstream> utf8_uifstream; // UTF-8 Input Stream
	typedef specific_ufstream<utf8_uostream, std::ofstream> utf8_uofstream; // UTF-8 Output Stream
	typedef specific_ufstream<utf8_ustream, std::fstream> utf8_ufstream; // UTF-8 I/O Stream
	typedef specific_ufstream<utf16le_uistream, std::ifstream> utf16le_uifstream; // UTF-16 Little-Endian Input Stream
	typedef specific_ufstream<utf16le_uostream, std::ofstream> utf16le_uofstream; // UTF-16 Little-Endian Output Stream
	typedef specific_ufstream<utf16le_ustream, std::fstream> utf16le_ufstream; // UTF-16 Little-Endian I/O Stream
	typedef specific_ufstream<utf16be_uistream, std::ifstream> utf16be_uifstream; // UTF-16 Big-Endian Input Stream
	typedef specific_ufstream<utf16be_uostream, std::ofstream> utf16be_uofstream; // UTF-16 Big-Endian Output Stream
	typedef specific_ufstream<utf16be_ustream, std::fstream> utf16be_ufstream; // UTF-16 Big-Endian I/O Stream
	typedef specific_ufstream<utf32le_uistream, std::ifstream> utf32le_uifstream; // UTF-32 Little-Endian Input Stream
	typedef specific_ufstream<utf32le_uostream, std::ofstream> utf32le_uofstream; // UTF-32 Little-Endian Output Stream
	typedef specific_ufstream<utf32le_ustream, std::fstream> utf32le_ufstream; // UTF-32 Little-Endian I/O Stream
	typedef specific_ufstream<utf32be_uistream, std::ifstream> utf32be_uifstream; // UTF-32 Big-Endian Input Stream
	typedef specific_ufstream<utf32be_uostream, std::ofstream> utf32be_uofstream; // UTF-32 Big-Endian Output Stream
	typedef specific_ufstream<utf32be_ustream, std::fstream> utf32be_ufstream; // UTF-32 Big-Endian I/O Stream

/*
	template<class codec_type>
	class specific_ustringstream : public specific_ustream<codec_type>
	{
	private:
		std::stringstream ss;

	public:
		specific_ustringstream() : specific_ustream_type(&ss)
		{
		}
	};

	typedef specific_ustringstream<utf8_ustream> utf8_ustringstream; // UTF-8 I/O Stream
	typedef specific_ustringstream<utf16le_ustream> utf16le_ustringstream; // UTF-16 Little-Endian I/O Stream
	typedef specific_ustringstream<utf16be_ustream> utf16be_ustringstream; // UTF-16 Big-Endian I/O Stream
	typedef specific_ustringstream<utf32le_ustream> utf32le_ustringstream; // UTF-32 Little-Endian I/O Stream
	typedef specific_ustringstream<utf32be_ustream> utf32be_ustringstream; // UTF-32 Big-Endian I/O Stream
*/

/*

	// Basic Unicode File Stream
	// This overlays file stream specific features to the specific_ustream class.
	template<class specific_ustream_type>
	class specific_ustringstream : public specific_ustream_type
	{
	private:
		std::stringstream ss;

	public:
		specific_ustringstream() : specific_ustream_type(&ss)
		{
		}
	};

	typedef specific_ustringstream<utf8_ustream> utf8_uistringstream; // UTF-8 Input Stream
	typedef specific_ustringstream<utf8_uostream> utf8_uostringstream; // UTF-8 Output Stream
	typedef specific_ustringstream<utf16le_uistream, std::istringstream> utf16le_uistringstream; // UTF-16 Little-Endian Input Stream
	typedef specific_ustringstream<utf16le_uostream> utf16le_uostringstream; // UTF-16 Little-Endian Output Stream
	typedef specific_ustringstream<utf16be_uistream, std::istringstream> utf16be_uistringstream; // UTF-16 Big-Endian Input Stream
	typedef specific_ustringstream<utf16be_uostream> utf16be_uostringstream; // UTF-16 Big-Endian Output Stream
	typedef specific_ustringstream<utf32le_uistream, std::istringstream> utf32le_uistringstream; // UTF-32 Little-Endian Input Stream
	typedef specific_ustringstream<utf32le_uostream> utf32le_uostringstream; // UTF-32 Little-Endian Output Stream
	typedef specific_ustringstream<utf32be_uistream, std::istringstream> utf32be_uistringstream; // UTF-32 Big-Endian Input Stream
	typedef specific_ustringstream<utf32be_uostream> utf32be_uostringstream; // UTF-32 Big-Endian Output Stream
*/


	// Smart Streams:
	//--------------------------------------------------------------------------------
/*
	bool is_valid(boost::int_fast32_t ch)
	{
		if(	ch == 0xFFFE ||
			ch == 0xFFFF ||
			ch >= DD016 && ch <= FDEF
			)
			return false;

		return true;
	};


	template<class t_default_decoder>
	class smart_uifstream : public specific_ufstream<uistream, std::ifstream, t_default_decoder>
	{
	public:
		smart_uifstream() : known_flag(false)
		{
		}

		// If deep_analysis is true we will rule out encodings if reading them with the decoder yields invalid code points.
		// If no_bom is true the stream will start just after the BOM, if there is one, otherwise we will move the file pointer back to 0.
		smart_uifstream(char* filename, bool deep_analysis=false, bool no_bom=false) : known_flag(false), specific_ufstream<uistream, std::ifstream, t_default_decoder>(filename)
		{
			std::istream* s = decoder()->istream();

			basic_decoder* known_decs[] = {
				new utf8_decoder(s),
				new utf16le_decoder(s),
				new utf16be_decoder(s),
				new utf32le_decoder(s),
				new utf32be_decoder(s)
			};

			for(int i=0; i < 5; ++i)
			{
				known_decs[i]->decode();
				s.seekg(0);

				if(peek() == 0xFEFF)
				{
					known_flag = true;
					decoder(save);
				}

				delete known_decs[i];
			}

		}

		bool analyze()
		{
			//boost::int_fast32_t
		}

		bool known()
		{
			return known_flag;
		}

	private:
		bool known_flag;
	};
*/
}

#endif