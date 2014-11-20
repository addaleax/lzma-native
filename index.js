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
var _ = require('lodash');

_.extend(exports, native);

exports.version = '0.1.3';

var Stream = exports.Stream;

Stream.prototype.getStream = 
Stream.prototype.syncStream = function(options) {
	var nativeStream = this;
	
	var ret = function() {
		ret.super_.call(this, options);
		this.nativeStream = nativeStream;
		this.stype = 'synchronous';
	};
	
	util.inherits(ret, stream.Transform);
	
	ret.prototype._transform = function(chunk, encoding, callback) {
		try {
			this.nativeStream.code(chunk, _.bind(this.push, this));
		} catch (e) {
			this.push(null);
			this.emit('error', e);
		}
		
		callback();
	};
	
	ret.prototype._flush = function(callback) {
		this._transform(null, null, callback);
	}
	
	return new ret();
}

Stream.curAsyncStreamCount = 0;
Stream.maxAsyncStreamCount = 32;
Stream.prototype.asyncStream = native.asyncCodeAvailable ? function(options) {
	var endpoints = this.asyncCode_();
	if (!endpoints || endpoints.length != 2)
		throw Error('asyncStream() could not successfully call underlying asyncCode_()');
	
	var readStream  = fs.createReadStream (null, { fd: endpoints[0] });
	var writeStream = fs.createWriteStream(null, { fd: endpoints[1] });
	
	// keep the main lzma object alive during threaded compression
	readStream._lzma = this;
	
	Stream.curAsyncStreamCount++;
	var duplex = new simpleDuplex.SimpleDuplex(options, readStream, writeStream);
	readStream.on('end', function() {
		var lzma = readStream._lzma;
		readStream._lzma = null;
		Stream.curAsyncStreamCount--;
		
		try {
			lzma.checkError();
		} catch (e) {
			duplex.emit('error', e);
		}
	});
	
	duplex.stype = 'asynchronous';
	return duplex;
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

var createStream =
exports.createStream = function(coder, options) {
	if ((_.isObject(coder) || _.isNumber(options)) && !options) {
		options = coder;
		coder = null;
	}
	
	if (_.isNumber(options))
		options = {preset: options};
	
	coder = coder || 'easyEncoder';
	options = options || {};
	
	var stream = new Stream();
	_.bind(stream[coder], stream)(options);
	
	if (options.memlimit)
		stream.memlimitSet(options.memlimit);
	
	return options.synchronous || ((Stream.curAsyncStreamCount >= Stream.maxAsyncStreamCount) && !options.forceAsynchronous)
		? stream.syncStream() : stream.asyncStream();
};

exports.crc32 = function(input, encoding, presetCRC32) {
	if (typeof encoding == 'number') {
		encoding = null;
		presetCRC32 = encoding;
	}
	
	if (typeof input == 'string') 
		input = new Buffer(input, encoding);
	
	return exports.crc32_(input, presetCRC32 || 0);
};

/* compatibility: node-xz (https://github.com/robey/node-xz) */
exports.Compressor = function(preset, options) {
	options = _.clone(options || {});
	
	if (preset)
		options.preset = preset;
	
	return createStream('easyEncoder', options);
};

exports.Decompressor = function(options) {
	return createStream('autoDecoder', options);
};

/* compatibility: LZMA-JS (https://github.com/nmrugg/LZMA-JS) */
function singleStringCoding(stream, string, on_finish, on_progress) {
	on_progress = on_progress || function() {};
	on_finish = on_finish || function() {};
	
	var buffers = [];

	stream.on('data', function(b) { buffers.push(b); });
	stream.on('end', function() {
		on_progress(1);
		on_finish(Buffer.concat(buffers));
	});

	on_progress(0);
	
	// possibly our input is an array of byte integers
	// or a typed array
	string = new Buffer(string);
	
	stream.end(string);
}

exports.LZMA = function() {
	return {
		compress: function(string, mode, on_finish, on_progress) {
			var opt = {};

			if (_.isNumber(mode) && mode >= 1 && mode <= 9)
				opt.preset = mode;

			var stream = createStream('aloneEncoder', opt);
			
			return singleStringCoding(stream, string, on_finish, on_progress);
		},
		decompress: function(byte_array, on_finish, on_progress) {
			var stream = createStream('aloneDecoder');
			
			return singleStringCoding(stream, byte_array, on_finish, on_progress);
		}
	};
};

exports.compress = function(string, opt, on_finish) {
	var stream = createStream('easyEncoder', opt);
	return singleStringCoding(stream, string, on_finish);
};

exports.decompress = function(string, opt, on_finish) {
	var stream = createStream('autoDecoder', opt);
	return singleStringCoding(stream, string, on_finish);
};

})();
