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
