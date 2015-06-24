/**
 *
 * Name        : websocket_rails.cpp
 * Version     : v0.7.4-NB
 * Description : WebsocketRails Class in C++, Ansi-style
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

#include "websocket_rails.hpp"



/************************************
 *  Constructors                    *
 ************************************/

WebsocketRails::WebsocketRails(std::string url) : url(url), conn() {}



/************************************
 *  Connection functions            *
 ************************************/

std::string WebsocketRails::connect() {
  this->setState("connecting");
  this->setConn(new WebsocketConnection(this->url, *this));
  this->websocket_connection_thread = boost::thread(&WebsocketConnection::run, this->getConn());
  int count = 0;
  while(!this->isConnected()) {
    boost::posix_time::seconds workTime(1);
    boost::this_thread::sleep(workTime);
    count++;
    if(count == TIMEOUT_CONN) {
      return this->disconnect();
    }
  }
  return this->getState();
}


std::string WebsocketRails::disconnect() {
  if(this->getConn() != NULL) {
    if(this->isConnected()) {
      this->getConn()->close();
    }
    this->websocket_connection_thread.interrupt();
    this->websocket_connection_thread.join();
    delete this->getConn();
    this->setConn(NULL);
  }
  return this->setState("disconnected");
}


void WebsocketRails::reconnect() {
  std::string oldconnection_id = this->getConn() != NULL ? this->getConn()->getConnectionId() : "";
  this->disconnect();
  if(this->connect() == "connected") {
    for(auto& x: this->event_queue) {
        Event event = x.second;
        if(event.getConnectionId() == oldconnection_id && !event.isResult()) {
          this->triggerEvent(event);
        }
    }
    this->reconnectChannels();
  }
}


/* Get Connection State */
std::string WebsocketRails::getState() {
  websocket_rails_lock guard(state_mutex);
  return this->state;
}


/* Set Connection State */
std::string WebsocketRails::setState(std::string state) {
  websocket_rails_lock guard(state_mutex);
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
      Event tmp;
      {
        websocket_rails_lock guard(event_queue_mutex);
        if(this->event_queue.find(event.getId()) != this->event_queue.end()) {
          tmp = this->event_queue[event.getId()];
        }
        this->event_queue.erase(event.getId());
      }
      tmp.runCallbacks(event.getSuccess(), event.getData());
    } else if(event.isChannel()) {
      this->dispatchChannel(event);
    } else if(event.isPing()) {
      this->pong();
    } else {
      this->dispatch(event);
    }
    if(this->getState() == "connecting" && event.getName() == "client_connected") {
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
  websocket_rails_lock guard(callbacks_mutex);
  if(this->callbacks.find(event_name) == this->callbacks.end()) {
    vec_cb_func v;
    this->callbacks[event_name] = v;
  }
  this->callbacks[event_name].push_back(callback);
}


void WebsocketRails::unbindAll(std::string event_name) {
  websocket_rails_lock guard(callbacks_mutex);
  if(this->callbacks.find(event_name) != this->callbacks.end()) {
    this->callbacks.erase(event_name);
  }
}


void WebsocketRails::trigger(std::string event_name, jsonxx::Object event_data) {
  jsonxx::Array data;
  data << event_name << event_data << (this->getConn() != NULL ? this->getConn()->getConnectionId() : "");
  Event event(data);
  this->triggerEvent(event);
}


void WebsocketRails::trigger(std::string event_name, jsonxx::Object event_data, cb_func success_callback, cb_func failure_callback) {
  jsonxx::Array data;
  data << event_name << event_data << (this->getConn() != NULL ? this->getConn()->getConnectionId() : "");
  Event event(data, success_callback, failure_callback);
  this->triggerEvent(event);
}


void WebsocketRails::triggerEvent(Event event) {
  {
    websocket_rails_lock guard(event_queue_mutex);
    if(this->event_queue.find(event.getId()) == this->event_queue.end()) {
      this->event_queue[event.getId()] = event;
    }
  }
  if(this->getConn() != NULL) {
    this->getConn()->trigger(event);
  }
}



/************************************
 *  Channel functions               *
 ************************************/


Channel * WebsocketRails::getChannel(std::string channel_name) {
  websocket_rails_lock guard(channel_queue_mutex);
  return &this->channel_queue[channel_name];
}


Channel * WebsocketRails::subscribe(std::string channel_name) {
  websocket_rails_lock guard(channel_queue_mutex);
  return this->processSubscribe(channel_name, false);
}


