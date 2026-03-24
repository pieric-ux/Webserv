// TODO: don't forget header
#ifndef WEBSERV_TYPES_TYPESREGISTRY_HPP
#define WEBSERV_TYPES_TYPESREGISTRY_HPP

/**
 * @file TypesRegistry.hpp
 * @brief [TODO:description]
 */

#include <map>
#include <string>

namespace webserv
{
namespace types
{

class TypesRegistry
{
	public:
		static TypesRegistry				&getInstance();
		std::string							getMimeType(const std::string &extension);
		void								loadFromFile(const std::string &filePath);
		void								addMimeType(const std::string &extension, const std::string &mimeType);

	private:
		std::map<std::string, std::string>	_types;

		TypesRegistry();
		~TypesRegistry();
		TypesRegistry(const TypesRegistry &rhs);
		TypesRegistry &operator=(const TypesRegistry &rhs);
};

} // !types
} // !webserv

#endif // !WEBSERV_TYPES_TYPESREGISTRY_HPP
