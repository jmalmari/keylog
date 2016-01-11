# Key logging software for GNU/Linux kernel input event devices

## Build dependencies

- *sqlite3-dev* – SQLite3 database for key events

## Building

    cd <keylogdir>; mkdir build; cd build
    cmake .. -DCMAKE_MODULE_PATH=$(pwd)/../cmake
    make

## Runtime dependencies

- *libsqlite3* – SQLite3 database for key events

## Running

Run `./keylog` for usage. Reading `/dev/input/event*` may require
sudoing.

Database file `keylog.db` is created at current working directory.
Keep it in a safe place. It'll contain all of your passwords.
