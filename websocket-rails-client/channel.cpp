/**
 *
 * Name        : channel.cpp
 * Version     : v0.7.0-alpha1
 * Description : Channel Class in C++, Ansi-style
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

#include "channel.hpp"
#include "websocket_rails.hpp"



/************************************
 *  Constructors                    *
 ************************************/

Channel::Channel() : is_private(false), dispatcher() {}


Channel::Channel(std::string name, WebsocketRails & dispatcher, bool is_private) : is_private(is_private), name(name) {
  this->dispatcher = &dispatcher;
  this->initObject();
}


Channel::Channel(std::string name, WebsocketRails & dispatcher, bool is_private, cb_func on_success, cb_func on_failure) : is_private(is_private), name(name) {
  this->on_success = on_success;
  this->on_failure = on_failure;
  this->dispatcher = &dispatcher;
  this->initObject();
}



/************************************
 *  Functions                       *
 ************************************/

void Channel::destroy() {
  if(this->connection_id == (this->dispatcher->getConn() != 0 ? this->dispatcher->getConn()->getConnectionId() : "")) {
    std::string event_name = "websocket_rails.unsubscribe";
    jsonxx::Array data = this->initEventData(event_name);
    Event event(data);
    this->dispatcher->triggerEvent(event);
  }
  this->callbacks.clear();
}


void Channel::bind(std::string event_name, cb_func callback) {
  if(this->callbacks.find(event_name) == this->callbacks.end()) {
    vec_cb_func v;
    this->callbacks[event_name] = v;
  }
  return this->callbacks[event_name].push_back(callback);
}


Event Channel::trigger(std::string event_name, jsonxx::Object event_data) {
  jsonxx::Array data = this->initEventData(event_name);
  data.get<jsonxx::Object>(1).import("channel", this->name);
  data.get<jsonxx::Object>(1).import("data", event_data);
  data.get<jsonxx::Object>(1).import("token", this->token);
  Event event(data);
  if(this->token.empty()) {
    this->event_queue.push(event);
    return event;
  } else {
    return this->dispatcher->triggerEvent(event);
  }
}


std::string Channel::getName() {
  return this->name;
}


map_vec_cb_func Channel::getCallbacks() {
  return this->callbacks;
}


void Channel::setCallbacks(map_vec_cb_func callbacks) {
  this-> callbacks = callbacks;
}


bool Channel::isPrivate() {
  return this->isPrivate();
}


void Channel::dispatch(std::string event_name, jsonxx::Object event_data) {
  if(event_name == "websocket_rails.channel_token") {
    this->connection_id =  this->dispatcher->getConn() != 0 ? this->dispatcher->getConn()->getConnectionId() : "";
    this->token = event_data.get<jsonxx::String>("token");
    this->flush_queue();
  } else {
    if(this->callbacks.find(event_name) == this->callbacks.end()) {
      return;
    }
    vec_cb_func event_callbacks = this->callbacks[event_name];
    for(vec_cb_func::iterator it = event_callbacks.begin(); it != event_callbacks.end(); ++it) {
      cb_func callback = *it;
      callback(event_data);
    }
  }
}



/********************************************************
 *                                                      *
 * PRIVATE METHODS                                      *
 *                                                      *
 ********************************************************/

void Channel::initObject() {
  std::string event_name;
  if(this->is_private) {
    event_name = "websocket_rails.subscribe_private";
  } else {
    event_name = "websocket_rails.subscribe";
  }
  this->connection_id = this->dispatcher->getConn() != 0 ? this->dispatcher->getConn()->getConnectionId() : "";
  jsonxx::Array data = this->initEventData(event_name);
  Event event(data, boost::bind(&Channel::successLauncher, this, _1), boost::bind(&Channel::failureLauncher, this, _1));
  this->dispatcher->triggerEvent(event);
}


jsonxx::Array Channel::initEventData(std::string event_name) {
  jsonxx::Array data;
  jsonxx::Object event_data;
  event_data << "data" << jsonxx::Object("channel", this->name);
  data << event_name << event_data << this->connection_id;
  return data;
}


void Channel::successLauncher(jsonxx::Object data) {
  if(this->on_success) {
    this->on_success(data);
  }
}


void Channel::failureLauncher(jsonxx::Object data) {
  if (this->on_failure) {
    this->on_failure(data);
  }
}


std::queue<Event> Channel::flush_queue() {
  while(!this->event_queue.empty()) {
    this->dispatcher->triggerEvent(this->event_queue.front());
  }
  std::swap(this->event_queue, this->empty);
  return this->event_queue;
}
