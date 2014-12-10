lzma-native
===========

[![Build Status](https://travis-ci.org/addaleax/lzma-native.png)](https://travis-ci.org/addaleax/lzma-native)
[![Dependency Status](https://david-dm.org/addaleax/lzma-native.svg)](https://david-dm.org/addaleax/lzma-native)
[![devDependency Status](https://david-dm.org/addaleax/lzma-native/dev-status.svg)](https://david-dm.org/addaleax/lzma-native#info=devDependencies)

Provides Node.js bindings to the native liblzma library (.xz file format, among others)

## Example usage

If you don’t have any fancy requirements, using this library is really simple:
```js
var lzma = require('lzma-native');
var encoder = lzma.createStream();

process.stdin.pipe(encoder).pipe(process.stdout);
```

So just call `lzma.createStream` and you’ll get a stream where you can pipe your
input in and read your compressed output from. You can decode `.xz` files by passing
`'autoDecoder'` as a parameter to `createStream`.
If you don’t need anything but simple streams, that’s it!

## API

Apart from the API described here, `lzma-native` implements the APIs of the following
other LZMA libraries so you can use it nearly as a drop-in replacement:

* [node-xz](https://github.com/robey/node-xz) via `lzma.Compressor` and `lzma.Decompressor`
* [LZMA-JS](https://github.com/nmrugg/LZMA-JS) via `lzma.LZMA().compress` and `lzma.LZMA().decompress`,
  though without actual support for progress functions and returning `Buffer` objects
  instead of integer arrays. (This produces output in the `.lzma` file format, *not* the `.xz` format!)

The above example code mimicks the functionality of the `xz` command line util (i. e. 
reads input from `stdin` and writes compressed data to `stdout`).
Equivalently, one could have written

```js
var encoder = lzma.createStream('easyEncoder', {preset: lzma.PRESET_DEFAULT, check: lzma.CHECK_CRC32});
```

or, for stronger and slower compression (`preset` corresponds to compression strengths from 1 to 9):
```js
var encoder = lzma.createStream('easyEncoder', {preset: 9});
```

Here `easyEncoder` corresponds to the `xz` command line util, resp. its file format [.xz](https://en.wikipedia.org/wiki/.xz).
For the older `.lzma` format, you can just use `aloneEncoder` instead. The decoder can automatically tell
between these file formats.

You can also use non-streaming functions for convenience, akin to the [LZMA-JS](https://github.com/nmrugg/LZMA-JS) API:

* `lzma.compress(string, [options], on_finish)`, where `on_finish` will be called with a Buffer in the `.xz` format
* `lzma.decompress(buffer, [options], on_finish)`, where `on_finish` will be called with a string.

The functions API is loosely based on the native API, with a few bits of wrapper code added for convenience.
Methods like `lzma.crc32` accept Node.js `Buffer`s as arguments.

Unless you set `.synchronous = true` in `createStream`’s `options` parameter, the library will use its
own thread for compression.

The `encoder` object here is an instance of `stream.Duplex` (see the [Node.js docs](http://nodejs.org/api/stream.html)),
so you could also manually perform any of the write and read operations that you’re familiar with on it.

## List of encoders/decoders and options

Encoders and decoders you are *probably* interested in:
* `easyEncoder`: Creates `.xz` files. Supports `.preset` and `.check` options.
* `aloneEncoder`: Creates `.lzma` files. Supports `.preset` and a bunch of very specific options (see the liblzma C headers for details)
* `autoDecoder`: Supports various flags. Detects input type automatically.

That is, the following is essentially (a relatively slow version of) `cat`:

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

## Licensing

The original C library package contains code under various licenses,
with its core (liblzma) being public domain. See its contents for details.
This wrapper is licensed under the LGPL 3 or any later version of the LGPL.

## Related projects

Other implementations of the LZMA algorithms for node.js and/or web clients include:

* [lzma-purejs](https://github.com/cscott/lzma-purejs)
* [LZMA-JS](https://github.com/nmrugg/LZMA-JS)
* [node-xz](https://github.com/robey/node-xz)
* [node-liblzma](https://github.com/oorabona/node-liblzma)

Note that LZMA has been designed to have much faster decompression than
compression, which is something you may want to take into account when
choosing an compression algorithm for large files. Almost always, LZMA achieves
higher compression ratios than other algorithms, though.

## Acknowledgements

This project is financially supported by the [Tradity project](https://tradity.de/) and
is an [Entless](https://entless.org/) project.
