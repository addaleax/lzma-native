'use strict';

var lzma = require('../');
var fs = require('fs');
var assert = require('assert');

describe('regression-#53', function() {
  it('should perform correctly', function(done) {
    var input = fs.readFileSync('test/invalid.xz');

    lzma.decompress(input, function(result, error) {
      assert.strictEqual(result, null);
      assert.strictEqual(error.name, 'LZMA_DATA_ERROR');
      done();
    });
  });
});