Channel * WebsocketRails::subscribe(std::string channel_name, cb_func success_callback, cb_func failure_callback) {
  websocket_rails_lock guard(channel_queue_mutex);
  if(this->channel_queue.find(channel_name) == this->channel_queue.end()) {
    Channel channel(channel_name, *this, false, success_callback, failure_callback);
    this->channel_queue[channel_name] = channel;
  }
  return &this->channel_queue[channel_name];
}


Channel * WebsocketRails::subscribePrivate(std::string channel_name) {
  websocket_rails_lock guard(channel_queue_mutex);
  return this->processSubscribe(channel_name, true);
}


Channel * WebsocketRails::subscribePrivate(std::string channel_name, cb_func success_callback, cb_func failure_callback) {
  websocket_rails_lock guard(channel_queue_mutex);
  if(this->channel_queue.find(channel_name) == this->channel_queue.end()) {
    Channel channel(channel_name, *this, true, success_callback, failure_callback);
    this->channel_queue[channel_name] = channel;
  }
  return &this->channel_queue[channel_name];
}


void WebsocketRails::unsubscribe(std::string channel_name) {
  Channel * channel;
  {
    websocket_rails_lock guard(channel_queue_mutex);
    if(this->channel_queue.find(channel_name) == this->channel_queue.end()) {
      return;
    }
    channel = &this->channel_queue[channel_name];
    this->channel_queue.erase(channel_name);
  }
  cb_func success_callback, failure_callback;
  channel->destroy(success_callback, failure_callback);
}


void WebsocketRails::unsubscribe(std::string channel_name, cb_func success_callback, cb_func failure_callback) {
  Channel * channel;
  {
    websocket_rails_lock guard(channel_queue_mutex);
    if(this->channel_queue.find(channel_name) == this->channel_queue.end()) {
      return;
    }
    channel = &this->channel_queue[channel_name];
    this->channel_queue.erase(channel_name);
  }
  channel->destroy(success_callback, failure_callback);
}



/********************************************************
 *                                                      *
 * PRIVATE METHODS                                      *
 *                                                      *
 ********************************************************/


Channel * WebsocketRails::processSubscribe(std::string channel_name, bool is_private) {
  if(this->channel_queue.find(channel_name) == this->channel_queue.end()) {
    Channel channel(channel_name, *this, is_private);
    this->channel_queue[channel_name] = channel;
  }
  return &this->channel_queue[channel_name];
}


void WebsocketRails::setConn(WebsocketConnection * conn) {
  this->conn = conn;
}


void WebsocketRails::connectionEstablished(jsonxx::Object event_data) {
  this->setState("connected");
  this->getConn()->setConnectionId(event_data.get<jsonxx::String>("connection_id"));
  this->getConn()->flushQueue();
  if(this->on_open_callback) {
    this->on_open_callback(event_data);
  }
}


void WebsocketRails::dispatch(Event event) {
  vec_cb_func event_callbacks;
  {
    websocket_rails_lock guard(callbacks_mutex);
    if(this->callbacks.find(event.getName()) == this->callbacks.end()) {
      return;
    }
    event_callbacks = this->callbacks[event.getName()];
  }
  for(vec_cb_func::iterator it = event_callbacks.begin(); it != event_callbacks.end(); ++it) {
    cb_func callback = *it;
    callback(event.getData());
  }
}


void WebsocketRails::dispatchChannel(Event event) {
  Channel * channel;
  {
    websocket_rails_lock guard(channel_queue_mutex);
    if(this->channel_queue.find(event.getChannel()) == this->channel_queue.end()) {
      return;
    }
    channel = &this->channel_queue[event.getChannel()];
  }
  channel->dispatch(event.getName(), event.getData());
}


void WebsocketRails::pong() {
  jsonxx::Array data;
  data << "websocket_rails.pong" << jsonxx::Object() << (this->getConn() != NULL ? this->getConn()->getConnectionId() : "");
  Event pong(data);
  this->getConn()->trigger(pong);
}


bool WebsocketRails::connectionStale() {
  return this->getState() != "connected";
}


void WebsocketRails::reconnectChannels() {
  websocket_rails_lock guard(channel_queue_mutex);
  std::tr1::unordered_map<std::string, Channel> channel_queue_old = this->channel_queue;
  for(auto& x: channel_queue_old) {
    Channel * channel = &x.second;
    map_vec_cb_func callbacks = channel->getCallbacks();
    cb_func success_callback, failure_callback;
    channel->destroy(success_callback, failure_callback);
    std::string channel_name = channel->getName();
    this->channel_queue.erase(channel_name);
    channel = this->processSubscribe(channel_name, channel->isPrivate());
    channel->setCallbacks(callbacks);
  }
}

