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

var stream = require('stream');
var util = require('util');
var _ = require('lodash');

function SimpleDuplex(options, readable, writable) {
	SimpleDuplex.super_.call(this, options);
	
	this.source = readable;
	this.sink   = writable;
	
	this.source.on('end',      _.bind(function()    { this.push(null); }, this));
	this.source.on('readable', _.bind(function()    { this.read(0); }, this));
	this.source.on('error',    _.bind(function(err) { this.emit('error', err); }, this));
	this.sink  .on('error',    _.bind(function(err) { this.emit('error', err); }, this));
	this.       on('finish',   _.bind(function()  { this.sink.end(); }, this));
}

util.inherits(SimpleDuplex, stream.Duplex);

SimpleDuplex.prototype._read = function(size) {
	var chunk = this.source.read(size);
	this.push(chunk === null ? '' : chunk);
	return chunk;
};

SimpleDuplex.prototype._write = function(chunk, encoding, callback) {
	return this.sink.write(chunk, encoding, callback);
};

exports.SimpleDuplex = SimpleDuplex;

})();
