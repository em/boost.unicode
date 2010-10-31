// (c) Copyright Emery De Nuccio 2007
// Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//#include "../../api/agui.h"
#include <iostream>
#include <fstream>
//#include "../../unicode/ustring.h"
#include <locale>
#include <vector>
#include "ustream.hpp"
#include <sstream>

using namespace std;

int main()
{

//	std::basic_ostream<
	//std::fstream f;
	//f.open(
	unicode::utf8_uifstream in("test_utf8_in.txt");
	unicode::utf8_ufstream out("test_utf8_out.txt");

//	unicode::utf16be_uifstream in("test_utf16big_in.txt");
//	unicode::utf8_uofstream out("test_utf16big_out.txt");

	std::stringstream test;

	unicode::utf16le_uostream poop(&test);


	for(;;)
	{
		boost::int_fast32_t ch = in.get();

		if(ch == EOF)
			break;

		std::cout << "U+" << std::hex << ch << std::endl;
	poop.put(ch);

	//	out.put(ch);
	}

	poop.put(0);

//	MessageBoxW(0, (wchar_t*)test.str().c_str(), 0, 0);


//	test.unget();
//	test.unget();
	//std::cout << "U+" << std::hex << test.get() << std::endl;
	

//	utf8_uistream();

/*
	unicode::uifstream<unicode::format_utf8> is("test.txt");
	std::cout << std::hex << is.get() << std::endl;
	std::cout << std::hex << is.get() << std::endl;
	std::cout << std::hex << is.get() << std::endl;
*/

//	while(int c = is.get())
//	std::cout << std::hex << c << std::endl;

	//unicode::ustringstream<unicode::utf8_encoding is>

	//	unicode::ustring<>
//	is.guess_encoding(


	//unicode::ustring bla("fuck you, dude!", unicode::utf8_encoding);


	//MessageBoxW(0, bla.utf16(), 0, 0);

//	agui::app app_main( unicode::uifstream("gui/poop.xml") );

//	app_main.attributes.size.subscribe()

//	app_main.attributes.subscribe();

//	app_main.attributes["st"].subscribe(subs_app_state);

	//app_main.attribute["state"] = "selected";

//	app_main.attributes["title"] = "selected";

//	widget::receive_events(test_handler, widget_button::event_press, "test_button");
//	widget::receive_events(test_handler);
//	widget::receive_events(


	//return run_xml(xml::file("gui/poop.xml"));

	return 0;
}