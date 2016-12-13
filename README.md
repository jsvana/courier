# Courier

`Courier` is a work-in-progress, curses-based IMAP client. Right now it only supports the user sending raw IMAP commands in the provided console. Hopefully at some point it will have a nicer interface to browse emails.

## Usage

    $ courier [config_file]

This will only connect over SSL.

The default config file location is `~/.config/courier/courier.conf`.

The config file has an example [here](config.example).

## Motivation

I wanted to see how weird the IMAP protocol was, so I started poking around on this project. Turns out it makes sense but is a bit of a pain to write by hand.

## Installation

Currently you'll need a compiler with C++1z support (I'm using G++ 6.2.1). This also only supports Linux.

    $ bfg9000 configure build
    $ cd build
    $ ninja
    $ ./courier [config_file]

## License

[MIT](LICENSE)
