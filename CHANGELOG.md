# Changelog for lzma-native

## 2.0.1, March 24 2017

* [[`c0491a0a07`](https://github.com/addaleax/lzma-native/commit/c0491a0a07)] - refactored binding.gyp (Refael Ackermann) 
* [[`70883635b7`](https://github.com/addaleax/lzma-native/commit/70883635b7)] - **ci**: skip artifact encryption setup for non-tag builds (Anna Henningsen) 

## 2.0.0, March 19 2017

Changes since 1.5.2

Notable changes:

* Dropped support for Node 0.10 and 0.12, which includes dropping `any-promise` and `util-extend` as dependencies.
* A changed path for the prebuilt binaries, which now includes versioning information.

* [[`83e0007061`](https://github.com/addaleax/lzma-native/commit/83e0007061)] - Bump version to 1.5.3
* [[`8021673b5d`](https://github.com/addaleax/lzma-native/commit/8021673b5d)] - Silence warnings about deprecated `NewInstance` usage
* [[`061933c4c7`](https://github.com/addaleax/lzma-native/commit/061933c4c7)] - **bin**: drop `commander` dependency
* [[`d752f96be4`](https://github.com/addaleax/lzma-native/commit/d752f96be4)] - **ci**: don’t use -flto for now
* [[`92188bee5e`](https://github.com/addaleax/lzma-native/commit/92188bee5e)] - **ci**: fix AppVeyor allocation failures
* [[`b79fa969d4`](https://github.com/addaleax/lzma-native/commit/b79fa969d4)] - **ci**: fix AppVeyor indexparser failures
* [[`5fcc17e54f`](https://github.com/addaleax/lzma-native/commit/5fcc17e54f)] - **ci**: fix Travis gcc CI failures
* [[`3f5d2609bd`](https://github.com/addaleax/lzma-native/commit/3f5d2609bd)] - **ci**: drop Node v0.10/v0.12 support
* [[`48e48ea25a`](https://github.com/addaleax/lzma-native/commit/48e48ea25a)] - **ci**: ci file housekeeping
* [[`c2d06b5e09`](https://github.com/addaleax/lzma-native/commit/c2d06b5e09)] - **ci**: work around node-gyp build failures
* [[`f94287f711`](https://github.com/addaleax/lzma-native/commit/f94287f711)] - **ci,test**: drop explicit nw.js testing
* [[`c61355984f`](https://github.com/addaleax/lzma-native/commit/c61355984f)] - **deps**: update xz to 5.2.3
* [[`b07f501e26`](https://github.com/addaleax/lzma-native/commit/b07f501e26)] - **doc**: leave blank lines around headings in README
* [[`dea30f3f20`](https://github.com/addaleax/lzma-native/commit/dea30f3f20)] - **lib**: drop util-extend dependency
* [[`0988b8d360`](https://github.com/addaleax/lzma-native/commit/0988b8d360)] - **lib**: refactor js-facing Stream into class
* [[`18bbdfc220`](https://github.com/addaleax/lzma-native/commit/18bbdfc220)] - **lib**: always use ES6 promises
* [[`f5030e027e`](https://github.com/addaleax/lzma-native/commit/f5030e027e)] - **lib**: fix unhandled Promise rejections
* [[`6e887ca52c`](https://github.com/addaleax/lzma-native/commit/6e887ca52c)] - **meta**: package.json housekeeping
* [[`e884b2e7c1`](https://github.com/addaleax/lzma-native/commit/e884b2e7c1)] - **prebuild**: add versioning to the binding file path
* [[`e8660b3728`](https://github.com/addaleax/lzma-native/commit/e8660b3728)] - **src**: use Nan::MakeCallback() for calling into JS
* [[`bd7ee7ce3f`](https://github.com/addaleax/lzma-native/commit/bd7ee7ce3f)] - **test**: use `fs.unlinkSync` for synchronous unlinking

