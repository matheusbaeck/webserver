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

/* template <typename Container>
void initializeContainer(Container& container, const std::string& data); */

template <typename Container>
void initializeContainer(Container& container, const std::string& data)
{
	std::istringstream stream(data);
	std::string line;

	while (std::getline(stream, line, '{'))
	{
		std::vector<int> innerContainer;
		std::string element;

		while (std::getline(stream, element, ','))
		{
			size_t endPos = element.find('}');
			if (endPos != std::string::npos)
			{
				element = element.substr(0, endPos);
				innerContainer.push_back(atoi(element.c_str()));
				break;
			}
			else
				innerContainer.push_back(atoi(element.c_str()));
		}

		if (!innerContainer.empty())
			container.push_back(innerContainer);
	}
}
// Explicitly instantiate templates if needed
// template void initializeContainer<std::vector<std::vector<int>>>(std::vector<std::vector<int>>&, const std::string&);

#endif
