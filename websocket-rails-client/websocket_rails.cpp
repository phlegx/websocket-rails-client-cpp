/**
 *
 * Name        : websocket_rails.cpp
 * Version     : v0.7.0-alpha1
 * Description : WebsocketRails Class in C++, Ansi-style
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

#include "websocket_rails.hpp"



/************************************
 *  Constructors                    *
 ************************************/

WebsocketRails::WebsocketRails(std::string url) : url(url), conn() {}



/************************************
 *  Connection functions            *
 ************************************/

std::string WebsocketRails::connect() {
  this->state = "connecting";
  this->conn = new WebsocketConnection(this->url, *this);
  this->websocket_connection_thread = boost::thread(&WebsocketConnection::run, this->conn);
  int count = 0;
  while(this->connectionStale()) {
    boost::posix_time::seconds workTime(1);
    boost::this_thread::sleep(workTime);
    count++;
    if(count == TIMEOUT_CONN) {
      return this->disconnect();
    }
  }
  return this->state;
}


std::string WebsocketRails::disconnect() {
  if(this->conn != 0) {
    this->conn->close();
    this->websocket_connection_thread.join();
    delete this->conn;
  }
  return this->state = "disconnected";
}


WebsocketRails::connection WebsocketRails::reconnect() {
  connection conn_struct;
  std::string oldconnection_id = this->conn != 0 ? this->conn->getConnectionId() : "";
  this->disconnect();
  if(this->connect() == "connected") {
    for(auto& x: this->queue) {
        Event event = x.second;
        if(event.getConnectionId() == oldconnection_id && !event.isResult()) {
          this->triggerEvent(event);
        }
    }
    conn_struct.channels = this->reconnectChannels(); 
  }
  conn_struct.state = this->state;
  return conn_struct;
}


/* Get Connection State */
std::string WebsocketRails::getState() {
  return this->state;
}


/* Set Connection State */
std::string WebsocketRails::setState(std::string state) {
  return this->state = state;
}


/* Get Connection Object */
WebsocketConnection * WebsocketRails::getConn() {
  return this->conn;
}


bool WebsocketRails::isConnected() {
  return !this->connectionStale();
}



/************************************
 *  Connection callbacks            *
 ************************************/

void WebsocketRails::newMessage(jsonxx::Array data) {
  for(int _i = 0, _len = data.size(); _i < _len; _i++) {
    jsonxx::Array socket_message = data.get<jsonxx::Array>(_i);
    Event event(socket_message);
    if(event.isResult()) {
      if(this->queue.find(event.getId()) != this->queue.end()) {
        this->queue[event.getId()].runCallbacks(event.getSuccess(), event.getData());
      }
      this->queue.erase(event.getId());
    } else if(event.isChannel()) {
      this->dispatchChannel(event);
    } else if(event.isPing()) {
      this->pong();
    } else {
      this->dispatch(event);
    }
    if(this->state == "connecting" && event.getName() == "client_connected") {
      this->connectionEstablished(event.getData());
    }
  }
}


void WebsocketRails::onOpen(cb_func callback) {
  this->on_open_callback = callback;
}


void WebsocketRails::onClose(cb_func callback) {
  this->on_close_callback = callback;
}


void WebsocketRails::onFail(cb_func callback) {
  this->on_fail_callback = callback;
}


cb_func WebsocketRails::getOnCloseCallback() {
  return this->on_close_callback;
}


cb_func WebsocketRails::getOnFailCallback() {
  return this->on_fail_callback;
}



/************************************
 *  Event functions                 *
 ************************************/

void WebsocketRails::bind(std::string event_name, cb_func callback) {
  if(this->callbacks.find(event_name) == this->callbacks.end()) {
    vec_cb_func v;
    this->callbacks[event_name] = v;
  }
  this->callbacks[event_name].push_back(callback);
}


Event WebsocketRails::trigger(std::string event_name, jsonxx::Object event_data) {
  jsonxx::Array data;
  data << event_name << event_data << (this->conn != 0 ? this->conn->getConnectionId() : "");
  Event event(data);
  return this->triggerEvent(event);
}


