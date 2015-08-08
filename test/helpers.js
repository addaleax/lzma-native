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
	if (a.length != b.length)
		return false;
	
	for (var i = 0; i < a.length; ++i)
		if (a[i] != b[i])
			return false;
	
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
