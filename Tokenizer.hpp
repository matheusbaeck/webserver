#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <sstream>
#include <algorithm>

class Tokenizer
{
	std::stringstream ss;
public:
	Tokenizer(const char *buffer);
	Tokenizer(std::stringstream &_ss);

	std::string	toLower(std::string &str);
	void		trim(void);
	bool		end(void);
	bool		isNewline(void);
	std::string next(char c);
};

#endif //TOKENIZER_HPP