'use strict';

var lzma = require('../');

// this test relies on mocha’s 2000 ms test timeout (only in case it fails, of course)
describe('regression-#1', function() {
  it('should perform correctly', function(done) {
    var complete = 0;
    var N = 4;
    
    for (var i = 0; i < N; ++i) {
      lzma.compress("", function() { // jshint ignore:line
        if (++complete === N)
          done();
      }); // jshint ignore:line
    }
  });
});
