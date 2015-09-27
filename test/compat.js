/**
 * lzma-native - Node.js bindings for liblzma
 * Copyright (C) 2014-2015 Hauke Henningsen <sqrt@entless.org>
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
var fs = require('fs');
var helpers = require('./helpers.js');

var lzma = require('../');

describe('Compressor/Decompressor', function() {
	it('can compress', function(done) {
		var c = new lzma.Compressor();
		
		c.on('finish', done);
		c.end('Hello!');
	});
	
	it('takes preset and options arguments', function(done) {
		var c = new lzma.Compressor(7, {synchronous: true});
		
		c.on('finish', done);
		c.end('Bananas');
	});
	
	it('can round-trip', function(done) {
		var enc = new lzma.Compressor();
		var dec = new lzma.Decompressor();
		var outfile = 'test/random.lzma.unlzma';
		var outstream = helpers.fsCreateWriteStream(outfile);
		
		outstream.on('finish', function() {
			assert.ok(helpers.bufferEqual(fs.readFileSync('test/random'), fs.readFileSync(outfile)));
			fs.unlink(outfile);
			done();
		});
		
		fs.createReadStream('test/random').pipe(enc).pipe(dec).pipe(outstream);
	});
});

describe('LZMA.compress()/decompress()', function() {
	it('can compress strings to Buffers', function(done) {
		var LZMA = new lzma.LZMA();
		
		LZMA.compress('Banana', 9, function(result) {
			assert.ok(Buffer.isBuffer(result));
			assert.ok(result.length > 0);
			
			done();
		});
	});
	
	it('can decompress integer arrays', function(done) {
		var LZMA = new lzma.LZMA();
		
		LZMA.decompress(
			[0x5d, 0x00, 0x00, 0x80, 0x00, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x21, 0x18,
			 0x49, 0xc6, 0x24, 0x17, 0x18, 0x93, 0x42, 0x5f,  0xff, 0xfd, 0xa2, 0xd0, 0x00], function(result) {
			assert.ok(Buffer.isBuffer(result));
			assert.equal(result.toString(), 'Banana');
			
			done();
		});
	});
	
	it('can decompress typed integer arrays', function(done) {
		var LZMA = new lzma.LZMA();
		
		LZMA.decompress(
			new Uint8Array(
			[0x5d, 0x00, 0x00, 0x80, 0x00, 0xff, 0xff, 0xff,  0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x21, 0x18,
			 0x49, 0xc6, 0x24, 0x17, 0x18, 0x93, 0x42, 0x5f,  0xff, 0xfd, 0xa2, 0xd0, 0x00]), function(result) {
			assert.ok(Buffer.isBuffer(result));
			assert.equal(result.toString(), 'Banana');
			
			done();
		});
	});
	
	it('can round-trip', function(done) {
		var LZMA = new lzma.LZMA();
		
		LZMA.compress('Bananas', 9, function(result) {
			assert.equal(result.toString('base64'), 'XQAAAAT//////////wAhGEnQgnOEP++//7v9AAA=');
			LZMA.decompress(result, function(result) {
				assert.ok(Buffer.isBuffer(result));
				assert.equal(result.toString(), 'Bananas');
				
				done();
			});
		});
	});
	
	it('can round-trip, even for compressed data which uses LZMA2', function(done) {
		var LZMA = new lzma.LZMA();
		
		lzma.compress('Bananas', function(result) {
			LZMA.decompress(result, function(result) {
				assert.equal(result.toString(), 'Bananas');
				
				done();
			});
		});
	});
});

var BananasCompressed = '/Td6WFoAAAFpIt42AgAhARwAAAAQz1jMAQAGQmFuYW5hcwAA0aJr3wABGwcS69QXkEKZDQEAAAAAAVla';
describe('lzma.compress()/decompress()', function() {
	it('can round-trip', function(done) {
		lzma.compress('Bananas', 9, function(result) {
			assert.equal(result.toString('base64'), BananasCompressed);
			lzma.decompress(result, function(result) {
				assert.ok(Buffer.isBuffer(result));
				assert.equal(result.toString(), 'Bananas');
				
				done();
			});
		});
	});
	
	it('can round-trip with Q promises enabled', function() {
		return lzma.compress('Bananas', 9).then(function(result) {
			assert.equal(result.toString('base64'), BananasCompressed);
			return lzma.decompress(result);
		}).then(function(result) {
			assert.ok(Buffer.isBuffer(result));
			assert.equal(result.toString(), 'Bananas');
		});
	});
	
	it('fails for invalid input', function() {
		return lzma.decompress('ABC').then(function(result) {
			assert.ok(false); // never get here due to error
		}).catch(function(err) {
			assert.ok(err);
		});
	});
});

describe('lzma.compress()/decompress() without promises', function() {
	var oldQ;
	before('disable Q',  function() { oldQ = lzma._setQ(null); });
	after('re-enable Q', function() { lzma._setQ(oldQ); });
	
	it('can round-trip', function(done) {
		lzma.compress('Bananas', 9, function(result) {
			assert.equal(result.toString('base64'), BananasCompressed);
			lzma.decompress(result, function(result) {
				assert.ok(Buffer.isBuffer(result));
				assert.equal(result.toString(), 'Bananas');
				
				done();
			});
		});
	});
	
	it('fails for invalid input', function(done) {
		lzma.decompress('ABC', function(result, err) {
			assert.strictEqual(result, null);
			assert.ok(err);
			done();
		});
	});
});
