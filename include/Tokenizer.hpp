#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <sstream>
#include <algorithm>
#include <iostream>

class Tokenizer : public std::stringstream
{
	int	row;
	int	col;
public:
	
	Tokenizer(const char *buffer);
	Tokenizer(void);
	~Tokenizer(void);
	Tokenizer &operator=(Tokenizer &other);

	void				setBuffer(const char *buffer);

	bool				isCRLF(void);
	bool				end(void);
	void				trim(void);
	void				trimSpace(void);
	void				consume(void);
	void				expected(int c, std::string const &chars);
	std::string	next(std::string const &chars);

};


#endif //TOKENIZER_HPP
