#ifndef CPP_WEB_SERVER_HTTP_REQUEST_HANDLER_HPP
#define CPP_WEB_SERVER_HTTP_REQUEST_HANDLER_HPP

#include "async_web_server_cpp/http_request.hpp"

#include <functional>
#include <memory>

namespace async_web_server_cpp
{

class HttpConnection;

/**
 * A handler for requests
 * Should return true if the request was successfuly handled
 * Returning false will cause the next matching handler to be triggered
 * If false is returned then nothing should be written to the connection
 */
using HttpServerRequestHandler = std::function<bool(const HttpRequest &, std::shared_ptr<HttpConnection>, const char* begin, const char* end)>;

/**
 * A hander that can dispatch to a request to different handlers depending on a
 * predicate. If none of registered handlers satisfy the request then the
 * default request handler is used.
 */
class HttpRequestHandlerGroup
{
public:
  using HandlerPredicate = std::function<bool(const HttpRequest &)>;

  HttpRequestHandlerGroup(HttpServerRequestHandler default_handler);

  void addHandlerForPath(const std::string &path_regex, HttpServerRequestHandler handler);

  void addHandler(HandlerPredicate predicate, HttpServerRequestHandler handler);

  bool operator()(const HttpRequest &request, std::shared_ptr<HttpConnection> connection, const char* begin, const char* end);

private:
  HttpServerRequestHandler default_handler_;
  std::vector<std::pair<HandlerPredicate, HttpServerRequestHandler> > handlers_;
};

class HttpRequestBodyCollector
{
public:
  using Handler = std::function<void(const HttpRequest &, std::shared_ptr<HttpConnection>, const std::string& body)>;

  HttpRequestBodyCollector(Handler handler);

  bool operator()(const HttpRequest &request, std::shared_ptr<HttpConnection> connection, const char* begin, const char* end);

private:
  Handler handler_;
};

}

#endif
