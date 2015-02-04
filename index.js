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
var stream = require('readable-stream');
var util = require('util');
var _ = require('lodash');

_.extend(exports, native);

exports.version = '0.2.7';

var Stream = exports.Stream;

Stream.curAsyncStreams = [];
Stream.maxAsyncStreamCount = 32;

Stream.prototype.getStream = 
Stream.prototype.asyncStream =
Stream.prototype.syncStream = function(options) {
	options = options || {};
	
	var nativeStream = this;

	var _forceNextTickCb = function() {
		/* I know this looks like “magic/more magic”, but
		 * apparently works around a bogus process.nextTick in
		 * node v0.11. This probably does not affect real
		 * applications which perform other I/O than LZMA compression. */
		setTimeout(function() {}, 1);
	};
	
	var ret = function() {
		ret.super_.call(this, options);
		var self = this;
		
		self.nativeStream = nativeStream;
		self.synchronous = (options.synchronous || !native.asyncCodeAvailable) ? true : false;
		self.chunkCallbacks = [];
		
		if (!self.synchronous) {
			Stream.curAsyncStreams.push(self);
			var cleanup = function() {
				var index = Stream.curAsyncStreams.indexOf(self);
				
				if (index != -1)
					Stream.curAsyncStreams.splice(index, 1);
				
				self.nativeStream = null;
			}
			
			self.once('finish', cleanup);
			self.once('error',  cleanup);
		}
		
		self.nativeStream.bufferHandler = function(buf, shouldInvokeChunkCallbacks, err) {
			process.nextTick(function() {
				if (err) {
					self.push(null);
					self.emit('error', err);
					_forceNextTickCb();
				}
				
				if (shouldInvokeChunkCallbacks) {
					// rotate the chunkCallbacks property, since more callbacks
					// may be added by the current ones
					var chunkCallbacks = self.chunkCallbacks;
					self.chunkCallbacks = [];
					
					while (chunkCallbacks.length > 0)
						chunkCallbacks.shift()();
					
					_forceNextTickCb();
				} else {
					self.push(buf);
				}
			});
			
			_forceNextTickCb();
		};
		
		// add all methods from the native Stream
		_.each(native.Stream.prototype, function(i, key) {
			self[key] = function() { return self.nativeStream[key].apply(self.nativeStream, arguments); };
		});
	};
	
	util.inherits(ret, stream.Transform);
	
	ret.prototype._transform = function(chunk, encoding, callback) {
		this.chunkCallbacks.push(callback);
		
		try {
			this.nativeStream.code(chunk, !this.synchronous);
		} catch (e) {
			this.emit('error', e);
		}
	};
	
	ret.prototype._flush = function(callback) {
		this._transform(null, null, callback);
	};
	
	return new ret();
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
	
	if (!options.synchronous)
		options.synchronous = ((Stream.curAsyncStreams.length >= Stream.maxAsyncStreamCount) && !options.forceAsynchronous);
	
	return stream.getStream(options);
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
	stream.once('end', function() {
		on_progress(1);
		on_finish(Buffer.concat(buffers));
	});

	on_progress(0);
	
	// possibly our input is an array of byte integers
	// or a typed array
	if (!Buffer.isBuffer(string))
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
	if (typeof opt == 'function') {
		on_finish = opt;
		opt = {};
	}
	
	var stream = createStream('easyEncoder', opt);
	return singleStringCoding(stream, string, on_finish);
};

exports.decompress = function(string, opt, on_finish) {
	if (typeof opt == 'function') {
		on_finish = opt;
		opt = {};
	}
	
	var stream = createStream('autoDecoder', opt);
	return singleStringCoding(stream, string, on_finish);
};

})();
