/**
 *
 * Name        : event.cpp
 * Version     : v0.7.0-alpha1
 * Description : Event Class in C++, Ansi-style
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

#include "event.hpp"



/************************************
 *  Constructors                    *
 ************************************/

Event::Event() : success(false), result(false) {}


Event::Event(jsonxx::Array data) : success(false), result(false) {
  this->initObject(data);
}


Event::Event(jsonxx::Array data, cb_func success_callback, cb_func failure_callback) : success(false), result(false) {
  this->success_callback = success_callback;
  this->failure_callback = failure_callback;
  this->initObject(data);
}



/************************************
 *  Functions                       *
 ************************************/

bool Event::isChannel() {
  return !this->channel.empty();
}


bool Event::isResult() {
  return this->result;
}


bool Event::isPing() {
  return this->name == "websocket_rails.ping";
}


std::string Event::serialize() {
  jsonxx::Array arr;
  arr << this->name;
  arr << this->attributes();
  return arr.json();
}


void Event::runCallbacks(bool success, jsonxx::Object event_data) {
  if(success) {
    if(this->success_callback)
      this->success_callback(event_data);
  } else {
    if(this->failure_callback)
      this->failure_callback(event_data);
  }
}


/* Get the connection id */
std::string Event::getConnectionId() {
  return this->connection_id;
}


/* Get the connection id */
std::string Event::setConnectionId(std::string connection_id) {
  return this->connection_id = connection_id;
}


/* Get the event id */
std::string Event::getId() {
  return this->id;
}


/* Get name of event */
std::string Event::getName() {
  return this->name;
}


/* Get channel of event */
std::string Event::getChannel() {
  return this->channel;
}


/* Get data of event */
jsonxx::Object Event::getData() {
  return this->data;
}


/* Get Success of event */
bool Event::getSuccess() {
  return this->success;
}



/********************************************************
 *                                                      *
 * PRIVATE METHODS                                      *
 *                                                      *
 ********************************************************/

void Event::initObject(jsonxx::Array data) {
  this->name = data.get<jsonxx::String>(0);
  if(data.has<jsonxx::Object>(1)) {
    this->attr = data.get<jsonxx::Object>(1);
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    this->id = this->attr.has<jsonxx::String>("id") ? this->attr.get<jsonxx::String>("id") : boost::lexical_cast<std::string>(uuid);
    this->channel = this->attr.has<jsonxx::String>("channel") ? this->attr.get<jsonxx::String>("channel") : "";
    this->data = this->attr.has<jsonxx::Object>("data") ? this->attr.get<jsonxx::Object>("data") : this->attr;
    this->token = this->attr.has<jsonxx::String>("token") ? this->attr.get<jsonxx::String>("token") : "";
    this->server_token = this->attr.has<jsonxx::String>("server_token") ? this->attr.get<jsonxx::String>("server_token") : "";
    this->user_id = this->attr.has<jsonxx::String>("user_id") ? this->attr.get<jsonxx::String>("user_id") : "";
    if(this->data.has<jsonxx::String>("connection_id")) {
      this->connection_id = this->data.get<jsonxx::String>("connection_id");
    }
    if(this->attr.has<jsonxx::Boolean>("success")) {
      this->result = true;
      this->success = this->attr.get<jsonxx::Boolean>("success");
    }
  }
}


jsonxx::Object Event::attributes() {
  jsonxx::Object obj;
  if(this->name != "websocket_rails.pong") { obj << "id"      << this->id;      }
  if(!this->channel.empty())               { obj << "channel" << this->channel; }
  if(!this->data.empty())                  { obj << "data"    << this->data;    }
  if(!this->token.empty())                 { obj << "token"   << this->token;   }
  return obj;
}
