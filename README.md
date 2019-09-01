##  Tgbook: Telegram bot for interacting with calibre, goodreads and lazylibrarian.

Tgbook is telegram bot wrap around [Tgbot](https://github.com/reo7sp/tgbot-cpp) and it's capable of searching and retrieving books in calibre, adding book in goodreads in lazylibrarian for downloading and much more to come!

### Building

#### Requirements
- [Cpr](https://github.com/whoshuu/cpr) library for handling http request.
- [nlohmann/json](https://github.com/nlohmann/json) for fast JSON parsing.
- [Boost library](https://www.boost.org/doc/libs/1_71_0/more/getting_started/unix-variants.html).

To build tgbot, firstly you need to install some dependencies such as Boost and build tools such as CMake. On Debian-based distibutives you can do it with these commands:

```
sudo apt-get install g++ make binutils cmake libssl-dev libboost-system-dev zlib1g-dev
```

#### Compilation
If you have Cmake, compile with:
```bash
cmake .
make -j4
```