Event WebsocketRails::trigger(std::string event_name, jsonxx::Object event_data, cb_func success_callback, cb_func failure_callback) {
  jsonxx::Array data;
  data << event_name << event_data << (this->conn != 0 ? this->conn->getConnectionId() : "");
  Event event(data, success_callback, failure_callback);
  return this->triggerEvent(event);
}


Event WebsocketRails::triggerEvent(Event event) {
  if(this->queue.find(event.getId()) == this->queue.end()) {
    this->queue[event.getId()] = event;
  }
  if(this->conn != 0) {
    this->conn->trigger(event);
  }
  return event;
}



/************************************
 *  Channel functions               *
 ************************************/

Channel WebsocketRails::subscribe(std::string channel_name) {
  if(this->channels.find(channel_name) == this->channels.end()) {
    Channel channel(channel_name, *this, false);
    this->channels[channel_name] = channel;
    return channel;
  } else {
    return this->channels[channel_name];
  }
}


Channel WebsocketRails::subscribe(std::string channel_name, cb_func success_callback, cb_func failure_callback) {
  if(this->channels.find(channel_name) == this->channels.end()) {
    Channel channel(channel_name, *this, false, success_callback, failure_callback);
    this->channels[channel_name] = channel;
    return channel;
  } else {
    return this->channels[channel_name];
  }
}


Channel WebsocketRails::subscribePrivate(std::string channel_name) {
  if(this->channels.find(channel_name) == this->channels.end()) {
    Channel channel(channel_name, *this, true);
    this->channels[channel_name] = channel;
    return channel;
  } else {
    return this->channels[channel_name];
  }
}


Channel WebsocketRails::subscribePrivate(std::string channel_name, cb_func success_callback, cb_func failure_callback) {
  if(this->channels.find(channel_name) == this->channels.end()) {
    Channel channel(channel_name, *this, true, success_callback, failure_callback);
    this->channels[channel_name] = channel;
    return channel;
  } else {
    return this->channels[channel_name];
  }
}


void WebsocketRails::unsubscribe(std::string channel_name) {
  if(this->channels.find(channel_name) == this->channels.end()) {
    return;
  }
  this->channels[channel_name].destroy();
  this->channels.erase(channel_name);
}


/********************************************************
 *                                                      *
 * PRIVATE METHODS                                      *
 *                                                      *
 ********************************************************/


void WebsocketRails::connectionEstablished(jsonxx::Object event_data) {
  this->state = "connected";
  this->conn->setConnectionId(event_data.get<jsonxx::String>("connection_id"));
  this->conn->flushQueue();
  if(this->on_open_callback) {
    this->on_open_callback(event_data);
  }
}


void WebsocketRails::dispatch(Event event) {
  if(this->callbacks.find(event.getName()) == this->callbacks.end()) {
    return;
  }
  vec_cb_func event_callbacks = this->callbacks[event.getName()];
  for(vec_cb_func::iterator it = event_callbacks.begin(); it != event_callbacks.end(); ++it) {
    cb_func callback = *it;
    callback(event.getData());
  }
}


void WebsocketRails::dispatchChannel(Event event) {
  if(this->channels.find(event.getChannel()) == this->channels.end()) {
    return;
  }
  this->channels[event.getChannel()].dispatch(event.getName(), event.getData());
}


void WebsocketRails::pong() {
  jsonxx::Array data;
  data << "websocket_rails.pong" << jsonxx::Object() << (this->conn != 0 ? this->conn->getConnectionId() : "");
  Event pong(data);
  this->conn->trigger(pong);
}


bool WebsocketRails::connectionStale() {
  return this->state != "connected";
}


std::vector<Channel> WebsocketRails::reconnectChannels() {
  std::vector<Channel> results;
  for (auto& x: this->channels) {
    Channel channel = x.second;
    callbacks = channel.getCallbacks();
    channel.destroy();
    std::string channel_name = channel.getName();
    this->channels.erase(channel_name);
    channel = channel.isPrivate() ? this->subscribePrivate(channel_name) : this->subscribe(channel_name);
    channel.setCallbacks(callbacks);
    results.push_back(channel);
  }
  return results;
}
