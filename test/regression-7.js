'use strict';

var lzma = require('../');
var fs = require('fs');
var bl = require('bl');

describe('regression-#7', function() {
  it('should perform correctly', function(done) {
    var input = fs.createReadStream('test/random-large');
    var compressor = lzma.createCompressor({synchronous:true});
    
    input.pipe(compressor).pipe(bl(done));
  });
});
