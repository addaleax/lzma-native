'use strict';

var assert = require('assert');
var fs = require('fs');
var util = require('util');
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
      fs.unlinkSync(outfile);
      done();
    });
    
    fs.createReadStream('test/random').pipe(enc).pipe(dec).pipe(outstream);
  });
});

describe('LZMA.compress()/decompress()', function() {
  it('can compress strings to Buffers', function(done) {
    var LZMA = new lzma.LZMA();
    
    LZMA.compress('Banana', 6, function(result) {
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
    
    LZMA.compress('Bananas', 5, function(result) {
      assert.equal(result.toString('base64'), 'XQAAgAD//////////wAhGEnQgnOEP++//7v9AAA=');
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

var BananasCompressed = '/Td6WFoAAAFpIt42AgAhARYAAAB0L+WjAQAGQmFuYW5hcwAA0aJr3wABGwcS69QXkEKZDQEAAAAAAVla';
describe('lzma.compress()/decompress()', function() {
  it('can round-trip', function(done) {
    lzma.compress('Bananas', 5, function(result) {
      assert.equal(result.toString('base64'), BananasCompressed);
      lzma.decompress(result, function(result) {
        assert.ok(Buffer.isBuffer(result));
        assert.equal(result.toString(), 'Bananas');
        
        done();
      });
    });
  });
});


describe('lzma.compress()/decompress() with ES6 Promises', function() {
  assert(typeof Promise === 'function');
  
  it('can round-trip', function() {
    return lzma.compress('Bananas', 5).then(function(result) {
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

describe('lzma.compress()/decompress() with util.promisify()', function() {
  var majorVersion = process.version.match(/^v(\d+)\./);
  if (majorVersion && +majorVersion[1] < 8) {
    return;
  }

  var compress = util.promisify(lzma.compress);
  var decompress = util.promisify(lzma.decompress);

  it('can round-trip', function() {
    return compress('Bananas', 5).then(function(result) {
      assert.equal(result.toString('base64'), BananasCompressed);
      return decompress(result);
    }).then(function(result) {
      assert.ok(Buffer.isBuffer(result));
      assert.equal(result.toString(), 'Bananas');
    });
  });
  
  it('fails for invalid input', function() {
    return decompress('ABC').then(function(result) {
      assert.ok(false); // never get here due to error
    }).catch(function(err) {
      assert.ok(err);
    });
  });
});
