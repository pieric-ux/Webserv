/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pdemont <pdemont@student.42lausanne.ch>    +#+  +:+       +#+        */
/*   By: blucken <blucken@student.42lausanne.ch>  +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*   Created: 2026/01/21                              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HANDLER_REQUESTHANDLER_HPP
#define WEBSERV_HANDLER_REQUESTHANDLER_HPP

/**
 * @file RequestHandler.hpp
 * @brief Declares the RequestHandler, which buffers raw request bytes and drives
 * parsing, validation, and absolute-path resolution of a client HTTP request.
 */

#include <algorithm>
#include <sstream>
#include <vector>
#include <string>
#include <common/common.hpp>
#include <webserv/client/HTTPError.hpp>
#include <webserv/client/Request.hpp>
#include <webserv/config/ServerConfig.hpp>
#include <webserv/config/LocationConfig.hpp>
#include <webserv/parser/Parser.hpp>
#include <webserv/types.hpp>

namespace webserv
{
namespace handler
{

class RequestHandler
{
	public:
		RequestHandler(config::ServerConfig &serverConfig);
		~RequestHandler();

		RequestHandler(const RequestHandler &rhs);
		RequestHandler &operator=(const RequestHandler &rhs);

		static t_Logger			getLogger();

		void					parseHeaders();
		void					parseBody();

		client::Request			&getRequest();
		const client::Request	&getRequest() const;
		void					setRequest(const client::Request &request);
		const t_raw				&getBufferRequest() const;
		void					appendToBufferRequest(const t_raw &buffer);
		void					eraseBufferRequestFront(std::size_t n);
		void					clearBufferRequest();
		parser::Parser			&getParser();
		std::size_t				getChunkedAvailable() const;
		void					shrinkChunkedAvailable(std::size_t n);

	private:
		t_Logger				_logger;
		client::Request			_request;
		t_raw					_bufferRequest;
		parser::Parser			_parser;
		config::ServerConfig	&_serverConfig;
		std::size_t				_chunkedAvailable;
		std::size_t				_chunkedTotalDecoded;

		void					validateHeaders();
		void					buildAbsolutPath();
		std::string				normalizePath(const std::string &path);
		void					decodeChunkedBody();
		bool					readChunkSizeLine(const std::string &tail, std::size_t pos,
										unsigned long &chunkSize, std::size_t &lineEnd) const;
		bool					readChunkData(const std::string &tail, std::size_t dataStart,
										unsigned long chunkSize, std::string &data) const;
		bool					readTrailerSection(const std::string &tail, std::size_t pos,
										std::size_t &newPos) const;
};

} // !handler
} // !webserv

#endif // !WEBSERV_HANDLER_REQUESTHANDLER_HPP
