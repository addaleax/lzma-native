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
	});

	describe('crc32_', function() {
		it('Should fail when non-numeric previous values are supplied', function() {
			assert.throws(function() {
				return lzma.crc32_('Banana', 'Not numeric');
			});
		});
	});
});
