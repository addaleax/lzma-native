'use strict';

var lzma = require('../');
var assert = require('assert');

// this test relies on mocha’s 2000 ms test timeout
describe('regression-#1', function() {
	it('should perform correctly', function(done) {
		var complete = 0;
		var N = 4;
		
		for (var i = 0; i < N; ++i) {
			lzma.compress("", function() {
				if (++complete == N)
					done();
			});
		}
	});
});
