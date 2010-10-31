// (c) Copyright Emery De Nuccio 2007
// Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_UNICODE_USTRING_HPP
#define BOOST_UNICODE_USTRING_HPP

#include <sstream>
#include "ustream.hpp"

namespace unicode
{
	class ascii_str_ptr
	{
	public:
		ascii_str_ptr(const char* _ptr) : ptr(_ptr)
		{
		}

		operator const char*() const
		{
			return ptr;
		}

	private:
		const char* ptr;
	};

	class basic_ustring
	{
	public:
		void append()
		{

		}
	};

	template<class codec_type>
	class ustring
	{
	protected:
		codec_type cod;
		std::stringstream data;

	public:
		ustring() : cod(&data)
		{
		}

		template<class encoder_type>
		char* c_str()
		{
			static std::stringstream buf;

			return ssbuf.str().c_str();
		}

		boost::int_fast32_t at(int i)
		{
			cod->seekg(i, beg);
		}

		void append(boost::int_fast32_t ch)
		{
			cod->encode(ch);
		}

		void append(std::string& s)
		{
			for(std::string::iterator i=s.begin(); i != s.end(); ++i)
				append(*i);
		}

/*
		friend void uistream::get(ustring& s)
		{

		}
		*/
	};
}

#endif