# KVlite

Lightweight in-memory key-value store written in pure C with network access, console and graphical interfaces.

## Project Structure


KVlite/
└── src/
    ├── client/
    │   └── main.c
    ├── core/
    │   ├── kvstore.c
    │   └── kvstore.h
    ├── gui/
    │   └── KVliteGUI.cpp
    ├── server/
    │   └── server.c
    ├── tools/
    │   └── kvlite_client.c
    └── CMakeLists.txt


## Features

- In-memory data storage as key-value pairs
- TCP server for network access (port 6379)
- Console clients: local and remote
- Graphical client on WinAPI
- Cyrillic support in Windows-1251 encoding
- Data save and load to disk

## Building

**Requirements:** Windows, Visual Studio 2022 or newer, CMake 3.14+

### Building in Visual Studio
1. Open the project folder in Visual Studio as a CMake project
2. Wait for CMake configuration to complete
3. Select the desired build configuration (Debug/Release)
4. Build the project through the Build menu

## Usage

### Starting the Server

kvlite_server.exe

The server will start at 127.0.0.1:6379 and wait for client connections.

### Local Console Client

kvlite_cli.exe

Launches an interactive console with `KVlite>` prompt.

### Remote Console Client

kvlite_client.exe SET key_name value
kvlite_client.exe GET key_name

To connect to a different server:

kvlite_client.exe -h 192.168.1.10 -p 6379 GET key


### Graphical Client
Launch `kvlite_gui.exe`, specify the server host and port, enter commands in the text field and click "Send" or press Enter.

## Supported Commands

- **SET key value** - Set value for key
- **GET key** - Get value by key
- **DELETE key** - Delete key and its value
- **EXISTS key** - Check if key exists
- **INCR key** - Increment numeric value by 1
- **DECR key** - Decrement numeric value by 1
- **APPEND key value** - Append string to existing value
- **RENAME old_key new_key** - Rename key
- **GETSET key new_value** - Return old value and set new one
- **KEYS** - Get list of all keys
- **FLUSHDB** - Clear entire database
- **INFO** - Get server information
- **SAVE filename** - Save data to file
- **LOAD filename** - Load data from file
- **PING** - Test server connection
- **ECHO message** - Echo request
- **QUIT** - Close server connection

## Architecture

The project uses a modular architecture:
- **core** - storage core implemented in pure C
- **server** - TCP server for handling network connections
- **client** - local console client
- **tools** - remote console client
- **gui** - graphical interface on WinAPI

Data is stored in RAM as a dynamic array of key-value pairs. When SAVE command is executed, data is serialized to a text file in "key=value" format.

## Limitations

- Server supports only one simultaneous connection
- Maximum value length is limited to 1023 characters
- No authentication or data encryption
- Project is intended for use in local networks

## Future Plans

- Multi-threading or asynchronous connections support
- TTL (time-to-live) for keys
- Encryption
- Configuration file support
- Operation logging system

## License

Project is distributed under MIT license. Detailed information is available in the LICENSE file.
