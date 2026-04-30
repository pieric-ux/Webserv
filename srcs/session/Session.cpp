// TODO: don't forget header

/**
 * @file Session.cpp
 * @brief [TODO:description]
 */

#include <webserv/session/Session.hpp>

namespace webserv
{
namespace session
{

/**
 * @brief [TODO:description]
 *
 * @param id [TODO:parameter]
 */
Session::Session(const t_uuid &id)
	:	_id(id),
		_data(),
		_expires(0),
		_ttl(0)
{
	_logger = log42::manager::Manager::getInstance().getLogger("webserv.session.session");
    _logger->setLevel(log42::logRecord::DEBUG);
    INFO(_logger, "Session instance created");
}

/**
 * @brief [TODO:description]
 */
Session::~Session() {}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 */
Session::Session(const Session &rhs)
	:	_logger(rhs._logger),
		_id(rhs._id),
		_data(rhs._data),
		_expires(rhs._expires),
		_ttl(rhs._ttl)
{}

/**
 * @brief [TODO:description]
 *
 * @param rhs [TODO:parameter]
 * @return [TODO:return]
 */
Session &Session::operator=(const Session &rhs)
{
	if (this != &rhs)
	{
		_logger = rhs._logger;
		_id = rhs._id;
		_data = rhs._data;
		_expires = rhs._expires;
		_ttl = rhs._ttl;
	}
	return (*this);
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
t_Logger Session::getLogger()
{
	return log42::manager::Manager::getInstance().getLogger("webserv.session.session");
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_uuid &Session::getId() const
{
	return _id;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
const t_SessionData &Session::getData() const
{
	return _data;
}

/**
 * @brief [TODO:description]
 *
 * @param data [TODO:parameter]
 */
void Session::setData(const t_SessionData &data)
{
	_data = data;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
std::time_t Session::getExpires() const
{
	return _expires;
}

/**
 * @brief [TODO:description]
 *
 * @param expires [TODO:parameter]
 */
void Session::setExpires(std::time_t expires)
{
	_expires = expires;
}

t_sessionTTL Session::getTtl() const
{
	return _ttl;
}

void Session::setTtl(t_sessionTTL ttl)
{
	_ttl = ttl;
}

/**
 * @brief [TODO:description]
 *
 * @return [TODO:return]
 */
bool Session::isExpired() const
{
	if (_expires == 0)
		return false;
	return std::time(NULL) > _expires;
}

/**
 * @brief [TODO:description]
 *
 * @param key [TODO:parameter]
 * @return [TODO:return]
 */
const std::string &Session::get(const std::string &key) const
{
	static const std::string empty;
	t_SessionData::const_iterator it = _data.find(key);
	if (it == _data.end())
		return empty;
	return it->second;
}

/**
 * @brief [TODO:description]
 *
 * @param key [TODO:parameter]
 * @param value [TODO:parameter]
 */
void Session::set(const std::string &key, const std::string &value)
{
	_data[key] = value;
}

} // !session
} // !webserv
