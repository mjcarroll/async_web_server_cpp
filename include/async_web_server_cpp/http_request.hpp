#ifndef CPP_WEB_SERVER_HTTP_REQUEST_HPP
#define CPP_WEB_SERVER_HTTP_REQUEST_HPP

#include "async_web_server_cpp/http_header.hpp"

#include <map>
#include <string>
#include <vector>

namespace async_web_server_cpp
{

/**
 * Represents a request from a browser
 */
struct HttpRequest
{
  std::string method;
  std::string uri;
  int http_version_major;
  int http_version_minor;
  std::vector<HttpHeader> headers;

  std::string path;
  std::string query;
  std::map<std::string, std::string> query_params;

  bool has_header(const std::string &name) const;

  std::string get_header_value_or_default(const std::string &name,
                                          const std::string &default_value) const;


  bool has_query_param(const std::string &name) const;

  std::string get_query_param_value_or_default(const std::string &name,
      const std::string &default_value) const;

  bool parse_uri();
};

}

#endif
