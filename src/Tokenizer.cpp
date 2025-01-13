#include "Tokenizer.hpp"

/* ------------- Constructors --------------- */

Tokenizer::Tokenizer(void) {}

Tokenizer::Tokenizer(const char *buffer) : std::stringstream(buffer)
{
	this->row = 0;
	this->col = 0;
}

Tokenizer	&Tokenizer::operator=(Tokenizer &other)
{
	if (&other != this)
	{
		std::stringstream::str(other.str());
	}
	return *this;
}

Tokenizer::~Tokenizer(void) {}

/* ------------- Methods --------------- */

void	Tokenizer::setBuffer(const char *buffer)
{
	this->str(buffer);
}

bool	Tokenizer::isCRLF(void)
{
	if (this->peek() == '\r')
	{
		this->get();
		if (this->peek() == '\n')
		{
			this->get();
			return true;
		}
	}
	return false;
}

bool	Tokenizer::end(void)
{
	return this->eof() || this->peek() == -1;
}

// TODO: trim make them as one function by passing delm

void	Tokenizer::trim(void)
{
	while (!this->end() && isspace(this->peek()))
	{
		this->get();
	}
}

void	Tokenizer::trimSpace(void)
{
	while (!this->end() && this->peek() == ' ')
	{
		this->get();
	}
}

//consume characters until a newline -- newline is consumed too
void	Tokenizer::consume(void)
{
	while (!this->end() && this->peek() != '\n')
	{
		this->get();
	}
	if (this->peek() == '\n')
	{
		this->get();
	}
}

//validate that the next character in the input stream matches an expected value
void	Tokenizer::expected(int c, std::string const &chars)
{
	(void) chars;
	if (this->peek() != c)
	{
		this->trim();
		//std::cerr << "error: unexpected token: " << this->next(chars) << std::endl;
		std::cerr << "error: expected token: `" << static_cast<char>(c) << "`" << std::endl;
		exit(1);
	}
	this->get();
}

//extract the next token from the input stream, stopping when it encounters any character from a specified set (chars)
std::string	Tokenizer::next(std::string const &chars)
{
	std::string token;

	while (!this->end())
	{
		if (chars.find(this->peek()) != std::string::npos)
			break;
		token += this->get();
	}
	return token;
}
