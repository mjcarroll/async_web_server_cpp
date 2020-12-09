#include "async_web_server_cpp/http_reply.hpp"

#include <asio/write.hpp>

using namespace std::placeholders;

namespace async_web_server_cpp
{

HttpConnection::HttpConnection(asio::io_service &io_service,
                               HttpServerRequestHandler handler)
  : strand_(io_service),
    socket_(io_service),
    request_handler_(handler),
    write_in_progress_(false)
{
}

asio::ip::tcp::socket &HttpConnection::socket()
{
  return socket_;
}

void HttpConnection::start()
{
  async_read(std::bind(&HttpConnection::handle_read, shared_from_this(), _1, _2));
}

void HttpConnection::handle_read(const char* begin, const char* end)
{
  std::optional<bool> result;
  const char* parse_end;
  std::tie(result, parse_end) = request_parser_.parse(request_, begin, end);

  if (result.has_value() && result)
  {
    request_.parse_uri();
    try
    {
      request_handler_(request_, shared_from_this(), parse_end, end);
    }
    catch (...)
    {
      // error constructing request
      // just kill the connection as the handler may have already started writing stuff out
    }
  }
  else if (!result)
  {
    HttpReply::stock_reply(HttpReply::bad_request)(request_, shared_from_this(), begin, end);
  }
  else
  {
    async_read(std::bind(&HttpConnection::handle_read, shared_from_this(), _1, _2));
  }
}

void HttpConnection::handle_read_raw(ReadHandler callback,
                                     const asio::error_code &e,
                                     std::size_t bytes_transferred)
{
  if (!e)
  {
    callback(buffer_.data(), buffer_.data() + bytes_transferred);
  }
  else
  {
    last_error_ = e;
  }
}
void HttpConnection::async_read(ReadHandler callback)
{
  if (last_error_)
  {
   throw asio::system_error(last_error_);
  }
  socket_.async_read_some(asio::buffer(buffer_),
                          strand_.wrap(std::bind(&HttpConnection::handle_read_raw, shared_from_this(),
                                       callback,
                                       _1, _2)));
}

void HttpConnection::write_and_clear(std::vector<unsigned char> &data)
{
  std::shared_ptr<std::vector<unsigned char> > buffer(new std::vector<unsigned char>());
  buffer->swap(data);
  write(asio::buffer(*buffer), buffer);
}

void HttpConnection::write(const std::string &content)
{
  std::shared_ptr<std::string> str(new std::string(content));
  write(asio::buffer(*str), str);
}

void HttpConnection::write(const asio::const_buffer &buffer,
                           ResourcePtr resource)
{
  std::scoped_lock lock(write_mutex_);

  pending_write_buffers_.push_back(buffer);
  if (resource)
    pending_write_resources_.push_back(resource);
  if (!write_in_progress_)
    write_pending();
}

void HttpConnection::write(const std::vector<asio::const_buffer> &buffers,
                           ResourcePtr resource)
{
  std::scoped_lock lock(write_mutex_);
  pending_write_buffers_.insert(pending_write_buffers_.end(), buffers.begin(), buffers.end());
  if (resource)
    pending_write_resources_.push_back(resource);
  if (!write_in_progress_)
    write_pending();
}


// Must be called while holding write lock
void HttpConnection::write_pending()
{
  if (last_error_)
  {
    throw asio::system_error(last_error_);
  }
  write_in_progress_ = true;
  asio::async_write(socket_, pending_write_buffers_,
                           std::bind(&HttpConnection::handle_write, shared_from_this(),
                                       _1,
                                       pending_write_resources_));
  pending_write_buffers_.clear();
  pending_write_resources_.clear();
}

void HttpConnection::handle_write(const asio::error_code &e,
                                  std::vector<ResourcePtr> /*resources*/)
{
  std::scoped_lock lock(write_mutex_);
  write_in_progress_ = false;
  if (!e)
  {
    if (!pending_write_buffers_.empty())
    {
      write_pending();
    }
  }
  else
  {
    last_error_ = e;
  }
}

}
