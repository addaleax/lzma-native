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

/*jshint unused:false */

'use strict';

var fs = require('fs');
var path = require('path');
var assert = require('assert');
var helpers = require('./helpers.js');

var lzma = require('../');

/*
 * This test module looks for code snippets of a certain format in the README
 * file for this repository and runs them (possibly modified to be better
 * suited for testing).
 * 
 * It greps the README file for code blocks enclosed in "```" which are
 * preceded by HTML comments of the approximate format
 *     <!-- runtest:{Test description text} -->
 * and replaces certain references to standard library objects
 * (e.g. process.stdin or console.log), then runs the test code.
 */

describe('Example code in README', function() {
  var snippets = [];
  var testSnippedRE = /<!--[^-]*runtest:\{((?!--)[^\}]+)\}[^-]*-->[^`]*```(?:js)?\n((?:[^`]|\n)+)```/mgi;
  
  var README = fs.readFileSync(path.join(__dirname, '../README.md'), 'utf-8');
  
  var match;
  while (match = testSnippedRE.exec(README))
    snippets.push({ name: match[1], code: match[2] });
  
  var identity = function(v) { return v; };
  
  describe('Should correctly run all test scripts', function() {
    for (var i = 0; i < snippets.length; i++) { (function() { // jshint ignore:line
      var code = snippets[i].code;
      
      code = code.replace(/var lzma = [^;]+;/mg, '');
      code = code.replace(/console\.(log|error|warn|trace)/g, 'identity');
      code = code.replace(/process\.stdin/g, 'fs.createReadStream("test/random")');
      code = code.replace(/process\.stdout/g, '(new helpers.NullStream())');
      
      it('Should run README test script: ' + snippets[i].name, function() {
        return eval(code); // jshint ignore:line
      });
    })(); } // jshint ignore:line
  });
});
