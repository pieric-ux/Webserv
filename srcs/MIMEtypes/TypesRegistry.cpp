// TODO: don't forget header

/**
 * @file TypesRegistry.cpp
 * @brief [TODO:description]
 */

#include <webserv/MIMEtypes/TypesRegistry.hpp>

namespace webserv
{
namespace types
{

/**
 * @brief [TODO:description]
 */
TypesRegistry::TypesRegistry() : _types()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.mimetypes.types");
	_logger->setLevel(log42::logRecord::INFO);
	INFO(_logger, "TypesRegistry instance created");
}

/**
 * @brief [TODO:description]
 */
TypesRegistry::~TypesRegistry() {}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
TypesRegistry &TypesRegistry::getInstance()
{
	static TypesRegistry instance;
	return instance;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	TypesRegistry::getLogger() const
{
	return _logger;
}

/**
 * @brief [TODO:description]
 *
 * @param extension [TODO:parameter]
 * @return [TODO:return]
 */
std::string TypesRegistry::getMimeType(const std::string &extension)
{
	(void)extension;
	return "";
}

/**
 * @brief [TODO:description]
 *
 * @param extension [TODO:parameter]
 * @param mimeType [TODO:parameter]
 */
void TypesRegistry::addMimeType(const std::string &extension, const std::string &mimeType)
{
	_types[extension] = mimeType;
}

/**
 * @brief [TODO:description]
 *
 * @param filePath [TODO:parameter]
 */
void TypesRegistry::loadFromFile(const std::string &filePath)
{
	(void)filePath;
}

} // !types
} // !webserv
