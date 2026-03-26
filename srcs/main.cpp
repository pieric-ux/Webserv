#include <webserv/HTTPServer.hpp>

int main()
{
	webserv::HTTPServer &HTTPserver = webserv::HTTPServer::getInstance();
	(void)HTTPserver;
	return 0;
}
