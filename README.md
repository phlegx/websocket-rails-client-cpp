# WebsocketRailsClient++ (v0.7.0-alpha1)

WebsocketRailsClient++ is a C++ library that uses the implementation of RFC6455 (The WebSocket Protocol)
implemented in the WebSocket++ library, the Json++ light-weight JSON parser and the Boost library. It allows
connections to the Ruby Websocket-Rails server and supports the Websocket-Rails protocol.

## Requirements

* Boost library up and running, http://www.boost.org/ (>= 1.48 | see own license)
* WebSocket++ library, https://github.com/zaphoyd/websocketpp (>= 0.3.0 | see own license)
* Jsonxx library, https://github.com/hjiang/jsonxx (latest | see own license)
* Websocket-Rails server, https://github.com/websocket-rails/websocket-rails (v0.7.0)


## Features

* Full support for RFC6455
* Full Websocket-Rails protocol support
* Message/event based interface
* Supports secure WebSockets (TLS), IPv6, and explicit proxies.
* Callbacks with member and non-member functions
* Thread-safe


## Websocket-Rails Protocol Implementation


### Event Dispatcher

#### Connection

 * ```connect(std::string url)```    : Start connection of the client.
 * ```disconnect()``` : Disconnect client.
 * ```reconnect()```  : Re-connect the client with all registered channels.

#### Connection Callbacks

 * ```on_open(boost::bind cb)```  : callback on open connection.
 * ```on_close(boost::bind cb)``` : callback on close connection.
 * ```on_fail(boost::bind cb)```  : callback on fail connection.

#### Trigger an Event on Server

* ```trigger(std::string event_name, jsonxx::Object event_data)``` : trigger event with data without callback.
* ```trigger(std::string event_name, jsonxx::Object event_data, boost::bind cb_succ, boost::bind cb_fail)``` : trigger event with data and callbacks.

#### Bind to an Incoming Event

* ```bind(std::string event_name, boost::bind cb)``` : Bind to an event with callback.


### Channel Event Dispatcher

#### Channel Management

* ```subscribe(std::string channel_name)``` : Subscribe to a channel.
* ```subscribePrivate(std::string channel_name, boost::bind cb_succ, boost::bind cb_fail)``` : Subscribe to a private channel.
* ```unsubscribe(std::string channel_name)``` : Unsubscribe a channel.

#### Trigger a Channel-Event on Server

* ```trigger(std::string event_name, jsonxx::Object event_data)``` : trigger channel event with data without callback.
* ```trigger(std::string event_name, jsonxx::Object event_data, boost::bind cb_succ, boost::bind cb_fail)``` : trigger channel event with data and callbacks.

#### Bind to an Incoming Channel-Event

* ```bind(std::string event_name, boost::bind cb)``` : Bind to a channel event with callback.

## Compile

### C++ Linker

#### Linker Flag -l

* **Booost libraries:** boost_systen, boost_thread
* **System libraries:** pthread, rt
* **TSL libraries:** ssl, crypto

### C++ Compiler

#### Compiler Flag -D

* ```-D_WEBSOCKETPP_CPP11_STL_```

#### Compiler Flag -I

* ```-I"/path/to/includes"```

#### Compiler Flag -std

* ```-std=c++0x```


## Usage

* Includes

```cpp
#include <iostream>
#include "websocket-rails-client/websocket_rails.hpp"
```

* Callback definitions

```cpp
/* Non-member callback function for onOpen */
void on_open(jsonxx::Object data) {
  std::cout << "Function on_open called" << std::endl;
  std::cout << data.json() << std::endl;
}

/* Non-member callback functions */
void callback(jsonxx::Object data) {
  std::cout << "Function callback called" << std::endl;
  std::cout << data.json() << std::endl;
}
void success_func(jsonxx::Object data) { ... }
void failure_func(jsonxx::Object data) { ... }
void bind_func_with_params(jsonxx::Object data, int number, bool valid) { ... }
```

* Initialization

```cpp
WebsocketRails dispatcher("ws://localhost:3000/websocket");

/* Connection callbacks definitions */
dispatcher.onOpen(boost::bind(on_open, _1));
dispatcher.onClose(boost::bind(on_close, _1));
dispatcher.onFail(boost::bind(on_fail, _1));

/* Define binds before connect. */

/* Start Websocket-Rails connection handshake */
dispatcher.connect();
```

* Bind events to callbacks

```cpp
/* Event bind callback definition */
dispatcher.bind("users.pool", boost::bind(callback, _1));

/* Event bind callback definition with member function callback */
Foo my_foo;
Event event3 = dispatcher.bind("users.pool", boost::bind(&Foo::success_func, my_foo, _1));
```

* Trigger events

```cpp
/* Trigger an event */
Event event1 = dispatcher.trigger("users.create", jsonxx::Object("name", "Hans Mustermann"));
Event event2 = dispatcher.trigger("users.create", jsonxx::Object("name", "Frau Mustermann"), boost::bind(success_func, _1), boost::bind(failure_func, _1));

/* Trigger an event with member function callback */
Bar my_bar;
Event event3 = dispatcher.trigger("users.pool", boost::bind(&Foo::success_func, my_bar, _1), boost::bind(&Foo::failure_func, my_bar, _1));
```

* Use channels

```cpp
/* Subscribe to channels */
Channel channel1 = dispatcher.subscribe("Authors");
Channel channel2 = dispatcher.subscribe("Users", boost::bind(success_func, _1), boost::bind(failure_func, _1));
Channel private_channel1 = dispatcher.subscribePrivate("Administrators");
Channel private_channel2 = dispatcher.subscribePrivate("Moderators", boost::bind(success_func, _1), boost::bind(failure_func, _1));
```

* Trigger and bind channel-events

```cpp
/* Trigger an event on channel */
Event event = channel1.trigger("users.create", jsonxx::Object("name", "Hans Mustermann"));
Event event = channel2.trigger("users.create", jsonxx::Object("name", "Frau Mustermann"), boost::bind(success_func, _1), boost::bind(failure_func, _1));

/* Channel event bind callback definition */
channel1.bind("users.pool", boost::bind(callback, _1));
```

* Callback additional parameters

```cpp
/* Define boost binds with additional parameters for the callback function */
boost::bind(bind_func_with_params, _1, param1, param2);
boost::bind(bind_func_with_params, _1, 123, true);
boost::bind(bind_func_with_params, _1, 123, param2);

```

* Other

To authenticate a user a separate C++ HTTP client library is required.


## TODO

* Add timestamp to events so that it is possible to clean the events queue with old events without response.
* Implement a clean-up method for the queue.
* See how to persist the connection object (```this->conn```) in the dispatcher so that the function ```trigger()``` can put the event trigger in event_queue if disconnected.


## Author

Egon Zemmer, Phlegx Systems - @phlegx


## License

```
The MIT License (http://opensource.org/licenses/MIT)

Copyright (C) 2014 Egon Zemmer, Phlegx Systems

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
```
