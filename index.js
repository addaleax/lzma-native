/**
 * lzma-native - Node.js bindings for liblzma
 * Copyright (C) 2014 Hauke Henningsen <sqrt@entless.org>
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

(function() {
'use strict';

var native = require('bindings')('lzma_native.node');
var stream = require('stream');
var util = require('util');
var fs = require('fs');
var simpleDuplex = require('./simpleDuplex');
var _ = require('underscore');

_.extend(exports, native);

var Stream = exports.Stream;

Stream.prototype.getStream = 
Stream.prototype.syncStream = function(options) {
	var nativeStream = this;
	
	var ret = function() {
		ret.super_.call(this, options);
		this.nativeStream = nativeStream;
	};
	
	util.inherits(ret, stream.Transform);
	
	ret.prototype._transform = function(chunk, encoding, callback) {
		this.nativeStream.code(chunk, _.bind(this.push, this));
		callback();
	};
	
	ret.prototype._flush = function(callback) {
		this.nativeStream.code(null, _.bind(this.push, this));
		callback();
	}
	
	return new ret();
}

Stream.prototype.asyncStream = native.asyncCodeAvailable ? function(options) {
	var endpoints = this.asyncCode_();
	if (!endpoints || endpoints.length != 2)
		throw Error('asyncStream() could not successfully call underlying asyncCode_()');
	
	var readStream  = fs.createReadStream (null, { fd: endpoints[0] });
	var writeStream = fs.createWriteStream(null, { fd: endpoints[1] });
	
	// keep the main lzma object alive during threaded compression
	readStream._lzma = this;
	readStream.on('end', function() { readStream._lzma = null; });
	
	return new simpleDuplex.SimpleDuplex(options, readStream, writeStream);
} : function() {
	// no native asyncCode, so we just use the synchronous method
	return this.getStream();
};

Stream.prototype.rawEncoder = function(options) {
	return this.rawEncoder_(options.filters || []);
};

Stream.prototype.rawDecoder = function(options) {
	return this.rawDecoder_(options.filters || []);
};

Stream.prototype.easyEncoder = function(options) {
	return this.easyEncoder_(options.preset || exports.PRESET_DEFAULT, options.check || exports.CHECK_CRC32);
};

Stream.prototype.streamEncoder = function(options) {
	return this.streamEncoder_(options.filters || [], options.check || exports.CHECK_CRC32);
};

Stream.prototype.streamDecoder = function(options) {
	return this.streamDecoder_(options.memlimit || null, options.flags || 0);
};

Stream.prototype.autoDecoder = function(options) {
	return this.autoDecoder_(options.memlimit || null, options.flags || 0);
};

Stream.prototype.aloneDecoder = function(options) {
	return this.aloneDecoder_(options.memlimit || null);
};

exports.createStream = function(coder, options) {
	coder = coder || 'easyEncoder';
	options = options || {};
	
	var stream = new Stream();
	_.bind(stream[coder], stream)(options);
	
	if (options.memlimit)
		stream.memlimitSet(options.memlimit);
	
	return options.synchronous ? stream.syncStream() : stream.asyncStream();
};

})();
