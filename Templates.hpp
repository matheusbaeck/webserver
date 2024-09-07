#ifndef TEMPLATES_HPP__
# define TEMPLATES_HPP__

# include <iostream>
# include <sstream>
# include <string>
# include <vector>
#include <cstdlib> // For atoi


// Template class to construct and use any callable object (functor)
template <typename Functor>
class FunctorConstructor
{
	private:
		Functor *func; // Callable object
	public:
		// Constructor to initialize the functor
		FunctorConstructor(Functor *f) : func(f) {}

		// Template method to handle different argument types
		template <typename Arg>
		void operator()(const Arg& arg) const { func(arg); }
};

#endif
