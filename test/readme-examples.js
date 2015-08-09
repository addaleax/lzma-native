var fs = require('fs');
var path = require('path');
var assert = require('assert');

var lzma = require('../');

describe('Example code in README', function() {
	var snippets = [];
	var testSnippedRE = /<!--[^-]*runtest[^-]*-->[^`]*```(?:js)?\n((?:[^`]|\n)+)```/mgi;
	
	var README = fs.readFileSync(path.join(__dirname, '../README.md'), 'utf-8');
	
	var match;
	while (match = testSnippedRE.exec(README))
		snippets.push(match[1]);
	
	describe('Should correctly run all test scripts', function() {
		for (var i = 0; i < snippets.length; i++) {
			it('Should run test script ' + i, function() {
				return eval(snippets[i]);
			});
		}
	});
});
