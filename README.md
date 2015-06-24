# WebsocketRailsClient++ (v0.7.4)

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

 * ```onOpen(boost::bind cb)```  : callback on open connection.
 * ```onClose(boost::bind cb)``` : callback on close connection.
 * ```onFail(boost::bind cb)```  : callback on fail connection.

#### Trigger an Event on Server

* ```trigger(std::string event_name, jsonxx::Object event_data)``` : trigger event with data without callback.
* ```trigger(std::string event_name, jsonxx::Object event_data, boost::bind cb_succ, boost::bind cb_fail)``` : trigger event with data and callbacks.

#### Bind to an Incoming Event

* ```bind(std::string event_name, boost::bind cb)``` : Bind to an event name with callback.

#### Unbind an Incoming Event

* ```unbindAll(std::string event_name)``` : Unbind all callbacks on a specific event name.

### Channel Event Dispatcher

#### Channel Management

* ```getChannel(std::string channel_name)``` : Get a channel after subscribed to it.
* ```subscribe(std::string channel_name)``` : Subscribe to a channel.
* ```subscribePrivate(std::string channel_name, boost::bind cb_succ, boost::bind cb_fail)``` : Subscribe to a private channel with callbacks.
* ```unsubscribe(std::string channel_name)``` : Unsubscribe a channel.
* ```unsubscribe(std::string channel_name, boost::bind cb_succ, boost::bind cb_fail)``` : Unsubscribe a private channel with callbacks.

#### Trigger a Channel-Event on Server

* ```trigger(std::string event_name, jsonxx::Object event_data)``` : trigger channel event with data without callback.

#### Bind to an Incoming Channel-Event

* ```bind(std::string event_name, boost::bind cb)``` : Bind to a channel event with callback.

#### Unbind an Incoming Channel-Event

* ```unbindAll(std::string event_name)``` : Unbind all callbacks on a specific event name.

## Compile

### C++ Linker

#### Linker Flag -l

* **Boost libraries:** boost_system, boost_thread
* **System libraries:** pthread, rt
* **TSL libraries:** ssl, crypto

### C++ Compiler

#### Compiler Flag -D

* ```-D_WEBSOCKETPP_CPP11_STL_```

#### Compiler Flag -I

* ```-I"/path/to/includes"```

#### Compiler Flag -std

* ```-std=c++11```


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

* Bind and unbind events to callbacks

```cpp
/* Event bind callback definition */
dispatcher.bind("users_pool", boost::bind(callback, _1));

/* Event bind callback definition with member function callback */
Foo my_foo;
dispatcher.bind("users_pool", boost::bind(&Foo::success_func, my_foo, _1));

/* Event unbind callback definition */
dispatcher.unbindAll("users_pool");
```

* Trigger events

```cpp
/* Trigger an event */
dispatcher.trigger("users_create", jsonxx::Object("name", "Hans Mustermann"));
dispatcher.trigger("users_create", jsonxx::Object("name", "Frau Mustermann"), boost::bind(success_func, _1), boost::bind(failure_func, _1));

/* Trigger an event with member function callback */
Bar my_bar;
dispatcher.trigger("users_pool", boost::bind(&Foo::success_func, my_bar, _1), boost::bind(&Foo::failure_func, my_bar, _1));
```

* Use channels

```cpp
/* Subscribe to channels */
dispatcher.subscribe("Authors");
dispatcher.subscribe("Users", boost::bind(success_func, _1), boost::bind(failure_func, _1));
dispatcher.subscribePrivate("Administrators");
dispatcher.subscribePrivate("Moderators", boost::bind(success_func, _1), boost::bind(failure_func, _1));

/* Unsubscribe channels */
dispatcher.unsubscribe("Authors");
dispatcher.unsubscribe("Users", boost::bind(success_func, _1), boost::bind(failure_func, _1));
```

* Trigger, bind and unbind channel-events

```cpp
/* Trigger an event on channel */
dispatcher.getChannel("Authors").trigger("users_create", jsonxx::Object("name", "Hans Mustermann"));

/* Channel event bind callback definition */
dispatcher.getChannel("Authors").bind("users_pool", boost::bind(callback, _1));

/* Event unbind callback definition */
dispatcher.getChannel("Authors").unbindAll("users_pool");
```

* Callback additional parameters

```cpp
/* Define boost binds with additional parameters for the callback function */
boost::bind(bind_func_with_params, _1, param1, param2);
boost::bind(bind_func_with_params, _1, 123, true);
boost::bind(bind_func_with_params, _1, 123, param2);

```

## Other

* To authenticate a user a separate C++ HTTP client library is required.


## TODO

* Add timestamp to events so that it is possible to clean the events queue with old events without response.
* Implement a clean-up method for the queue.


## Author

Egon Zemmer, Phlegx Systems - @phlegx


## License

```
The MIT License (http://opensource.org/licenses/MIT)

Copyright (C) 2015 Egon Zemmer, Phlegx Systems

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
```

