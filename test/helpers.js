'use strict';

var fs = require('fs');
var util = require('util');
var stream = require('readable-stream');

function fsCreateWriteStream(filename) {
  var s = fs.createWriteStream(filename);
  if (process.version.match(/^v0.8/))
    s.on('close', function() { s.emit('finish'); });
  return s;
}

function bufferEqual(a, b) {
  /* The bl module does not expose array indexing for its instances,
   * however, Buffer.get is deprecated and will be removed.
   * (See https://github.com/nodejs/io.js/blob/60a974d200/lib/buffer.js#L425)
   * => All incoming objects will be coerced to Buffer */
  if (!Buffer.isBuffer(a))
    a = a.slice();
  
  if (!Buffer.isBuffer(b))
    b = b.slice();
  
  if (a.length != b.length)
    return false;
  
  for (var i = 0; i < a.length; ++i) {
    if (a[i] != b[i])
      return false;
  }
  
  return true;
}

function NullStream(options) {
  stream.Writable.call(this, options);
}
util.inherits(NullStream, stream.Writable);
NullStream.prototype._write = function(chunk, encoding, callback) {
  callback();
};

exports.fsCreateWriteStream = fsCreateWriteStream;
exports.bufferEqual = bufferEqual;
exports.NullStream = NullStream;
