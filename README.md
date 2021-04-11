# lu9 -- Lua for Plan 9 space

lu9 is an umbrella project providing a native Plan 9 port of the Lua
library and several other libraries and programs built on top of it.

The project is currently comprised of the following:

* [lu9](https://git.sr.ht/~kvik/lu9) is a simple Lua interpreter
  host intended for running standalone Lua programs and interactive
  usage. It also serves as an example on how to use the Lua library
  in your own programs.
* [lu9-lua](https://git.sr.ht/~kvik/lu9-lua) is a Lua 5.4 library
  patched to work well within the Plan 9 environment. It does not
  depend on any other code in this project.
  Unlike many previous ports it does *NOT* use APE, the POSIX
  emulation layer, which means that you can use it alongside regular
  Plan 9 libraries.
* [lu9-p9](https://git.sr.ht/~kvik/lu9-p9) is a system interface
  module providing access to system calls and other useful libc
  interfaces. It also provides some higher-level interfaces for
  the most common tasks. 
* [lu9-lpeg](https://git.sr.ht/~kvik/lu9-lpeg) is a simple port
  of the LPeg pattern matching module.
  
Check the respective repositories for more information.
  
## lu9(1) standalone interpreter

### Building and installing

Building and installing lu9 is simple: grab this repository,
pull the dependencies, and run `mk install`.

	; git/clone https://git.sr.ht/~kvik/lu9
	; mk pull
	; mk install

Only the lu9 binary and Lua modules get installed into the system.
The binary is statically linked so no C libraries have to be installed.

### Usage

Check the [lu9(1) manual page](http://a-b.xyz/man/1/lu9) for more
information.

	; man lu9

## Legal

All of my original work is released under the MIT license, the same
license used by Lua and LPeg.
