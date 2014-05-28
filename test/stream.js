'use strict';

var assert = require('assert');
var fs = require('fs');

var lzma = require('../');

function bufferEqual(a, b) {
	if (a.length != b.length)
		return false;
	
	for (var i = 0; i < a.length; ++i)
		if (a[i] != b[i])
			return false;
	
	return true;
}

describe('LZMAStream', function() {
	describe('#autoDecoder', function() {
		it('should be able to decode .lzma in async mode', function(done) {
			var stream = lzma.createStream('autoDecoder');
			stream.on('end', done);
			stream.on('data', function() {});
			
			fs.createReadStream('test/hamlet.txt.lzma').pipe(stream);
		});
		
		it('should be able to decode .xz in async mode', function(done) {
			var stream = lzma.createStream('autoDecoder');
			stream.on('end', done);
			stream.on('data', function() {});
			
			fs.createReadStream('test/hamlet.txt.xz').pipe(stream);
		});
		
		it('should be able to decode .lzma in sync mode', function(done) {
			var stream = lzma.createStream('autoDecoder', {synchronous: true});
			stream.on('end', done);
			stream.on('data', function() {});
			
			fs.createReadStream('test/hamlet.txt.lzma').pipe(stream);
		});
		
		it('should be able to decode .xz in sync mode', function(done) {
			var stream = lzma.createStream('autoDecoder', {synchronous: true});
			stream.on('end', done);
			stream.on('data', function() {});
			
			fs.createReadStream('test/hamlet.txt.xz').pipe(stream);
		});
		
		it('should bark loudly when given non-decodable data in async mode', function(done) {
			var stream = lzma.createStream('autoDecoder');
			var sawError = false;
			
			stream.on('error', function() { sawError = true; });
			stream.on('end', function() {
				assert.ok(sawError);
				done();
			});
			stream.on('data', function() {});
			
			fs.createReadStream('test/random').pipe(stream);
		});
		
		it('should bark loudly when given non-decodable data in sync mode', function(done) {
			var stream = lzma.createStream('autoDecoder', {synchronous: true});
			var sawError = false;
			
			stream.on('error', function() { sawError = true; });
			stream.on('end', function() {
				assert.ok(sawError);
				done();
			});
			stream.on('data', function() {});
			
			fs.createReadStream('test/random').pipe(stream);
		});
	});
	
	describe('#aloneEncoder', function() {
		it('should be undone by autoDecoder', function(done) {
			var enc = lzma.createStream('aloneEncoder');
			var dec = lzma.createStream('autoDecoder');
			var outfile = 'test/random.lzma.unauto';
			
			dec.on('end', function() {
				assert.ok(bufferEqual(fs.readFileSync('test/random'), fs.readFileSync(outfile)));
				fs.unlink(outfile);
				done();
			});
			
			fs.createReadStream('test/random').pipe(enc).pipe(dec).pipe(fs.createWriteStream(outfile));
		});
		
		it('should be undone by aloneDecoder', function(done) {
			var enc = lzma.createStream('aloneEncoder');
			var dec = lzma.createStream('aloneDecoder');
			var outfile = 'test/random.lzma.unlzma';
			
			dec.on('end', function() {
				assert.ok(bufferEqual(fs.readFileSync('test/random'), fs.readFileSync(outfile)));
				fs.unlink(outfile);
				done();
			});

			fs.createReadStream('test/random').pipe(enc).pipe(dec).pipe(fs.createWriteStream(outfile));
		});
	});
	
	describe('#easyEncoder', function() {
		it('should be undone by autoDecoder', function(done) {
			var enc = lzma.createStream('easyEncoder');
			var dec = lzma.createStream('autoDecoder');
			var outfile = 'test/random.xz.unauto';
			
			dec.on('end', function() {
				assert.ok(bufferEqual(fs.readFileSync('test/random'), fs.readFileSync(outfile)));
				fs.unlink(outfile);
				done();
			});
			
			fs.createReadStream('test/random').pipe(enc).pipe(dec).pipe(fs.createWriteStream(outfile));
		});
	});
});
