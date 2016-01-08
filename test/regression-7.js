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
var fs = require('fs');
var bl = require('bl');

describe('regression-#7', function() {
  it('should perform correctly', function(done) {
    var input = fs.createReadStream('test/random-large');
    var compressor = lzma.createCompressor({synchronous:true});
    
    input.pipe(compressor).pipe(bl(done));
  });
});
