#pragma once

#include "utilities/logger.h"
#include <string>
#include <unordered_map>

template <class T>
void* Constructor() {
	return (void*) new T();
}

class ObjectFromStringFactory {
public:
	ObjectFromStringFactory() = default;
	typedef void* (*ConstructorT)();
	typedef std::unordered_map<std::string, ConstructorT> MapType;
	MapType classes;

	/// <summary>
	/// Add a class type to the list of available classes, and provide a string with the class name
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="n"></param>
	template <class T>
	void RegisterType(std::string const& n);

	/// <summary>
	/// Takes in a string and return a pointer to a new instance of the class with that name.
	/// </summary>
	/// <param name="n"></param>
	/// <returns></returns>
	void* Construct(std::string const& n);
};

template <class T>
inline void ObjectFromStringFactory::RegisterType(std::string const& n) {
	classes.insert(std::make_pair(n, &Constructor<T>));
}
