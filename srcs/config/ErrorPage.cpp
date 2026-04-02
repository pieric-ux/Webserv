// TODO: don't forget header

/**
 * @file ErrorPage.cpp
 * @brief [TODO:description]
 */

#include <webserv/config/ErrorPage.hpp>

namespace webserv
{
namespace config
{

/**
 * @brief [TODO:description]
 *
 * @param codes [TODO:parameter]
 */
static std::string	formatStatusCodesInfo(const t_StatusCodes &codes);

/**
 * @brief [TODO:description]
 */
ErrorPage::ErrorPage() : _codes(), _path()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.config.ErrorPage");
	_logger->setLevel(log42::logRecord::INFO);
	INFO(_logger, "ErrorPage instance created with default constructor");
}

/**
 * @brief [TODO:description]
 *
 * @param codes [TODO:parameter]
 * @param path [TODO:parameter]
 */
ErrorPage::ErrorPage(const t_StatusCodes &codes, const std::string &path)
	:	_codes(codes),
		_path(path)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.config.ErrorPage");
	_logger->setLevel(log42::logRecord::INFO);
	INFO(_logger, "ErrorPage instance created with status codes " + formatStatusCodesInfo(codes) + " and path: " + path);
}

/**
 * @brief [TODO:description]
 */
ErrorPage::~ErrorPage() {}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger	ErrorPage::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.config.ErrorPage");
}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
ErrorPage::ErrorPage(const ErrorPage &rhs)
	: _codes(rhs._codes),
	  _path(rhs._path)
{}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
 */
ErrorPage &ErrorPage::operator=(const ErrorPage &rhs)
{
	if (this != &rhs)
	{
		_codes = rhs._codes;
		_path = rhs._path;
	}
	return (*this);
}

static std::string	formatStatusCodesInfo(const t_StatusCodes &codes)
{
	std::ostringstream oss;

	oss << "[";
	t_StatusCodes::const_iterator it = codes.begin();
	for (; it != codes.end(); ++it)
	{
		if (it != codes.begin())
			oss << ", ";
		oss << it->getCode();
	}
	return oss.str();
}

} // !config
} // !webserv
