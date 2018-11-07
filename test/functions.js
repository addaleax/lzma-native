'use strict';

var assert = require('assert');
var fs = require('fs');

var lzma = require('../');

describe('lzma', function() {
  describe('#versionNumber', function() {
    it('should be present and of number type', function() {
      assert.ok(lzma.versionNumber());
      assert.equal(typeof lzma.versionNumber(), 'number');
    });
  });

  describe('#versionString', function() {
    it('should be present and of string type', function() {
      assert.ok(lzma.versionNumber());
      assert.equal(typeof lzma.versionString(), 'string');
    });
  });

  describe('#checkIsSupported', function() {
    it('should at least support no check and crc32', function() {
      assert.strictEqual(true, lzma.checkIsSupported(lzma.CHECK_NONE));
      assert.strictEqual(true, lzma.checkIsSupported(lzma.CHECK_CRC32));
    });
    it('should return false for non-existing checks', function() {
      // -1 would be thee bitwise or of all possible checks
      assert.strictEqual(false, lzma.checkIsSupported(-1));
    });
  });

  describe('#checkSize', function() {
    it('should be zero for CHECK_NONE', function() {
      assert.strictEqual(0, lzma.checkSize(lzma.CHECK_NONE));
    });

    it('should be non-zero for crc32', function() {
      assert.ok(lzma.checkSize(lzma.CHECK_CRC32) > 0);
    });

    it('should be monotonous', function() {
      assert.ok(lzma.checkSize(lzma.CHECK_CRC32 | lzma.CHECK_SHA256) >= lzma.checkSize(lzma.CHECK_CRC32));
    });

    it('should be strictly monotonous if SHA256 is supported', function() {
      assert.ok(lzma.checkSize(lzma.CHECK_CRC32 | lzma.CHECK_SHA256) > lzma.checkSize(lzma.CHECK_CRC32) ||
        !lzma.checkIsSupported(lzma.CHECK_SHA256));
    });
  });

  var exampleSentenceWords = ['The ', 'quick ', 'brown ', 'fox ', 'jumps ', 'over ', 'the ', 'lazy ', 'dog'];
  var exampleSentence = exampleSentenceWords.join('');

  describe('#crc32', function() {
    it('should be the standard CRC32 value for a few strings', function() {
      assert.strictEqual(0x00000000, lzma.crc32(''));
      assert.strictEqual(0x414fa339, lzma.crc32(exampleSentence));
      assert.strictEqual(0x414fa339, lzma.crc32(Buffer.from(exampleSentence)));
      assert.strictEqual(0xafabd35e, lzma.crc32('crc32'));
    });

    it('should allow cumulative calculation of the checksum', function() {
      var crc32Rev = function(prev, cur) {
        return lzma.crc32(cur, prev);
      };

      assert.strictEqual(lzma.crc32(exampleSentence),
      exampleSentenceWords.reduce(crc32Rev, 0));
    });

    it('should fail if the input type is not obvious', function() {
      assert.throws(function() {
        lzma.crc32({some: 'object'});
      });
    });
  });

  describe('#filterEncoderIsSupported', function() {
    it('should return true for LZMA1, LZMA2', function() {
      assert.strictEqual(true, lzma.filterEncoderIsSupported(lzma.FILTER_LZMA1));
      assert.strictEqual(true, lzma.filterEncoderIsSupported(lzma.FILTER_LZMA2));
    });

    it('should return false for VLI_UNKNOWN', function() {
      assert.strictEqual(false, lzma.filterEncoderIsSupported(lzma.VLI_UNKNOWN));
    });

    it('should throw for objects which are not convertible to string', function() {
      var badObject = { toString: function() { throw Error('badObject.toString()'); } };
      assert.throws(function() { lzma.filterEncoderIsSupported(badObject); });
    });
  });

  describe('#filterDecoderIsSupported', function() {
    it('should return true for LZMA1, LZMA2', function() {
      assert.strictEqual(true, lzma.filterDecoderIsSupported(lzma.FILTER_LZMA1));
      assert.strictEqual(true, lzma.filterDecoderIsSupported(lzma.FILTER_LZMA2));
    });

    it('should return false for VLI_UNKNOWN', function() {
      assert.strictEqual(false, lzma.filterDecoderIsSupported(lzma.VLI_UNKNOWN));
    });
  });

  describe('#mfIsSupported', function() {
    it('should return true for MF_HC4', function() {
      assert.strictEqual(true, lzma.mfIsSupported(lzma.MF_HC4));
    });

    it('should return true for a wrong value', function() {
      assert.strictEqual(false, lzma.mfIsSupported(-1));
    });
  });

  describe('#modeIsSupported', function() {
    it('should return true for LZMA_MODE_FAST', function() {
      assert.strictEqual(true, lzma.modeIsSupported(lzma.MODE_FAST));
    });

    it('should return true for a wrong value', function() {
      assert.strictEqual(false, lzma.modeIsSupported(-1));
    });
  });

  describe('#lzmaFilterEncoderIsSupported', function() {
    it('should return true for and only for encoding-related filters', function() {
      assert.strictEqual(false, lzma.filterEncoderIsSupported());
      assert.strictEqual(false, lzma.filterEncoderIsSupported(null));
      assert.strictEqual(false, lzma.filterEncoderIsSupported(''));
      assert.strictEqual(false, lzma.filterEncoderIsSupported(lzma.LZMA_VLI_UNKNOWN));
      assert.strictEqual(true,  lzma.filterEncoderIsSupported(lzma.FILTER_LZMA1));
      assert.strictEqual(true,  lzma.filterEncoderIsSupported(lzma.FILTER_LZMA2));
      assert.strictEqual(true,  lzma.filterEncoderIsSupported(lzma.FILTERS_MAX));
      assert.strictEqual(false, lzma.filterEncoderIsSupported(lzma.FILTERS_MAX+1));

      assert.strictEqual('boolean', typeof lzma.filterEncoderIsSupported(lzma.FILTER_POWERPC));
      assert.strictEqual('boolean', typeof lzma.filterEncoderIsSupported(lzma.FILTER_IA64));
      assert.strictEqual('boolean', typeof lzma.filterEncoderIsSupported(lzma.FILTER_ARM));
      assert.strictEqual('boolean', typeof lzma.filterEncoderIsSupported(lzma.FILTER_ARMTHUMB));
      assert.strictEqual('boolean', typeof lzma.filterEncoderIsSupported(lzma.FILTER_SPARC));
      assert.strictEqual('boolean', typeof lzma.filterEncoderIsSupported(lzma.FILTER_DELTA));
    });
  });

  describe('#filterDecoderIsSupported', function() {
    it('should return true for and only for encoding-related filters', function() {
      assert.strictEqual(false, lzma.filterDecoderIsSupported());
      assert.strictEqual(false, lzma.filterDecoderIsSupported(null));
      assert.strictEqual(false, lzma.filterDecoderIsSupported(''));
      assert.strictEqual(false, lzma.filterDecoderIsSupported(lzma.LZMA_VLI_UNKNOWN));
      assert.strictEqual(true,  lzma.filterDecoderIsSupported(lzma.FILTER_LZMA1));
      assert.strictEqual(true,  lzma.filterDecoderIsSupported(lzma.FILTER_LZMA2));
      assert.strictEqual(true,  lzma.filterDecoderIsSupported(lzma.FILTERS_MAX));
      assert.strictEqual(false, lzma.filterDecoderIsSupported(lzma.FILTERS_MAX+1));

      assert.strictEqual('boolean', typeof lzma.filterDecoderIsSupported(lzma.FILTER_POWERPC));
      assert.strictEqual('boolean', typeof lzma.filterDecoderIsSupported(lzma.FILTER_IA64));
      assert.strictEqual('boolean', typeof lzma.filterDecoderIsSupported(lzma.FILTER_ARM));
      assert.strictEqual('boolean', typeof lzma.filterDecoderIsSupported(lzma.FILTER_ARMTHUMB));
      assert.strictEqual('boolean', typeof lzma.filterDecoderIsSupported(lzma.FILTER_SPARC));
      assert.strictEqual('boolean', typeof lzma.filterDecoderIsSupported(lzma.FILTER_DELTA));
    });
  });

  describe('#rawEncoderMemusage', function() {
    it('should be positive for LZMA1, LZMA2', function() {
      assert.ok(lzma.rawEncoderMemusage([{id: lzma.FILTER_LZMA1}]) > 0);
      assert.ok(lzma.rawEncoderMemusage([{id: lzma.FILTER_LZMA2}]) > 0);
    });

    it('should return null for VLI_UNKNOWN', function() {
      assert.strictEqual(null, lzma.rawEncoderMemusage([{id: lzma.VLI_UNKNOWN}]));
    });

    it('should be monotonous in the preset parameter', function() {
      for (var i = 1; i < 9; ++i)
        assert.ok(lzma.rawEncoderMemusage([{id: lzma.FILTER_LZMA2, preset: i+1}]) >=
          lzma.rawEncoderMemusage([{id: lzma.FILTER_LZMA2, preset: i}]));
    });

    it('should fail if input is not an array of filter objects', function() {
      assert.throws(function() { lzma.rawEncoderMemusage(null); });
      assert.throws(function() { lzma.rawEncoderMemusage([null]); });
    });
  });

  describe('#rawDecoderMemusage', function() {
    it('should be positive for LZMA1, LZMA2', function() {
      assert.ok(lzma.rawDecoderMemusage([{id: lzma.FILTER_LZMA1}]) > 0);
      assert.ok(lzma.rawDecoderMemusage([{id: lzma.FILTER_LZMA2}]) > 0);
    });

    it('should return null for VLI_UNKNOWN', function() {
      assert.strictEqual(null, lzma.rawDecoderMemusage([{id: lzma.VLI_UNKNOWN}]));
    });

    it('should be monotonous in the preset parameter', function() {
      for (var i = 1; i < 9; ++i)
        assert.ok(lzma.rawDecoderMemusage([{id: lzma.FILTER_LZMA2, preset: i+1}]) >=
          lzma.rawDecoderMemusage([{id: lzma.FILTER_LZMA2, preset: i}]));
    });

    it('should fail if input is not an array of filter objects', function() {
      assert.throws(function() { lzma.rawDecoderMemusage(null); });
      assert.throws(function() { lzma.rawDecoderMemusage([null]); });
    });
  });

  describe('#easyEncoderMemusage', function() {
    if('should be positive', function() {
      assert.ok(lzma.easyEncoderMemusage(1) > 0);
    });

    it('should be monotonous in the preset parameter', function() {
      for (var i = 1; i < 9; ++i)
        assert.ok(lzma.easyEncoderMemusage(i+1) >= lzma.easyEncoderMemusage(i));
    });
  });

  describe('#easyDecoderMemusage', function() {
    if('should be positive', function() {
      assert.ok(lzma.easyDecoderMemusage(1) > 0);
    });

    it('should be monotonous in the preset parameter', function() {
      for (var i = 1; i < 9; ++i)
        assert.ok(lzma.easyDecoderMemusage(i+1) >= lzma.easyDecoderMemusage(i));
    });
  });

  describe('#isXZ', function() {
    it('should correctly identify an XZ file', function() {
      assert.ok(lzma.isXZ(fs.readFileSync('test/hamlet.txt.xz')));
    });

    it('should fail for an LZMA file', function() {
      assert.ok(!lzma.isXZ(fs.readFileSync('test/hamlet.txt.lzma')));
    });

    it('should fail for invalid buffers', function() {
      assert.ok(!lzma.isXZ());
      assert.ok(!lzma.isXZ([]));
      assert.ok(!lzma.isXZ(Buffer.alloc(1)));
      assert.ok(!lzma.isXZ('ASDFGHJKL'));
    });
  });

  /* meta stuff */
  describe('.version', function() {
    it('should be the same as the package.json version', function() {
      var pj = JSON.parse(fs.readFileSync('package.json'));

      assert.strictEqual(pj.version, lzma.version);
    });
  });
});
