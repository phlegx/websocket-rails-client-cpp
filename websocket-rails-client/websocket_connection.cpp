/**
 *
 * Name        : websocket.cpp
 * Version     : v0.7.0-alpha1
 * Description : WebsocketConnection Class in C++, Ansi-style
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

#include "websocket_connection.hpp"
#include "websocket_rails.hpp"



/************************************
 *  Constructor                     *
 ************************************/

WebsocketConnection::WebsocketConnection(std::string url, WebsocketRails & dispatcher) : url(url) {
  const std::string connection_type = "websocket";
  this->dispatcher = &dispatcher;

  /* Set up access channels to only log interesting things */
  this->ws_client.clear_access_channels(websocketpp::log::alevel::all);
  this->ws_client.set_access_channels(websocketpp::log::alevel::connect);
  this->ws_client.set_access_channels(websocketpp::log::alevel::disconnect);
  this->ws_client.set_access_channels(websocketpp::log::alevel::app);

  /* Initialize the Asio transport policy */
  this->ws_client.init_asio();

  /* Bind the handlers we are using */
  using websocketpp::lib::placeholders::_1;
  using websocketpp::lib::placeholders::_2;
  using websocketpp::lib::bind;
  this->ws_client.set_open_handler(bind(&WebsocketConnection::openHandler,this,::_1));
  this->ws_client.set_close_handler(bind(&WebsocketConnection::closeHandler,this,::_1));
  this->ws_client.set_fail_handler(bind(&WebsocketConnection::failHandler,this,::_1));
  this->ws_client.set_message_handler(bind(&WebsocketConnection::messageHandler,this,::_1,::_2));
}



/************************************
 *  Functions                       *
 ************************************/

/* This method will block until the connection is complete */
void WebsocketConnection::run() {
  websocketpp::lib::error_code ec;
  client::connection_ptr conn_ptr = this->ws_client.get_connection(this->url, ec);
  if (ec) {
    this->ws_client.get_alog().write(websocketpp::log::alevel::app,
    "Get Connection Error (" + this->url + "): " + ec.message());
    return;
  }
  this->ws_hdl = conn_ptr->get_handle();
  this->ws_client.connect(conn_ptr);
  websocketpp::lib::thread asio_thread(&client::run, &this->ws_client);
  asio_thread.join();
}


void WebsocketConnection::close() {
  this->ws_client.get_alog().write(websocketpp::log::alevel::app,
  "Connection closed by client!");
  websocket_lock guard(ws_mutex);
  if(this->dispatcher && this->dispatcher->getConn() == this) {
    this->ws_client.close(this->ws_hdl, websocketpp::close::status::normal, "Close by client.");
  }
}


/* Trigger an event on the server */
void WebsocketConnection::trigger(Event event) {
  if(this->dispatcher->getState() != "connected") {
    this->event_queue.push(event);
  } else {
    this->sendEvent(event);
  }
}


/* Set the connection id */
std::string WebsocketConnection::setConnectionId(std::string connection_id) {
  return this->connection_id = connection_id;
}


/* Get the connection id */
std::string WebsocketConnection::getConnectionId() {
  return this->connection_id;
}


/* Flush all events in queue */
std::queue<Event> WebsocketConnection::flushQueue() {
  while(!this->event_queue.empty()) {
    this->trigger(this->event_queue.front());
  }
  std::swap(this->event_queue, this->empty);
  return this->event_queue;
}



/********************************************************
 *                                                      *
 * PRIVATE METHODS                                      *
 *                                                      *
 ********************************************************/

/* The open handler will signal that we are ready to start sending */
void WebsocketConnection::openHandler(websocketpp::connection_hdl hdl) {
  this->ws_client.get_alog().write(websocketpp::log::alevel::app,
  "Connection opened, starting websocket!");
  websocket_lock guard(ws_mutex);
}


/* The close handler will signal that we should stop sending */
void WebsocketConnection::closeHandler(websocketpp::connection_hdl hdl) {
  this->ws_client.get_alog().write(websocketpp::log::alevel::app,
  "Connection closed, stopping websocket!");
  websocket_lock guard(ws_mutex);
  if(this->dispatcher && this->dispatcher->getConn() == this) {
    this->dispatcher->setState("disconnected");
    if(this->dispatcher->getOnCloseCallback()) {
      cb_func callback = this->dispatcher->getOnCloseCallback();
      callback(jsonxx::Object("connection_id", this->connection_id));
    }
  }
}


/* The fail handler will signal that we should stop sending */
void WebsocketConnection::failHandler(websocketpp::connection_hdl hdl) {
  this->ws_client.get_alog().write(websocketpp::log::alevel::app,
  "Connection failed, stopping websocket!");
  websocket_lock guard(ws_mutex);
  if(this->dispatcher && this->dispatcher->getConn() == this) {
    this->dispatcher->setState("disconnected");
    if(this->dispatcher->getOnFailCallback()) {
      cb_func callback = this->dispatcher->getOnFailCallback();
      callback(jsonxx::Object("connection_id", this->connection_id));
    }
  }
}


/* The message handler will signal that we have received a message */
void WebsocketConnection::messageHandler(websocketpp::connection_hdl hdl, message_ptr msg) {
  std::string event_names;
  std::string tmp = msg->get_payload();
  jsonxx::Array event_data;
  event_data.parse(tmp);
  for(int i = 0, len = event_data.size(); i < len; i++) {
    event_names += event_data.get<jsonxx::Array>(i).get<jsonxx::String>(0) + " ";
  }
  this->ws_client.get_alog().write(websocketpp::log::alevel::app,
  "Message arrived: " + event_names);
  if(this->dispatcher && this->dispatcher->getConn() == this) {
    this->dispatcher->newMessage(event_data);
  }
}


void WebsocketConnection::sendEvent(Event event) {
  if(this->connection_id != "") {
    event.setConnectionId(this->connection_id);
  }
  websocketpp::lib::error_code ec;
  this->ws_client.send(this->ws_hdl, event.serialize(), websocketpp::frame::opcode::text, ec);
  if(ec) {
    this->ws_client.get_alog().write(websocketpp::log::alevel::app,
    "Send Error: " + ec.message());
  }
}
