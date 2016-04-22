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
});

describe('lzma.compress()/decompress() with Q promises', function() {
  var oldPromiseAPI;
  before('set Promise API to Q',  function() { oldPromiseAPI = lzma.setPromiseAPI(require('q')); });
  after('re-enable old Promise API', function() { lzma.setPromiseAPI(oldPromiseAPI); });
  
  it('can round-trip', function() {
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

describe('lzma.compress()/decompress() with ES6 Promises', function() {
  if (typeof Promise !== 'function')
    return; // sorry, nothing to test here
  
  var oldPromiseAPI;
  before('set Promise API to Q',  function() { oldPromiseAPI = lzma.setPromiseAPI(Promise); });
  after('re-enable old Promise API', function() { lzma.setPromiseAPI(oldPromiseAPI); });
  
  it('can round-trip', function() {
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
  var oldPromiseAPI;
  before('set Promise API to Q',  function() { oldPromiseAPI = lzma.setPromiseAPI(null); });
  after('re-enable old Promise API', function() { lzma.setPromiseAPI(oldPromiseAPI); });
  
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

describe('lzma.setPromiseAPI', function() {
  it('Should set or return the currently used API', function() {
    var fakeAPI = {v: 1};
    var old = lzma.setPromiseAPI(fakeAPI);
    
    assert.strictEqual(fakeAPI, lzma.setPromiseAPI());
    assert.strictEqual(fakeAPI, lzma.setPromiseAPI());
    
    var reset = lzma.setPromiseAPI(old);
    assert.notStrictEqual(fakeAPI, lzma.setPromiseAPI());
    assert.strictEqual(fakeAPI, reset);
    
    lzma.setPromiseAPI('default');
    assert.strictEqual(old, lzma.setPromiseAPI());
  });
});
