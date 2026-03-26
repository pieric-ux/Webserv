#include <webserv/HTTPServer.hpp>

namespace webserv
{
	
int main()
{
	HTTPServer &HTTPserver = HTTPServer::getInstance();
	(void)HTTPserver;
	return 0;
}

} //!webserv