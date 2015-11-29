/**
 * lzma-native - Node.js bindings for liblzma
 * Copyright (C) 2014-2015 Anna Henningsen <sqrt@entless.org>
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

var lzma = require('../');
var assert = require('assert');

// this test relies on mocha’s 2000 ms test timeout (only in case it fails, of course)
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
