(function() {
'use strict';

var native = require('bindings')('lzma_native.node');
var stream = require('stream');
var util = require('util');
var _ = require('underscore');

_.extend(exports, native);

exports.Stream.prototype.getStream = function(options) {
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

})();
