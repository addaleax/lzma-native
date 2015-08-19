var fs = require('fs');
var path = require('path');
var assert = require('assert');
var helpers = require('./helpers.js');

var lzma = require('../');

describe('Example code in README', function() {
	var snippets = [];
	var testSnippedRE = /<!--[^-]*runtest:\{((?!--)[^\}]+)\}[^-]*-->[^`]*```(?:js)?\n((?:[^`]|\n)+)```/mgi;
	
	var README = fs.readFileSync(path.join(__dirname, '../README.md'), 'utf-8');
	
	var match;
	while (match = testSnippedRE.exec(README))
		snippets.push({ name: match[1], code: match[2] });
	
	var header = (
		'(function() {\n' +
		'var console = { log: function() {} };\n' + 
		'var process = { \n' + 
		'	stdout: new helpers.NullStream(), \n' +
		'	stdin: fs.createReadStream("test/random") \n' +
		'};\n'
	);
	
	var footer = (
		'})();\n'
	);
	
	describe('Should correctly run all test scripts', function() {
		for (var i = 0; i < snippets.length; i++) { (function() {
			var code = snippets[i].code;
			
			code = code.replace(/var lzma = [^;]+;/g, '');
			
			it('Should run README test script: ' + snippets[i].name, function() {
				return eval(header + code + footer);
			});
		})(); }
	});
});
