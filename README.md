lzma-native
===========

[![Build Status][1]][2]

Provides Node.js bindings to the native liblzma library

## Example usage:

The simplest possible usage:
```js
var lzma = require('lzma-native');
var encoder = lzma.createStream();

process.stdin.pipe(encoder).pipe(process.stdout);
```

This mimicks the functionality of the `xz` command line util.
Equivalently, one could have written

```js
var encoder = lzma.createStream('easyEncoder', {preset: lzma.PRESET_DEFAULT, check: lzma.CHECK_CRC32});
```

or, for stronger and slower compression:
```js
var encoder = lzma.createStream('easyEncoder', {preset: 9});
```

Here `easyEncoder` corresponds to the `xz` command line util, resp. its file format [.xz](https://en.wikipedia.org/wiki/.xz).
For the older `.lzma` format, you can just use `aloneEncoder` instead.

The API is loosely based on the native API, with a few bits of wrapper code added for convenience.
Methods like `stream.code` and `lzma.crc32` accept Node.js `Buffer`s as arguments.

Unless you set `.synchronous = true` in `createStream`s second parameter, the library will use its
own thread for compression (if compiled with support for that).

The `encoder` object here is an instance of `stream.Duplex` (see the [Node.js docs](http://nodejs.org/api/stream.html)),
so you could also manually perform any of the write and read operations that you’re familiar with on it.

## List of encoders/decoders and options

Encoders and decoders you *probably* are interested in:
* `easyEncoder`: Creates `.xz` files. Supports `.preset` and `.check` options.
* `aloneEncoder`: Creates `.lzma` files. Supports `.preset` and a bunch of very specific options (see the liblzma C headers for details)
* `autoDecoder`: Supports various flags. Detects input type automatically.

That is, the following is essentially (quite a slow version of) `cat`:

```js
var encoder = lzma.createStream('easyEncoder');
var decoder = lzma.createStream('autoDecoder');

process.stdin.pipe(encoder).pipe(decoder).pipe(process.stdout);
```

If you know specifically what you want, you may also look into these encoders:
* `rawDecoder`: Supports `.filters`.
* `rawEncoder`: Supports `.filters`.
* `streamEncoder`: Supports `.filters` and `.check`.
* `streamDecoder`: Supports various flags.
* `aloneDecoder`: Supports various flags.

Also, all encoders accept a `.memlimit` option.

## Installation

This package includes the native C library, so there is no need to install it separately.

You may want to use a somewhat recent C++ compiler, and asynchronous
compression support requires `std::thread`, which is included in C++11
(but is not essential for compiling).

## Licensing

The original C library package contains code under various licenses,
with its core (liblzma) being public domain. See its contents for details.
This wrapper is licensed under the LGPL 3 or any later version of the LGPL.

[1]: https://travis-ci.org/addaleax/lzma-native.png
[2]: https://travis-ci.org/addaleax/lzma-native
