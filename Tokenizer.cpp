#include "Tokenizer.hpp"

void	_toLower(char &c)
{
	c = tolower(c);
}

Tokenizer::Tokenizer(const char *buffer)
{
	this->ss << buffer;
}
Tokenizer::Tokenizer(std::stringstream &_ss)
{
	this->ss << _ss.rdbuf() << std::noskipws;
}

std::string	Tokenizer::toLower(std::string &str)
{
	std::for_each(str.begin(), str.end(), _toLower);	
	return str;
}

bool	Tokenizer::isNewline(void)
{
	if (this->ss.peek() == '\r')
	{
		this->ss.get();
		if (this->ss.peek() == '\n')
		{
			this->ss.get();
			return true;
		}
	}
	return false;
}

void	Tokenizer::trim(void)
{
	while (!this->ss.eof() && isspace(this->ss.peek()))
	{
		this->ss.get();
	}
}

bool Tokenizer::end(void) 
{
	return ss.eof();
}


std::string Tokenizer::next(char c)
{
	//char c;
	std::string token;
	this->trim();
	while (!this->end() && this->ss.peek() != -1)
	{
		if (this->isNewline())
			break;

		// TODO: pass it as argument
		//std::cout << "[" << (char)this->ss.peek() << "]" << std::endl;
		if (this->ss.peek() == c)
		{
			this->ss.get();
			break;
		}

		//this->ss >> c >> std::noskipws;
		token += this->ss.get();
		

		//token += this->ss.get();
	}
	return token;
}
