/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPage.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file ErrorPage.cpp
 * @brief Implements config::ErrorPage, associating a set of HTTP status codes with a custom error page file path (nginx-style error_page directive).
 */

#include <webserv/config/ErrorPage.hpp>

namespace webserv
{
namespace config
{

/**
 * @brief Builds a human-readable, comma-separated list of the numeric values of the given status codes, prefixed with an opening bracket, used for logging.
 *
 * @param codes The collection of status codes whose numeric codes are formatted.
 */
static std::string	formatStatusCodesInfo(const t_StatusCodes &codes);


/**
 * @brief Default-constructs an ErrorPage with no status codes and an empty path, and initializes its logger.
 */
ErrorPage::ErrorPage() : _codes(), _path()
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.config.ErrorPage");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "ErrorPage instance created with default constructor");
}

/**
 * @brief Constructs an ErrorPage mapping the given status codes to the given error page path, and initializes its logger.
 *
 * @param codes The HTTP status codes served by this custom error page.
 * @param path The filesystem path of the error page file to serve for those codes.
 */
ErrorPage::ErrorPage(const t_StatusCodes &codes, const std::string &path) :	_codes(codes), _path(path)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.config.ErrorPage");
	_logger->setLevel(log42::logRecord::DEBUG);
	INFO(_logger, "ErrorPage instance created with status codes " + formatStatusCodesInfo(codes) + " and path: " + path);
}

/**
 * @brief Destroys the ErrorPage instance.
 */
 ErrorPage::~ErrorPage() {}
 

/**
 * @brief Copy-constructs an ErrorPage by copying the logger, status codes, and path from another instance.
 *
 * @param rhs The ErrorPage instance to copy from.
 */
ErrorPage::ErrorPage(const ErrorPage &rhs) : _logger(rhs._logger), _codes(rhs._codes), _path(rhs._path)
{}

/**
 * @brief Returns the shared logger for the config::ErrorPage class, retrieved from the logging manager.
 *
 * @return The logger associated with the "webserv.config.ErrorPage" channel.
 */
t_Logger	ErrorPage::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.config.ErrorPage");
}

/**
 * @brief Returns the HTTP status codes associated with this custom error page.
 *
 * @return A const reference to the collection of status codes served by this error page.
 */
const t_StatusCodes	&ErrorPage::getCodes() const
{
	return _codes;
}

/**
 * @brief Returns the filesystem path of the custom error page file.
 *
 * @return A const reference to the error page file path.
 */
const std::string	&ErrorPage::getPath() const
{
	return _path;
}

/**
 * @brief Copy-assigns this ErrorPage from another, copying the logger, status codes, and path while guarding against self-assignment.
 *
 * @param rhs The ErrorPage instance to copy from.
 * @return A reference to this ErrorPage after assignment.
 */
ErrorPage &ErrorPage::operator=(const ErrorPage &rhs)
{
	if (this != &rhs)
	{
		_logger = rhs._logger;
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
