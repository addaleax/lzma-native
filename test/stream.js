'use strict';

var assert = require('assert');
var fs = require('fs');
var bl = require('bl');
var helpers = require('./helpers.js');

var lzma = require('../');

describe('LZMAStream', function() {
	var random_data;

	function encodeAndDecode(enc, dec, done, data) {
		data = data || random_data;
		
		data.duplicate().pipe(enc).pipe(dec).pipe(bl(function(err, buf) {
			assert.ok(helpers.bufferEqual(data, buf));
			done(err);
		}));
	}

	before('read random test data', function(done) {
		random_data = bl(done);
		fs.createReadStream('test/random').pipe(random_data);
	});

	describe('#autoDecoder', function() {
		it('should be able to decode .lzma in async mode', function(done) {
			var stream = lzma.createStream('autoDecoder');
			stream.on('end', done);
			stream.on('data', function() {});
			
			if (lzma.asyncCodeAvailable)
				assert.ok(!stream.synchronous);
			
			fs.createReadStream('test/hamlet.txt.lzma').pipe(stream);
		});
		
		it('should be able to decode .xz in async mode', function(done) {
			var stream = lzma.createStream('autoDecoder');
			stream.on('end', done);
			stream.on('data', function() {});
			
			if (lzma.asyncCodeAvailable)
				assert.ok(!stream.synchronous);
			
			fs.createReadStream('test/hamlet.txt.xz').pipe(stream);
		});
		
		it('should be able to decode .lzma in sync mode', function(done) {
			var stream = lzma.createStream('autoDecoder', {synchronous: true});
			stream.on('end', done);
			stream.on('data', function() {});
			
			assert.ok(stream.synchronous);
			
			fs.createReadStream('test/hamlet.txt.lzma').pipe(stream);
		});
		
		it('should be able to decode .xz in sync mode', function(done) {
			var stream = lzma.createStream('autoDecoder', {synchronous: true});
			stream.on('end', done);
			stream.on('data', function() {});
			
			assert.ok(stream.synchronous);
			
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
			encodeAndDecode(enc, dec, done);
		});
		
		it('should be undone by aloneDecoder in async mode', function(done) {
			var enc = lzma.createStream('aloneEncoder');
			var dec = lzma.createStream('aloneDecoder');
			encodeAndDecode(enc, dec, done);
		});
		
		it('should be undone by autoDecoder in sync mode', function(done) {
			var enc = lzma.createStream('aloneEncoder', {synchronous: true});
			var dec = lzma.createStream('autoDecoder',  {synchronous: true});
			encodeAndDecode(enc, dec, done);
		});
		
		it('should be undone by aloneDecoder in sync mode', function(done) {
			var enc = lzma.createStream('aloneEncoder', {synchronous: true});
			var dec = lzma.createStream('aloneDecoder', {synchronous: true});
			encodeAndDecode(enc, dec, done);
		});
	});
	
	describe('#easyEncoder', function() {
		it('should be undone by autoDecoder in async mode', function(done) {
			var enc = lzma.createStream('easyEncoder');
			var dec = lzma.createStream('autoDecoder');
			encodeAndDecode(enc, dec, done);
		});
		
		it('should be undone by autoDecoder in sync mode', function(done) {
			var enc = lzma.createStream('easyEncoder', {synchronous: true});
			var dec = lzma.createStream('autoDecoder', {synchronous: true});
			encodeAndDecode(enc, dec, done);
		});
		
		it('should correctly encode the empty string', function(done) {
			var enc = lzma.createStream('easyEncoder', {synchronous: true});
			var dec = lzma.createStream('autoDecoder', {synchronous: true});
			encodeAndDecode(enc, dec, done, bl(''));
		});

		it('should be reasonably fast for one big chunk', function(done) {
			// “node createData.js | xz -9 > /dev/null” takes about 120ms for me.
			this.timeout(360); // three times as long as the above shell pipeline
			var outstream = new helpers.NullStream();
			outstream.on('finish', done);
			var enc = lzma.createStream('easyEncoder');
			enc.pipe(outstream);
			var x = 0, y = 0, str = '';
			for (var i = 0; i < 1000; ++i) {
				var data = {type: "position", x: x, y: y, i: i};
				str += JSON.stringify(data) + ",\n";
				x += (i * 101) % 307;
				y += (i * 211) % 307;
			}
			enc.end(str);
		});

		it('should be reasonably fast for many small chunks', function(done) {
			// “node createData.js | xz -9 > /dev/null” takes about 120ms for me.
			this.timeout(360); // three times as long as the above shell pipeline
			var outstream = new helpers.NullStream();
			outstream.on('finish', done);
			var enc = lzma.createStream('easyEncoder');
			enc.pipe(outstream);
			var x = 0, y = 0;
			for (var i = 0; i < 1000; ++i) {
				var data = {type: "position", x: x, y: y, i: i};
				enc.write(JSON.stringify(data) + ",\n");
				x += (i * 101) % 307;
				y += (i * 211) % 307;
			}
			enc.end();
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
				assert.ok(streams[i].synchronous);
				
			for (var i = 0; i < lzma.Stream.maxAsyncStreamCount * 2; ++i) 
				streams[i].end();
			
			done();
		});

		it('should work fine when synchronous streams are abandoned', function(done) {
			lzma.createStream({synchronous: true});
			
			done();
		});
	});
	
	describe('#memusage', function() {
		it('should return a meaningful value when decoding', function(done) {
			var stream = lzma.createStream('autoDecoder', {synchronous: true});
			stream.on('end', done);
			stream.on('data', function() {});
			
			fs.createReadStream('test/hamlet.txt.lzma').pipe(stream);
			assert.ok(stream.memusage() > 0);
		});
	});
	
	describe('#memlimitGet/#memlimitSet', function() {
		it('should set values of memory limits', function(done) {
			var stream = lzma.createStream('autoDecoder', {synchronous: true});
			stream.on('end', done);
			stream.on('data', function() {});
			
			assert.ok(stream.memlimitGet() > 0);
			stream.memlimitSet(1 << 30);
			assert.equal(stream.memlimitGet(), 1 << 30);
			fs.createReadStream('test/hamlet.txt.lzma').pipe(stream);
		});
	});
	
	describe('#totalIn/#totalOut', function() {
		it('should return meaningful values during the coding process', function(done) {
			var stream = lzma.createStream('autoDecoder', {synchronous: true});
			var valuesWereSet = false;
			
			stream.on('end', function() {
				assert(valuesWereSet);
				done()
			});
			
			stream.on('data', function() {
				valuesWereSet = valuesWereSet || stream.totalIn() > 0 && stream.totalOut() > 0;
			});
			
			fs.createReadStream('test/hamlet.txt.lzma').pipe(stream);
		});
	});
	
	after('should not have any open asynchronous streams', function() {
		assert.equal(lzma.Stream.curAsyncStreams.length, 0);
	});
});
