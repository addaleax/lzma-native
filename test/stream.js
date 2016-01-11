/**
 * lzma-native - Node.js bindings for liblzma
 * Copyright (C) 2014-2016 Anna Henningsen <sqrt@entless.org>
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
var bl = require('bl');
var helpers = require('./helpers.js');

var lzma = require('../');

describe('LZMAStream', function() {
  var random_data, x86BinaryData, hamlet, largeRandom;

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
  
  before('read large random test data', function(done) {
    largeRandom = bl(done);
    fs.createReadStream('test/random-large').pipe(largeRandom);
  });
  
  before('read hamlet.txt test data', function(done) {
    hamlet = bl(done);
    fs.createReadStream('test/hamlet.txt.xz').pipe(lzma.createDecompressor()).pipe(hamlet);
  });
  
  before('read an executable file', function(done) {
    /* process.execPath is e.g. /usr/bin/node
     * it does not matter for functionality testing whether
     * this is actually x86 code, only for compression ratio */
    x86BinaryData = bl(function() {
      x86BinaryData = bl(x86BinaryData.slice(0, 20480));
      return done();
    });
    
    fs.createReadStream(process.execPath).pipe(x86BinaryData);
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
    [
      {value: lzma.PRESET_EXTREME, name: 'e' },
      {value: 0, name: '' },
    ].map(function(presetFlag) {
    [
      1, 3, 4, 6, 7, 9
    ].map(function(preset) { // test only some presets
    [
      { file: hamlet, name: 'Hamlet' },
      { file: random_data, name: 'random test data' },
      { file: largeRandom, name: 'large random test data' },
      { file: x86BinaryData, name: 'x86 binary data' }
    ].map(function(entry) {
    [
      { synchronous: true,  name: 'sync' },
      { synchronous: false, name: 'async' },
    ].map(function(syncInfo) {
      var info = 'with ' + entry.name + ', preset = ' + preset + presetFlag.name;
      it('should be undone by autoDecoder in ' + syncInfo.name + ' mode ' + info, function(done) {
        var enc = lzma.createStream('easyEncoder', {
          preset: preset | presetFlag.value,
          synchronous: syncInfo.synchronous
        });
        
        var dec = lzma.createStream('autoDecoder');
        
        encodeAndDecode(enc, dec, done, entry.file);
      });
    });
    });
    });
    });
    
    it('should correctly encode the empty string in async mode', function(done) {
      var enc = lzma.createStream('easyEncoder');
      var dec = lzma.createStream('autoDecoder');
      encodeAndDecode(enc, dec, done, bl(''));
    });
    
    it('should correctly encode the empty string in sync mode', function(done) {
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
  
  describe('#streamEncoder', function() {
    it('should be undone by autoDecoder in async mode using the x86 filter', function(done) {
      var enc = lzma.createStream('streamEncoder', {
        filters: [
          { id: lzma.FILTER_X86 },
          { id: lzma.FILTER_LZMA2 }
        ],
        check: lzma.CHECK_SHA256
      });
      var dec = lzma.createStream('autoDecoder');
      
      encodeAndDecode(enc, dec, done, x86BinaryData);
    });
    
    it('should be undone by autoDecoder in sync mode using the x86 filter', function(done) {
      var enc = lzma.createStream('streamEncoder', {
        filters: [
          { id: lzma.FILTER_X86 },
          { id: lzma.FILTER_LZMA2 }
        ],
        check: lzma.CHECK_SHA256,
        synchronous: true
      });
      var dec = lzma.createStream('autoDecoder', {synchronous: true});
      
      encodeAndDecode(enc, dec, done, x86BinaryData);
    });
    
    it('should be undone by streamDecoder in async mode using the delta filter', function(done) {
      var enc = lzma.createStream('streamEncoder', {
        filters: [
          { id: lzma.FILTER_DELTA, options: { dist: 2 } },
          { id: lzma.FILTER_LZMA2 }
        ],
        check: lzma.CHECK_SHA256
      });
      var dec = lzma.createStream('streamDecoder');
      
      encodeAndDecode(enc, dec, done, x86BinaryData);
    });
    
    it('should be undone by streamDecoder in sync mode using the delta filter', function(done) {
      var enc = lzma.createStream('streamEncoder', {
        filters: [
          { id: lzma.FILTER_DELTA, options: { dist: 2 } },
          { id: lzma.FILTER_LZMA2 }
        ],
        check: lzma.CHECK_SHA256,
        synchronous: true
      });
      var dec = lzma.createStream('streamDecoder', {synchronous: true});
      
      encodeAndDecode(enc, dec, done, x86BinaryData);
    });
    
    it('should fail for an invalid combination of filter objects', function() {
      assert.throws(function() {
        lzma.createStream('streamEncoder', {
          filters: [
            {id: lzma.FILTER_LZMA2},
            {id: lzma.FILTER_X86}
          ]
        });
      });
    });
    
    it('should fail for filters which do not expect options', function() {
      assert.throws(function() {
        lzma.createStream('streamEncoder', {
          filters: [
            { id: lzma.FILTER_X86, options: { Banana: 'Banana' } },
            { id: lzma.FILTER_LZMA2 }
          ]
        });
      });
    });
  });
  
  describe('#streamDecoder', function() {
    it('should accept an memlimit argument', function() {
      var memlimit = 20 << 20; /* 20 MB */
      var s = lzma.createStream('streamDecoder', { memlimit: memlimit, synchronous: true });
      
      assert.strictEqual(s.memlimitGet(), memlimit);
    });
    
    it('should fail when the memlimit argument is invalid', function() {
      assert.throws(function() {
        lzma.createStream('streamDecoder', { memlimit: 'ABC' });
      });
    });
  });
  
  describe('#rawEncoder', function() {
    var rawFilters = [
      { id: lzma.FILTER_X86 },
      { id: lzma.FILTER_LZMA2, options: { dictSize: 1 << 24 /* 16 MB */ } }
    ];
    
    it('should be undone by rawDecoder in async mode', function(done) {
      var enc = lzma.createStream('rawEncoder', { filters: rawFilters });
      var dec = lzma.createStream('rawDecoder', { filters: rawFilters });
      
      encodeAndDecode(enc, dec, done);
    });
    
    it('should be undone by rawDecoder in sync mode', function(done) {
      var enc = lzma.createStream('rawEncoder', { filters: rawFilters, synchronous: true });
      var dec = lzma.createStream('rawDecoder', { filters: rawFilters, synchronous: true });
      
      encodeAndDecode(enc, dec, done);
    });
  });

  describe('#createStream', function() {
    it('should switch to synchronous streams after too many Stream creations', function(done) {
      assert.ok(lzma.Stream.maxAsyncStreamCount);
      lzma.Stream.maxAsyncStreamCount = 3;
      
      var streams = [], i;
      
      for (i = 0; i < lzma.Stream.maxAsyncStreamCount * 2; ++i) 
        streams.push(lzma.createStream({synchronous: false}));
      
      for (i = lzma.Stream.maxAsyncStreamCount + 1; i < lzma.Stream.maxAsyncStreamCount * 2; ++i)
        assert.ok(streams[i].synchronous);
        
      for (i = 0; i < lzma.Stream.maxAsyncStreamCount * 2; ++i) 
        streams[i].end();
      
      done();
    });

    it('should work fine when synchronous streams are abandoned', function(done) {
      lzma.createStream({synchronous: true});
      
      done();
    });
    
    it('should return streams which emit `finish` and `end` events', function(done) {
      var s = lzma.createStream();
      var finished = false;
      var ended    = false;
      
      var maybeDone = function() {
        if (finished && ended)
          done();
      };
      
      s.on('finish', function() { finished = true; maybeDone(); });
      s.on('end',    function() { ended    = true; maybeDone(); });
      s.on('data', function() {});
      
      s.end();
    });
    
    it('should return errors with .code, .name and .desc properties', function(done) {
      try {
        lzma.createStream({check: lzma.CHECK_ID_MAX + 1});
      } catch (e) {
        assert.ok(e.name);
        assert.ok(e.desc);
        
        done();
      }
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
    
    it('should return null when encoding', function() {
      var stream = lzma.createCompressor({synchronous: true});
      
      assert.strictEqual(stream.memusage(), null);
    });
    
    it('should fail when called with null or {} as the this object', function() {
      var stream = lzma.createStream('autoDecoder', {synchronous: true});
      assert.throws(stream.nativeStream.memusage.bind(null));
      assert.throws(stream.nativeStream.memusage.bind({}));
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
    
    it('should fail for invalid memory limit specifications', function() {
      var stream = lzma.createStream('autoDecoder', {synchronous: true});
      
      // use undefined because that’s never converted to Number
      assert.throws(function() { stream.memlimitSet(undefined); });
    });
  });
  
  describe('#totalIn/#totalOut', function() {
    it('should return meaningful values during the coding process', function(done) {
      var stream = lzma.createStream('autoDecoder', {synchronous: true});
      var valuesWereSet = false;
      
      stream.on('end', function() {
        assert(valuesWereSet);
        done();
      });
      
      stream.on('data', function() {
        valuesWereSet = valuesWereSet || stream.totalIn() > 0 && stream.totalOut() > 0;
      });
      
      fs.createReadStream('test/hamlet.txt.lzma').pipe(stream);
    });
  });
  
  describe('bufsize', function() {
    it('Should only accept positive integers', function() {
      var stream = new lzma.createStream({synchronous: true});
      
      assert.throws(function() {
        stream.bufsize = 'Not numeric';
      }, /bufsize must be a positive number/);
      
      assert.throws(function() {
        stream.bufsize = 0;
      }, /bufsize must be a positive number/);
      
      assert.throws(function() {
        stream.bufsize = -65536;
      }, /bufsize must be a positive number/);
    });
    
    it('Should default to 64k', function() {
      var stream = new lzma.createStream({synchronous: true});
      
      assert.strictEqual(stream.bufsize, 65536);
    });
    
    it('Should accept values from options', function() {
      var stream = new lzma.createStream({synchronous: true, bufsize: 16384});
      
      assert.strictEqual(stream.bufsize, 16384);
    });
    
    it('Should be overridable', function() {
      var stream = new lzma.createStream({synchronous: true});
      
      stream.bufsize = 8192;
      assert.strictEqual(stream.bufsize, 8192);
    });
  });
  
  after('should not have any open asynchronous streams', function(done) {
    if (typeof gc === 'function')
      gc();
      
    setTimeout(function() {
      assert.equal(lzma.Stream.curAsyncStreams.length, 0);
      done();
    }, 500);
  });
});
