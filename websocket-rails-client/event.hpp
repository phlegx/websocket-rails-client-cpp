/**
 *
 * Name        : event.hpp
 * Version     : v0.7.0-alpha1
 * Description : Event Header Class in C++, Ansi-style
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

#ifndef EVENT_HPP_
#define EVENT_HPP_

#include "websocket.hpp"

class Event {
public:

  /**
   *  Constructors
   **/
  Event();
  Event(jsonxx::Array data);
  Event(jsonxx::Array data, cb_func success_callback, cb_func failure_callback);

  /**
   *  Functions
   **/
  bool isChannel();
  bool isResult();
  bool isPing();
  std::string serialize();
  void runCallbacks(bool success, jsonxx::Object result);
  std::string getConnectionId();
  std::string setConnectionId(std::string connection_id);
  std::string getId();
  std::string getName();
  std::string getChannel();
  jsonxx::Object getData();
  bool getSuccess();

private:

  /**
   *  Variables
   **/
  bool success;
  bool result;
  std::string id;
  std::string connection_id;
  std::string name;
  std::string channel;
  std::string token;				/* Partially used for the moment */
  std::string server_token; /* Not used for the moment */
  std::string user_id;      /* Not used for the moment */
  jsonxx::Object attr;
  jsonxx::Object data;
  cb_func success_callback;
  cb_func failure_callback;

  /**
   *  Functions
   **/
  void initObject(jsonxx::Array data);
  jsonxx::Object attributes();

};


#endif /* EVENT_HPP_ */
