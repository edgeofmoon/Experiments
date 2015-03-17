#pragma once

#include <string>

class MyString :
	public std::string
{
public:
	MyString(void);
	MyString(int i);
	MyString(float f);
	MyString(double d);
	MyString (const std::string& str);
	MyString (const std::string& str, size_t pos, size_t len = npos);
	MyString (const char* s);
	MyString (const char* s, size_t n);
	MyString (size_t n, char c);
	template <class InputIterator>
	  MyString  (InputIterator first, InputIterator last);
	~MyString(void);

	int ToInt() const;
	double ToDouble() const;
};

