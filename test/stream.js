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
			
			if (lzma.asyncCodeAvailable)
				assert.equal(stream.stype, 'asynchronous');
			
			fs.createReadStream('test/hamlet.txt.lzma').pipe(stream);
		});
		
		it('should be able to decode .xz in async mode', function(done) {
			var stream = lzma.createStream('autoDecoder');
			stream.on('end', done);
			stream.on('data', function() {});
			
			if (lzma.asyncCodeAvailable)
				assert.equal(stream.stype, 'asynchronous');
			
			fs.createReadStream('test/hamlet.txt.xz').pipe(stream);
		});
		
		it('should be able to decode .lzma in sync mode', function(done) {
			var stream = lzma.createStream('autoDecoder', {synchronous: true});
			stream.on('end', done);
			stream.on('data', function() {});
			
			assert.equal(stream.stype, 'synchronous');
			
			fs.createReadStream('test/hamlet.txt.lzma').pipe(stream);
		});
		
		it('should be able to decode .xz in sync mode', function(done) {
			var stream = lzma.createStream('autoDecoder', {synchronous: true});
			stream.on('end', done);
			stream.on('data', function() {});
			
			assert.equal(stream.stype, 'synchronous');
			
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
		it('should be undone by autoDecoder in async mode', function(done) {
			var enc = lzma.createStream('aloneEncoder');
			var dec = lzma.createStream('autoDecoder');
			var outfile = 'test/random.lzma.unauto';
			var outstream = fs.createWriteStream(outfile);
			
			outstream.on('finish', function() {
				assert.ok(bufferEqual(fs.readFileSync('test/random'), fs.readFileSync(outfile)));
				fs.unlink(outfile);
				done();
			});
			
			fs.createReadStream('test/random').pipe(enc).pipe(dec).pipe(outstream);
		});
		
		it('should be undone by aloneDecoder in async mode', function(done) {
			var enc = lzma.createStream('aloneEncoder');
			var dec = lzma.createStream('aloneDecoder');
			var outfile = 'test/random.lzma.unlzma';
			var outstream = fs.createWriteStream(outfile);
			
			outstream.on('finish', function() {
				assert.ok(bufferEqual(fs.readFileSync('test/random'), fs.readFileSync(outfile)));
				fs.unlink(outfile);
				done();
			});

			fs.createReadStream('test/random').pipe(enc).pipe(dec).pipe(outstream);
		});
		
		it('should be undone by autoDecoder in sync mode', function(done) {
			var enc = lzma.createStream('aloneEncoder', {synchronous: true});
			var dec = lzma.createStream('autoDecoder', {synchronous: true});
			var outfile = 'test/random.lzma.unauto';
			var outstream = fs.createWriteStream(outfile);
			
			outstream.on('finish', function() {
				assert.ok(bufferEqual(fs.readFileSync('test/random'), fs.readFileSync(outfile)));
				fs.unlink(outfile);
				done();
			});
			
			fs.createReadStream('test/random').pipe(enc).pipe(dec).pipe(outstream);
		});
		
		it('should be undone by aloneDecoder in sync mode', function(done) {
			var enc = lzma.createStream('aloneEncoder', {synchronous: true});
			var dec = lzma.createStream('aloneDecoder', {synchronous: true});
			var outfile = 'test/random.lzma.unlzma';
			var outstream = fs.createWriteStream(outfile);
			
			outstream.on('finish', function() {
				assert.ok(bufferEqual(fs.readFileSync('test/random'), fs.readFileSync(outfile)));
				fs.unlink(outfile);
				done();
			});

			fs.createReadStream('test/random').pipe(enc).pipe(dec).pipe(outstream);
		});
	});
	
	describe('#easyEncoder', function() {
		it('should be undone by autoDecoder in async mode', function(done) {
			var enc = lzma.createStream('easyEncoder');
			var dec = lzma.createStream('autoDecoder');
			var outfile = 'test/random.xz.unauto';
			var outstream = fs.createWriteStream(outfile);
			
			outstream.on('finish', function() {
				assert.ok(bufferEqual(fs.readFileSync('test/random'), fs.readFileSync(outfile)));
				fs.unlink(outfile);
				done();
			});
			
			fs.createReadStream('test/random').pipe(enc).pipe(dec).pipe(outstream);
		});
	});
	
	describe('#easyEncoder', function() {
		it('should be undone by autoDecoder in sync mode', function(done) {
			var enc = lzma.createStream('easyEncoder', {synchronous: true});
			var dec = lzma.createStream('autoDecoder', {synchronous: true});
			var outfile = 'test/random.xz.unauto';
			var outstream = fs.createWriteStream(outfile);
			
			outstream.on('finish', function() {
				assert.ok(bufferEqual(fs.readFileSync('test/random'), fs.readFileSync(outfile)));
				fs.unlink(outfile);
				done();
			});
			
			fs.createReadStream('test/random').pipe(enc).pipe(dec).pipe(outstream);
		});
	});
	
	describe('#createStream', function() {
		it('should switch to synchronous streams after too many Stream creations', function(done) {
			assert.ok(lzma.Stream.maxAsyncStreamCount);
			lzma.Stream.maxAsyncStreamCount = 3;
			
			var streams = [];
			for (var i = 0; i < lzma.Stream.maxAsyncStreamCount * 2; ++i) 
				streams.push(lzma.createStream({synchronous: false}));
			
			for (var i = lzma.Stream.maxAsyncStreamCount + 1; i < lzma.Stream.maxAsyncStreamCount * 2; ++i)
				assert.equal(streams[i].stype, 'synchronous');
				
			for (var i = 0; i < lzma.Stream.maxAsyncStreamCount * 2; ++i) 
				streams[i].end();
			
			done();
		});
	});
	
	describe('#createStream', function() {
		it('should work fine when synchronous streams are abandoned', function(done) {
			lzma.createStream({synchronous: true});
			
			done();
		});
	});
});
