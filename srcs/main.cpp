// TODO: don't forget header

#include <abnf/Abnf.hpp>
#include <log42/Log42.hpp>
#include <webserv/HTTPServer.hpp>

static void	initLogging();
static void	initAbnf();

int main(int ac, char **av)
{
	signal(SIGPIPE, SIG_IGN);
	
	try {
		initLogging();
	} catch (const std::exception &e) {
		std::cerr << "Failed to initialize logging: " << e.what() << std::endl;
		return (EXIT_FAILURE);
	}

	try {
		initAbnf();
	} catch (const std::exception &e) {
		ROOT_CRITICAL(std::string("Failed to initialize abnf: ") + e.what());
		return (EXIT_FAILURE);
	}

	webserv::HTTPServer &HTTPserver = webserv::HTTPServer::getInstance();

	if (ac == 2)
	{
		if (common::core::utils::hasExtension(av[1], ".conf"))
		{
			HTTPserver.setConfigPath(av[1]);
			ROOT_INFO("Configuration file set to: " + std::string(av[1]));
		}
		else
		{
			ROOT_CRITICAL("Invalid configuration file: " + std::string(av[1]) + " must have .conf extension");
			return (EXIT_FAILURE);
		}
	}

	try {
		HTTPserver.setup();
	} catch (const std::exception &e)
	{
		ROOT_CRITICAL(std::string("Failed to setup HTTP server: ") + e.what());
		return (EXIT_FAILURE);
	}

	HTTPserver.run();

	return (EXIT_SUCCESS);
}

/**
 * @brief [TODO:description]
 */
static void	initLogging()
{
	log42::formatter::Formatter fmt(LOG_FMT, LOG_DATE_FMT);

	webserv::t_Logger rootLogger = log42::manager::Manager::getInstance().getRoot();
	if (DEV_LOG)
		rootLogger->setLevel(log42::logRecord::DEBUG);
	else
		rootLogger->setLevel(log42::logRecord::INFO);
	common::core::raii::SharedPtr<log42::handler::StreamHandler> consoleHandler =
		MAKE_SHARED(log42::handler::StreamHandler);
	consoleHandler->setFormatter(fmt);
	consoleHandler->setLevel(log42::logRecord::WARNING);
	rootLogger->addHandler(
		common::core::raii::staticPointerCast<log42::handler::Handler>(consoleHandler));

	std::vector<std::string> names;
	names.push_back("root");
	names.push_back("webserv");
	names.push_back("webserv.server");
	names.push_back("webserv.serverfactory");
	names.push_back("webserv.client.client");
	names.push_back("webserv.client.request");
	names.push_back("webserv.client.response");
	names.push_back("webserv.client.HTTPError");
	names.push_back("webserv.config.httpconfig");
	names.push_back("webserv.config.serverconfig");
	names.push_back("webserv.config.locationconfig");
	names.push_back("webserv.config.ErrorPage");
	names.push_back("webserv.handler.clienthandler");
	names.push_back("webserv.handler.requesthandler");
	names.push_back("webserv.handler.executionhandler");
	names.push_back("webserv.handler.responsehandler");
	names.push_back("webserv.headers.httpheader");
	names.push_back("webserv.headers.headersregistry");
	names.push_back("webserv.mimetypes.types");
	names.push_back("webserv.parser.parser");
	names.push_back("webserv.status.statuscode");
	names.push_back("webserv.status.statuscoderegistery");

	std::vector<std::string>::const_iterator it = names.begin();
	for (; it != names.end(); ++it)
	{
		webserv::t_Logger logger = log42::manager::Manager::getInstance().getLogger(*it);
		std::string file = std::string(WEBSERV_LOG_DIR) + "/" + *it + ".log";
		common::core::raii::SharedPtr<log42::handler::FileHandler> fileHandler =
			MAKE_SHARED(log42::handler::FileHandler, file);
		fileHandler->setFormatter(fmt);
		if (DEV_LOG)
			fileHandler->setLevel(log42::logRecord::DEBUG);
		else
			fileHandler->setLevel(log42::logRecord::WARNING);
		logger->addHandler(
			common::core::raii::staticPointerCast<log42::handler::Handler>(fileHandler));
	}
	ROOT_INFO("Logging initialized successfully");
}

/**
 * @brief [TODO:description]
 */
static void	initAbnf()
{
	abnf::AbnfLogConfig abnfConfig;
	abnfConfig.dir = WEBSERV_LOG_DIR;
	abnfConfig.formatter = log42::formatter::Formatter(LOG_FMT, LOG_DATE_FMT);
	if (DEV_LOG)
		abnfConfig.level = log42::logRecord::INFO;
	else
		abnfConfig.level = log42::logRecord::WARNING;
	abnf::Abnf::getInstance().initFromDirectory(ABNF_DIR, ABNF_EXTENSION, abnfConfig);
	ROOT_INFO("Abnf initialized successfully");
}
