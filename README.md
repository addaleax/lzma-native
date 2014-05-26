lzma-native
===========

Provides bindings to the native liblzma library

===========

Example usage:

```js
var lzma = require('lzma-native');
var stream = new lzma.Stream();
stream.easyEncoder({preset: 6, check: lzma.CHECK_CRC32});

var input = fs.createReadStream(...);
var output = fs.createWriteStream(...);

input.pipe(stream.getStream()).pipe(output);
```

The API is loosely based on the native API, with the `stream.getStream()` method added for convenience.
Methods like `stream.code` and `lzma.crc32` accept Node.js `Buffer`s as arguments.

The package requires that you have the corresponding C library installed, e. g. via
`sudo apt-get install liblzma-dev` or your equivalent of that.
