#include "MyString.h"
#include <sstream>

using namespace std;
MyString::MyString(void)
{
}


MyString::MyString(int i){
	stringstream ss;
	ss << i;
	*this = ss.str();
}
MyString::MyString(float f){
	stringstream ss;
	ss << f;
	*this = ss.str();
}
MyString::MyString(double d){
	stringstream ss;
	ss << d;
	*this = ss.str();
}
MyString::MyString(const std::string& str)
:string(str){
}
MyString::MyString (const std::string& str, size_t pos, size_t len)
:string(str, pos, len){
}
MyString::MyString (const char* s)
:string(s){
}
MyString::MyString (const char* s, size_t n)
:string(s, n){
}
MyString::MyString (size_t n, char c)
:string(n, c){
}
template <class InputIterator>
MyString::MyString  (InputIterator first, InputIterator last)
:string(first, last){
}

MyString::~MyString(void)
{
}

int MyString::ToInt() const{
	return atoi(this->c_str());
}

double MyString::ToDouble() const{
	return atof(this->c_str());
}
