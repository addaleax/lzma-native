/**
 * lzma-native - Node.js bindings for liblzma
 * Copyright (C) 2014-2016 Anna Henningsen <sqrt@entless.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/

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
  
  if (a.length !== b.length)
    return false;
  
  for (var i = 0; i < a.length; ++i) {
    if (a[i] !== b[i])
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
