/**
 *
 * Name        : channel.hpp
 * Version     : v0.7.0-alpha1
 * Description : Channel Header Class in C++, Ansi-style
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

#ifndef CHANNEL_HPP_
#define CHANNEL_HPP_

class WebsocketRails;

#include "websocket.hpp"
#include "event.hpp"

class Channel {
public:

  /**
   *  Constructors
   **/
  Channel();
  Channel(std::string name, WebsocketRails & dispatcher, bool is_private);
  Channel(std::string name, WebsocketRails & dispatcher, bool is_private, cb_func on_success, cb_func on_failure);

  /**
   *  Functions
   **/
  void destroy();
  void bind(std::string event_name, cb_func callback);
  Event trigger(std::string event_name, jsonxx::Object event_data);
  std::string getName();
  map_vec_cb_func getCallbacks();
  void setCallbacks(map_vec_cb_func callbacks);
  bool isPrivate();
  void dispatch(std::string event_name, jsonxx::Object event_data);

private:

  /**
   *  Variables
   **/
  bool is_private;
  std::string connection_id;
  std::string name;
  std::string token;
  cb_func on_success;
  cb_func on_failure;
  map_vec_cb_func callbacks; 	    /* Map<key,value>: Event Name, Callback Array */
  std::queue<Event> empty;
  std::queue<Event> event_queue;
  WebsocketRails * dispatcher;

  /**
   *  Functions
   **/
  void initObject();
  jsonxx::Array initEventData(std::string event_name);
  void successLauncher(jsonxx::Object data);
  void failureLauncher(jsonxx::Object data);
  std::queue<Event> flush_queue();

};


#endif /* CHANNEL_HPP_ */
