/**
 *
 * Name        : websocket_rails.hpp
 * Version     : v0.7.3-NB
 * Description : WesocketRails Header Class in C++, Ansi-style
 * Author      : Egon Zemmer
 * Company     : Phlegx Systems
 * License     : The MIT License (http://opensource.org/licenses/MIT)
 *
 * Copyright (C) 2015 Egon Zemmer
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

#ifndef WEBSOCKET_RAILS_HPP_
#define WEBSOCKET_RAILS_HPP_

#include "websocket.hpp"
#include "event.hpp"
#include "channel.hpp"
#include "websocket_connection.hpp"

class WebsocketRails {
public:

  /**
   *  Mutex
   **/
  websocketpp::lib::mutex ch_connection_id_mutex;
  websocketpp::lib::mutex ch_token_mutex;
  websocketpp::lib::mutex ch_callbacks_mutex;
  websocketpp::lib::mutex ch_event_queue_mutex;

  websocketpp::lib::mutex state_mutex;
  websocketpp::lib::mutex callbacks_mutex;
  websocketpp::lib::mutex channel_queue_mutex;
  websocketpp::lib::mutex event_queue_mutex;
  typedef websocketpp::lib::lock_guard<websocketpp::lib::mutex> websocket_rails_lock;

  /**
   *  Connection Return Type
   **/
  typedef struct {
    std::string state;
    std::vector<Channel> channels;
  } connection;

  /**
   *  Constructors
   **/
  WebsocketRails();
  WebsocketRails(std::string url);

  /**
   *  Connection functions
   **/
  std::string connect();
  std::string disconnect();
  connection reconnect();
  std::string getState();
  std::string setState(std::string state);
  WebsocketConnection * getConn();
  bool isConnected();

  /**
   *  Connection callbacks
   **/
  void newMessage(jsonxx::Array data);
  void onOpen(cb_func callback);
  void onClose(cb_func callback);
  void onFail(cb_func callback);
  cb_func getOnCloseCallback();
  cb_func getOnFailCallback();


  /**
   *  Event functions
   **/
  void bind(std::string event_name, cb_func callback);
  void unbindAll(std::string event_name);
  void trigger(std::string event_name, jsonxx::Object event_data);
  void trigger(std::string event_name, jsonxx::Object event_data, cb_func success_callback, cb_func failure_callback);
  void triggerEvent(Event event);

  /**
   *  Channel functions
   **/
  Channel subscribe(std::string channel_name);
  Channel subscribe(std::string channel_name, cb_func success_callback, cb_func failure_callback);
  Channel subscribePrivate(std::string channel_name);
  Channel subscribePrivate(std::string channel_name, cb_func success_callback, cb_func failure_callback);
  void unsubscribe(std::string channel_name);
  void unsubscribe(std::string channel_name, cb_func success_callback, cb_func failure_callback);

private:

  /**
   *  Variables
   **/
  std::string url;
  std::string state;
  boost::thread websocket_connection_thread;
  cb_func on_open_callback;
  cb_func on_close_callback;
  cb_func on_fail_callback;
  map_vec_cb_func callbacks;                                        /* Map<key,value>: Event Name, Callback Array     */
  std::tr1::unordered_map<std::string, Channel> channel_queue;      /* Map<key,value>: Channel Name, Channel Object   */
  std::tr1::unordered_map<std::string, Event> event_queue;          /* Map<key,value>: Event UUID, Event Object       */
  WebsocketConnection * conn;

  /**
   *  Functions
   **/
  Channel processSubscribe(std::string channel_name, bool is_private);
  void setConn(WebsocketConnection * conn);
  void connectionEstablished(jsonxx::Object data);
  void dispatch(Event event);
  void dispatchChannel(Event event);
  void pong();
  bool connectionStale();
  std::vector<Channel> reconnectChannels();

};

#endif /* WEBSOCKET_RAILS_HPP_ */
