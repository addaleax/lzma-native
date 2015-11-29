/**
 * lzma-native - Node.js bindings for liblzma
 * Copyright (C) 2014-2015 Anna Henningsen <sqrt@entless.org>
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

var assert = require('assert');

var lzma = require('../');

describe('lzma-native internals', function() {
	describe('#code', function() {
		it('should fail for non-buffer input', function() {
			var stream = lzma.createStream('autoDecoder', {synchronous: true});
			
			stream.nativeStream.bufferHandler = function() {};
			assert.throws(function() { stream.nativeStream.code('I am not a Buffer object'); });
		});
	});
	
	describe('new/constructor', function() {
		it('can be called with `new`', function() {
			var stream = new lzma.Stream({synchronous: true});
			assert.ok(stream.code);
		});
		
		it('can be called without `new`', function() {
			var stream = lzma.Stream({synchronous: true});
			assert.ok(stream.code);
		});
	});
	
	describe('crc32_', function() {
		it('Should fail when non-numeric previous values are supplied', function() {
			assert.throws(function() {
				return lzma.crc32_('Banana', 'Not numeric');
			});
		});
	});
});
