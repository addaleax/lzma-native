(function() {
'use strict';

var stream = require('stream');
var util = require('util');
var _ = require('underscore');

function SimpleDuplex(options, readable, writable) {
	SimpleDuplex.super_.call(this, options);
	
	this.readable = readable;
	this.writable = writable;
	
	this.readable.on('end',      _.bind(function()    { this.push(null); }, this));
	this.readable.on('readable', _.bind(function()    { this.read(0); }, this));
	this.readable.on('error',    _.bind(function(err) { this.emit('error', err); }, this));
	this.writable.on('error',    _.bind(function(err) { this.emit('error', err); }, this));
	this.         on('finish',   _.bind(function()    { this.writable.end(); }, this));
}

util.inherits(SimpleDuplex, stream.Duplex);

SimpleDuplex.prototype._read = function(size) {
	var chunk = this.readable.read(size);
	this.push(chunk === null ? '' : chunk);
	return chunk;
};

SimpleDuplex.prototype._write = function(chunk, encoding, callback) {
	return this.writable.write(chunk, encoding, callback);
};

exports.SimpleDuplex = SimpleDuplex;

})();
