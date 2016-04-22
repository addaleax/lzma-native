(function() {
'use strict';

var stream = require('readable-stream');
var util = require('util');
var extend = require('util-extend');
var assert = require('assert');

// node-pre-gyp magic
var nodePreGyp = require('node-pre-gyp');
var path = require('path');
var binding_path = nodePreGyp.find(path.resolve(path.join(__dirname,'./package.json')));
var native = require(binding_path);

extend(exports, native);

// We allow usage of any promise library using any-promise
var Promise_ = null;

// helper to enable/disable promises
exports.setPromiseAPI = function(newPromiseAPI) {
  var oldPromiseAPI = Promise_;
  
  if (newPromiseAPI === 'default') {
    newPromiseAPI = null;
    
    try {
      newPromiseAPI = require('any-promise');
    } catch(e) {}
  }
  
  // allow passing in undefined to only *get* the currently used API
  if (typeof newPromiseAPI !== 'undefined')
    Promise_ = newPromiseAPI;
  
  return oldPromiseAPI;
};

exports.setPromiseAPI('default');

exports.version = '1.2.0';

var Stream = exports.Stream;

Stream.curAsyncStreams = [];
Stream.maxAsyncStreamCount = 32;

Stream.prototype.getStream = function(options) {
  options = options || {};
  
  var _forceNextTickCb = function() {
    /* I know this looks like “magic/more magic”, but
     * apparently works around a bogus process.nextTick in
     * node v0.11. This probably does not affect real
     * applications which perform other I/O than LZMA compression. */
    setTimeout(function() {}, 1);
  };
  
  var Ret = function(nativeStream) {
    Ret.super_.call(this, options);
    var self = this;
    
    self.nativeStream = nativeStream;
    self.synchronous = (options.synchronous || !native.asyncCodeAvailable) ? true : false;
    self.chunkCallbacks = [];
    
    self.totalIn_ = 0;
    self.totalOut_ = 0;
    
    self._writingLastChunk = false;
    self._isFinished = false;
    
    self.totalIn  = function() { return self.totalIn_; };
    self.totalOut = function() { return self.totalOut_; };
    
    self.cleanup = function() {
      if (self.nativeStream)
        self.nativeStream.resetUnderlying();
      
      self.nativeStream = null;
    };
    
    if (!self.synchronous) {
      Stream.curAsyncStreams.push(self);
      
      var oldCleanup = self.cleanup;
      self.cleanup = function() {
        var index = Stream.curAsyncStreams.indexOf(self);
        
        if (index !== -1)
          Stream.curAsyncStreams.splice(index, 1);
        
        oldCleanup();
      };
    }
    
    // always clean up in case of error
    self.once('error-cleanup', self.cleanup);
    
    self.nativeStream.bufferHandler = function(buf, processedChunks, err, totalIn, totalOut) {
      if (totalIn !== null) {
        self.totalIn_  = totalIn;
        self.totalOut_ = totalOut;
      }
      
      process.nextTick(function() {
        if (err) {
          self.push(null);
          self.emit('error-cleanup', err);
          self.emit('error', err);
          _forceNextTickCb();
        }
        
        if (totalIn !== null) {
          self.emit('progress', {
            totalIn: self.totalIn_,
            totalOut: self.totalOut_
          });
        }
        
        if (typeof processedChunks === 'number') {
          assert.ok(processedChunks <= self.chunkCallbacks.length);
          
          var chunkCallbacks = self.chunkCallbacks.splice(0, processedChunks);
          
          while (chunkCallbacks.length > 0)
            chunkCallbacks.shift()();
          
          _forceNextTickCb();
        } else if (buf === null) {
          if (self._writingLastChunk) {
            self.push(null);
          } else {
            // There may be additional members in the file.
            // Reset and set _isFinished to tell `_flush()` that nothing
            // needs to be done.
            self._isFinished = true;
            
            if (self.nativeStream && self.nativeStream._restart) {
              self.nativeStream._restart();
            } else {
              self.push(null);
            }
          }
        } else {
          self.push(buf);
        }
      });
      
      _forceNextTickCb();
    };
    
    // add all methods from the native Stream
    Object.keys(native.Stream.prototype).forEach(function(key) {
      self[key] = function() { return self.nativeStream[key].apply(self.nativeStream, arguments); };
    });
    
    Object.defineProperty(self, 'bufsize', {
      get: function() {
        return self.setBufsize(null);
      },
      set: function(n) {
        if (typeof n !== 'number' || n <= 0) {
          throw new TypeError('bufsize must be a positive number');
        }
        
        return self.setBufsize(parseInt(n));
      }
    });
    
    if (typeof options.bufsize !== 'undefined') {
      return self.bufsize = options.bufsize;
    }
  };
  
  util.inherits(Ret, stream.Transform);
  
  Ret.prototype._transform = function(chunk, encoding, callback) {
    this.chunkCallbacks.push(callback);
    
    try {
      this._isFinished = false;
      this.nativeStream.code(chunk, !this.synchronous);
    } catch (e) {
      this.emit('error-cleanup', e);
      this.emit('error', e);
    }
  };
  
  Ret.prototype._writev = function(chunks, callback) {
    chunks = chunks.map(function (chunk) { return chunk.chunk; });
    this._write(Buffer.concat(chunks), null, callback);
  };

  Ret.prototype._flush = function(callback) {
    this._writingLastChunk = true;
    var cleanup = this.cleanup;
    
    if (this._isFinished) {
      cleanup();
      callback(null);
      return;
    }

    this._transform(null, null, function() {
      cleanup();
      callback.apply(this, arguments);
    });
  };
  
  return new Ret(this);
};

Stream.prototype.rawEncoder = function(options) {
  return this.rawEncoder_(options.filters || []);
};

Stream.prototype.rawDecoder = function(options) {
  return this.rawDecoder_(options.filters || []);
};

Stream.prototype.easyEncoder = function(options) {
  return this.easyEncoder_(options.preset || exports.PRESET_DEFAULT, options.check || exports.CHECK_CRC32);
};

Stream.prototype.streamEncoder = function(options) {
  return this.streamEncoder_(options.filters || [], options.check || exports.CHECK_CRC32);
};

Stream.prototype.streamDecoder = function(options) {
  this._initOptions = options;
  this._restart = function() {
    this.resetUnderlying();
    this.streamDecoder(this._initOptions);
  };

  return this.streamDecoder_(options.memlimit || null, options.flags || 0);
};

Stream.prototype.autoDecoder = function(options) {
  this._initOptions = options;
  this._restart = function() {
    this.resetUnderlying();
    this.autoDecoder(this._initOptions);
  };

  return this.autoDecoder_(options.memlimit || null, options.flags || 0);
};

Stream.prototype.aloneDecoder = function(options) {
  return this.aloneDecoder_(options.memlimit || null);
};

/* helper functions for easy creation of streams */
var createStream =
exports.createStream = function(coder, options) {
  if (['number', 'object'].indexOf(typeof coder) !== -1 && !options) {
    options = coder;
    coder = null;
  }
  
  if (parseInt(options) === parseInt(options))
    options = {preset: parseInt(options)};
  
  coder = coder || 'easyEncoder';
  options = options || {};
  
  var stream = new Stream();
  stream[coder](options);
  
  if (options.memlimit)
    stream.memlimitSet(options.memlimit);
  
  if (!options.synchronous)
    options.synchronous = ((Stream.curAsyncStreams.length >= Stream.maxAsyncStreamCount) && !options.forceAsynchronous);
  
  return stream.getStream(options);
};

exports.createCompressor = function(options) {
  return createStream('easyEncoder', options);
};

exports.createDecompressor = function(options) {
  return createStream('autoDecoder', options);
};

exports.crc32 = function(input, encoding, presetCRC32) {
  if (typeof encoding === 'number') {
    presetCRC32 = encoding;
    encoding = null;
  }
  
  if (typeof input === 'string') 
    input = new Buffer(input, encoding);
  
  return exports.crc32_(input, presetCRC32 || 0);
};

/* compatibility: node-xz (https://github.com/robey/node-xz) */
exports.Compressor = function(preset, options) {
  options = extend({}, options);
  
  if (preset)
    options.preset = preset;
  
  return createStream('easyEncoder', options);
};

exports.Decompressor = function(options) {
  return createStream('autoDecoder', options);
};

/* compatibility: LZMA-JS (https://github.com/nmrugg/LZMA-JS) */
function singleStringCoding(stream, string, on_finish, on_progress) {
  on_progress = on_progress || function() {};
  on_finish = on_finish || function() {};
  
  // possibly our input is an array of byte integers
  // or a typed array
  if (!Buffer.isBuffer(string))
    string = new Buffer(string);
  
  var deferred = null, failed = false;
  
  stream.once('error', function(err) {
    failed = true;
    on_finish(null, err);
  });
  
  if (Promise_) {
    if (Promise_.defer) {
      deferred = Promise_.defer();
    } else {
      // emulate Promise.defer() if not supported
      deferred = {};
      deferred.promise = new Promise_(function(resolve, reject) {
        deferred.resolve = resolve;
        deferred.reject = reject;
      });
    }
    
    assert.equal(typeof deferred.resolve, 'function');
    assert.equal(typeof deferred.reject, 'function');
    
    stream.once('error', function(e) {
      deferred.reject(e);
    });
  }
  
  var buffers = [];

  stream.on('data', function(b) {
    buffers.push(b);
  });
  
  stream.once('end', function() {
    var result = Buffer.concat(buffers);
    
    if (!failed) {
      on_progress(1.0);
      on_finish(result);
    }
    
    if (deferred)
      deferred.resolve(result);
  });

  on_progress(0.0);
  
  stream.end(string);
  
  if (deferred)
    return deferred.promise;
}

exports.LZMA = function() {
  return {
    compress: function(string, mode, on_finish, on_progress) {
      var opt = {};

      if (parseInt(mode) === parseInt(mode) && mode >= 1 && mode <= 9)
        opt.preset = parseInt(mode);

      var stream = createStream('aloneEncoder', opt);
      
      return singleStringCoding(stream, string, on_finish, on_progress);
    },
    decompress: function(byte_array, on_finish, on_progress) {
      var stream = createStream('autoDecoder');
      
      return singleStringCoding(stream, byte_array, on_finish, on_progress);
    },
    // dummy, we don’t use web workers
    worker: function() { return null; }
  };
};

exports.compress = function(string, opt, on_finish) {
  if (typeof opt === 'function') {
    on_finish = opt;
    opt = {};
  }
  
  var stream = createStream('easyEncoder', opt);
  return singleStringCoding(stream, string, on_finish);
};

exports.decompress = function(string, opt, on_finish) {
  if (typeof opt === 'function') {
    on_finish = opt;
    opt = {};
  }
  
  var stream = createStream('autoDecoder', opt);
  return singleStringCoding(stream, string, on_finish);
};

exports.isXZ = function(buf) {
  return buf && buf.length >= 6 &&
         buf[0] === 0xfd &&
         buf[1] === 0x37 &&
         buf[2] === 0x7a &&
         buf[3] === 0x58 &&
         buf[4] === 0x5a &&
         buf[5] === 0x00;
};

})();
