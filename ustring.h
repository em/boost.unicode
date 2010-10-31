#pragma once

#include "unicode.h"

namespace unicode
{
	template<class tpl_enc, class tpl_dec>
	class basic_ustring
	{
	public:
		class iterator
		{
		public:
			iterator()
			{
			}

			iterator(utf8_unit *p)
			{
				ptr = p;
			}

			utf32_unit operator*()
			{
				int tmp = table_extra_bytes[*ptr];

				return utf8_to_utf32(ptr);
			}

			bool operator==(iterator op)
			{
				return (ptr == op.ptr);
			}

			bool operator!=(iterator op)
			{
				return !(ptr == op.ptr);
			}

			iterator& operator--()
			{
				ptr -= table_extra_bytes[sizeof(table_extra_bytes)-*ptr]+1;;

				return (*this);
			}

			iterator operator++()
			{
				ptr += table_extra_bytes[*ptr]+1;

				return (*this);
			}

			iterator operator++(int)
			{
				iterator tmp = *this;
				++*this;
				return tmp;
			}

			iterator operator+(int op)
			{
				iterator tmp = *this;

				while(op--)
					tmp++;

				return tmp;
			}

		private:
			utf8_unit* ptr;
		};

		ustring()
		{
			length = 0;
		}

		template<class tpl>
		ustring(tpl& t)
		{
			length = 0;
			append(t);
		}

		ustring& operator +=(utf32_unit op)
		{
			append(op);
			return (*this);
		}

		int compare(const ustring& with) const
		{
			iterator i=begin();
			iterator j=with.begin();

			for(; i != end() && j != with.end(); ++i, ++j)
			{
				if(*i < *j)
					return -1; // Less than
				else if(*i > *j)
					return 1; // Greater than
			}

			return 0; // Equal
		}

		bool operator ==(const ustring& op) const
		{
			return compare(op) == 0;
		}

		bool operator <(const ustring& op) const
		{
			return compare(op) < 0;
		}

		bool operator >(const ustring& op) const
		{
			return compare(op) > 0;
		}

		template<class tpl>
		ustring& operator =(tpl t)
		{
			assign(t);
			return *this;
		}

		iterator begin() const
		{
			return iterator((utf8_unit*)&*data.begin());
		}

		iterator end() const
		{
			return iterator((utf8_unit*)&*data.end());
		}

		iterator at(int pos) const
		{
			assert(pos < length);

			iterator ret=begin();
			for(int i=0; i < pos; ++i)
			{
				++ret;
			}

			return ret;
		}

		const utf8_unit* utf8()
		{
			// We use UTF-8 internally so no conversion is needed.
			static std::vector<utf8_unit> buf;
			buf = data;
			buf.push_back(0); // Null terminator.
			return (utf8_unit*)&*buf.begin();
		}

		const utf16_unit* utf16()
		{
			static std::vector<utf16_unit> buf;
			buf.clear();

			utf16_unit tmp;

			for(iterator i=begin(); i != end(); ++i)
			{
				utf32_unit ch = *i;

				if(ch <= 0xFFFF)
				{
//					tmp = (utf16_unit)ch;
//					buf.append((char*)&ch, 2);
					buf.push_back((utf16_unit)ch);
				}
				else
				{
					ch -= 0x0010000UL;

					tmp = (utf16_unit)((ch >> 10) + (utf32_unit)0xD800);
					buf.push_back(tmp);
					tmp = (utf16_unit)((ch & 0x3FFUL) + (utf32_unit)0xDC00);
					buf.push_back(tmp);
				}
	
			}

			buf.push_back(0); // Null terminate.
			return (utf16_unit*)&*buf.begin();
		}

		const utf32_unit* utf32()
		{
			static std::vector<utf32_unit> buf;
			buf.clear();

			for(iterator i=begin(); i != end(); ++i)
			{
				buf.push_back(*i);
			}

			buf.push_back(0); // Null terminate.
			return (utf32_unit*)&*buf.begin();
		}

		const char* ascii(char replacement='?')
		{
			static std::vector<utf8_unit> buf;
			buf.clear();

			for(iterator i=begin(); i != end(); ++i)
			{
				if(*i > 126)
				{
					buf.push_back(replacement);
				}
				else
				{
					buf.push_back((utf8_unit)*i);
				}
			}

			buf.push_back(0); // Null terminator.
			return (char*)&*buf.begin();
		}

		void append(utf32_unit c)
		{
			// Encoded in UTF-8 internally.
			if(c < 0x80)
			{
//				data += (char)c;
				data.push_back((char)c);
			}
			else if(c < 0x800)
			{
//				data += (char)(0xC0 | c>>6);
//				data += (char)(0x80 | c & 0x3F);
				data.push_back((char)(0xC0 | c>>6));
				data.push_back((char)(0x80 | c & 0x3F));
			}
			else if(c < 0x10000)
			{
//				data += (char)(0xE0 | c>>12);
//				data += (char)(0x80 | c>>6 & 0x3F);
//				data += (char)(0x80 | c & 0x3F);
				data.push_back((char)(0xE0 | c>>12));
				data.push_back((char)(0x80 | c>>6 & 0x3F));
				data.push_back((char)(0x80 | c & 0x3F));
			}
			else if(c < 0x200000)
			{
//				data += (char)(0xF0 | c>>18);
//				data += (char)(0x80 | c>>12 & 0x3F);
//				data += (char)(0x80 | c>>6 & 0x3F);
//				data += (char)(0x80 | c & 0x3F);
				data.push_back((char)(0xF0 | c>>18));
				data.push_back((char)(0x80 | c>>12 & 0x3F));
				data.push_back((char)(0x80 | c>>6 & 0x3F));
				data.push_back((char)(0x80 | c & 0x3F));
			}

			++length;
		}

		void append(wchar_t c)
		{
			append((utf32_unit)c);
		}

		void append(char c)
		{
			append((utf32_unit)c);
		}

		void append(const char* cstr)
		{
			while(*cstr)
				append(*cstr++);
		}

		void append(const wchar_t* cstr)
		{
			while(*cstr)
				append(*cstr++);
		}

		void append(const std::string& str)
		{
			for(std::string::const_iterator i=str.begin(); i != str.end(); ++i)
				append(*i);
		}

		void append(const std::wstring& str)
		{
			for(std::wstring::const_iterator i=str.begin(); i != str.end(); ++i)
				append(*i);
		}

		void append(const ustring& str)
		{
			for(unicode::ustring::iterator i=str.begin(); i != str.end(); ++i)
				append(*i);
		}

		template<class tpl>
		void assign(tpl& t)
		{
			clear();
			append(t);
		}

		int size() const
		{
			return length;
		}

		bool empty() const
		{
			return !length;
		}

		void clear()
		{
			length = 0;
			data.clear();
		}

		friend std::ostream& operator<<(std::ostream& os, const ustring s)
		{
			for(iterator i=s.begin(); i != s.end();	++i)
			{
				os << (char)(*i);
			}

			return os;
		}

		friend std::wostream& operator<<(std::wostream& os, ustring& s)
		{
			for(iterator i=s.begin(); i != s.end(); ++i)
			{
				os << (wchar_t)(*i);
			}

			return os;
		}

	private:
		std::vector<utf8_unit> data; // Raw data buffer. We use UTF-8 for internal encoding.
		int length; // Number of characters in string.
	};
}