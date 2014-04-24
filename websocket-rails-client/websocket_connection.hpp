/**
 *
 * Name        : websocket.hpp
 * Version     : v0.7.0-alpha1
 * Description : WebsocketConnection Header Class in C++, Ansi-style
 * Author      : Egon Zemmer
 * Company     : Phlegx Systems
 * License     : The MIT License (http://opensource.org/licenses/MIT)
 *
 * Copyright (C) 2014 Egon Zemmer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef WEBSOCKET_CONNECTION_HPP
#define WEBSOCKET_CONNECTION_HPP

#include "websocket.hpp"
#include "event.hpp"

class WebsocketRails;

class WebsocketConnection {
public:

  /**
   *  Type Definitions and Variables
   **/
  typedef websocketpp::lib::lock_guard<websocketpp::lib::mutex> websocket_lock;
  typedef websocketpp::client<websocketpp::config::asio_client> client;
  typedef websocketpp::config::asio_client::message_type::ptr message_ptr;
  websocketpp::lib::mutex ws_mutex;
  static const std::string connection_type;

  /**
   *  Constructor
   **/
  WebsocketConnection(std::string url, WebsocketRails & dispatcher);

  /**
   *  Functions
   **/
  void run();
  void close();
  void trigger(Event event);
  std::string setConnectionId(std::string connection_id);
  std::string getConnectionId();
  std::queue<Event> flushQueue();

private:

  /**
   *  Variables
   **/
  std::string connection_id;
  std::string url;
  WebsocketRails * dispatcher;
  std::queue<Event> empty;
  std::queue<Event> event_queue;
  websocketpp::connection_hdl ws_hdl;
  client ws_client;

  /**
   *  Functions
   **/
  void openHandler(websocketpp::connection_hdl hdl);
  void closeHandler(websocketpp::connection_hdl hdl);
  void failHandler(websocketpp::connection_hdl hdl);
  void messageHandler(websocketpp::connection_hdl hdl, message_ptr msg);
  void sendEvent(Event event);

};

#endif /* WEBSOCKET_CONNECTION_HPP */
