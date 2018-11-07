'use strict';

var assert = require('assert');
var fs = require('fs');

var lzma = require('../');

if (!assert.deepStrictEqual) {
  assert.deepStrictEqual = assert.deepEqual;
}

describe('lzma', function() {
  var checkInfo = function(info) {
    assert.strictEqual(info.memlimit, null);
    assert.strictEqual(info.streams, 2);
    assert.strictEqual(info.blocks, 2);
    assert.ok(info.fileSize < info.uncompressedSize);
    assert.deepStrictEqual(info.checks, [ lzma.CHECK_CRC64 ]);
  };

  if (typeof gc !== 'undefined') {
    afterEach('garbage-collect', gc);
  }

  describe('#parseFileIndex', function() {
    var hamletXZ;
    before('Read from a buffer into memory', function() {
      hamletXZ = fs.readFileSync('test/hamlet.txt.2stream.xz');
    });

    it('should fail for zero-length files', function(done) {
      lzma.parseFileIndex({
        fileSize: 0,
        read: function(count, offset, cb) {
          assert.ok(false);
        }
      }, function(err, info) {
        assert.ok(err);
        assert(/File is empty/.test(err.message));

        done();
      });
    });

    it('should fail for too-small files', function(done) {
      lzma.parseFileIndex({
        fileSize: 10,
        read: function(count, offset, cb) {
          assert.ok(false);
        }
      }, function(err, info) {
        assert.ok(err);
        assert(/Too small/.test(err.message));

        done();
      });
    });

    it('should fail for truncated files', function(done) {
      lzma.parseFileIndex({
        fileSize: hamletXZ.length,
        read: function(count, offset, cb) {
          cb(hamletXZ.slice(offset, offset + count - 1));
        }
      }, function(err, info) {
        assert.ok(err);
        assert.strictEqual(err.name, 'LZMA_DATA_ERROR');

        done();
      });
    });

    it('should fail when I/O errors are passed along', function(done) {
      lzma.parseFileIndex({
        fileSize: hamletXZ.length,
        read: function(count, offset, cb) {
          cb(new Error('I/O failed'));
        }
      }, function(err, info) {
        assert.ok(err);
        assert.strictEqual(err.message, 'I/O failed');

        done();
      });
    });

    it('should fail when I/O errors are passed along, sync version', function() {
      assert.throws(function() {
        lzma.parseFileIndex({
          fileSize: hamletXZ.length,
          read: function(count, offset, cb) {
            cb(new Error('I/O failed'));
          }
        });
      }, /I\/O failed/);
    });

    it('should fail for invalid files', function(done) {
      lzma.parseFileIndex({
        fileSize: hamletXZ.length,
        read: function(count, offset, cb) {
          var buf = Buffer.alloc(count);
          cb(buf);
        }
      }, function(err, info) {
        assert.ok(err);
        assert.strictEqual(err.name, 'LZMA_DATA_ERROR');

        done();
      });
    });

    it('should be able to parse a file synchronously', function(done) {
      lzma.parseFileIndex({
        fileSize: hamletXZ.length,
        read: function(count, offset, cb) {
          cb(hamletXZ.slice(offset, offset + count));
        }
      }, function(err, info) {
        if (err) return done(err);

        checkInfo(info);

        done();
      });
    });

    it('should be able to parse a file with synchronous return', function() {
      var info = lzma.parseFileIndex({
        fileSize: hamletXZ.length,
        read: function(count, offset, cb) {
          cb(hamletXZ.slice(offset, offset + count));
        }
      });

      checkInfo(info);
    });

    it('should be able to parse a file asynchronously', function(done) {
      lzma.parseFileIndex({
        fileSize: hamletXZ.length,
        read: function(count, offset, cb) {
          process.nextTick(function() {
            cb(null, hamletXZ.slice(offset, offset + count));
          });
        }
      }, function(err, info) {
        if (err) return done(err);

        checkInfo(info);

        done();
      });
    });

    it('should be able to parse a file asynchronously, alternative callback style', function(done) {
      lzma.parseFileIndex({
        fileSize: hamletXZ.length,
        read: function(count, offset, cb) {
          process.nextTick(function() {
            cb(hamletXZ.slice(offset, offset + count));
          });
        }
      }, function(err, info) {
        if (err) return done(err);

        checkInfo(info);

        done();
      });
    });
  });

  describe('#parseFileIndexFD', function() {
    var fd;
    before('Open the file', function(done) {
      fs.open('test/hamlet.txt.2stream.xz', 'r', function(err, fd_) {
        fd = fd_;
        done(err);
      });
    });

    after('Close the file', function(done) {
      fs.close(fd, done);
    });

    it('should be able to parse a file from a file descriptor', function(done) {
      lzma.parseFileIndexFD(fd, function(err, info) {
        if (err) return done(err);

        checkInfo(info);
        done();
      });
    });

    it('should fail for invalid file descriptors', function(done) {
      lzma.parseFileIndexFD(1000, function(err, info) {
        assert.ok(err);
        assert.ok(!info);
        done();
      });
    });
  });
});
