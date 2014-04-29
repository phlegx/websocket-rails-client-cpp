/**
 *
 * Name        : websocket.hpp
 * Version     : v0.7.0-alpha1
 * Description : Websocket Header File in C++, Ansi-style
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

#ifndef WEBSOCKET_HPP_
#define WEBSOCKET_HPP_

#include <string>
#include <tr1/unordered_map>
#include <vector>
#include <queue>
#include <jsonxx/jsonxx.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>

#define TIMEOUT_CONN 30

typedef boost::function<void(jsonxx::Object)> cb_func;
typedef std::vector<boost::function<void(jsonxx::Object)> > vec_cb_func;
typedef std::tr1::unordered_map<std::string, vec_cb_func> map_vec_cb_func;

#endif /* WEBSOCKET_HPP_ */
