# Changelog for lzma-native

## 3.0.8, May 12 2018

* [[`8c18848609`](https://github.com/addaleax/lzma-native/commit/8c18848609)] - **ci**: add Node.js 10 to matrix (Anna Henningsen)

## 3.0.7, Mar 26 2018

This likely fixed a regression related to node-pre-gyp.

* [[`430a440276`](https://github.com/addaleax/lzma-native/commit/430a440276)] - **package**: pin node-pre-gyp to 0.6.39 (Anna Henningsen)

## 3.0.6, Mar 26 2018

* [[`484c53577f`](https://github.com/addaleax/lzma-native/commit/484c53577f)] - **package**: update dependencies (Anna Henningsen)
* [[`6513708704`](https://github.com/addaleax/lzma-native/commit/6513708704)] - **lib**: use `Buffer.*` instead of deprecated Buffer constructor (Anna Henningsen)

## 3.0.5, Feb 21 2018

* [[`c03299db13`](https://github.com/addaleax/lzma-native/commit/c03299db13)] - **ci**: remove OS X from coverage (Anna Henningsen)
* [[`5f640416e0`](https://github.com/addaleax/lzma-native/commit/5f640416e0)] - **lib**: fix issue with invalid input (Anna Henningsen)

## 3.0.4, Nov 27 2017

* [[`669ee5098b`](https://github.com/addaleax/lzma-native/commit/669ee5098b)] - **package**: replace unavailable host to node-pre-gyp.addaleax.net (JianyingLi) [#48](https://github.com/addaleax/lzma-native/pull/48)

## 3.0.3, Nov 26 2017

* [[`fcba77ebe0`](https://github.com/addaleax/lzma-native/commit/fcba77ebe0)] - **ci**: include Node 9 support (Anna Henningsen)

## 3.0.2, Nov 07 2017

* [[`82b97dd94f`](https://github.com/addaleax/lzma-native/commit/82b97dd94f)] - **package**: update dependencies (Anna Henningsen)

## 3.0.1, Jul 04 2017

* [[`9e2ee5129f`](https://github.com/addaleax/lzma-native/commit/9e2ee5129f)] - **ci**: fix CI on Windows (Anna Henningsen)
* [[`8d75757031`](https://github.com/addaleax/lzma-native/commit/8d75757031)] - **lib**: fix race condition (Alexander Sagen) [#40](https://github.com/addaleax/lzma-native/pull/40)

## 3.0.0, Jun 26 2017

This is unlikely to break anybody’s code, but removing the build files after install might qualify as semver-major.

* [[`d5a252e3de`](https://github.com/addaleax/lzma-native/commit/d5a252e3de)] - **build**: rimraf build/ after install (Anna Henningsen)
* [[`fd2165e2ae`](https://github.com/addaleax/lzma-native/commit/fd2165e2ae)] - **ci**: add electron prebuilts again (Anna Henningsen)
* [[`039ac523d0`](https://github.com/addaleax/lzma-native/commit/039ac523d0)] - **lib**: explicit util.promisify() compat (Anna Henningsen)

## 2.0.4, Jun 25 2017

* [[`0cc00000b3`](https://github.com/addaleax/lzma-native/commit/0cc00000b3)] - **ci**: fix macOS prebuild binaries (Anna Henningsen)

## 2.0.3, Jun 21 2017

* [[`621628abac`](https://github.com/addaleax/lzma-native/commit/621628abac)] - **ci**: add Node 8 to CI matrix (Anna Henningsen)

## 2.0.2, May 18 2017

* [[`39bd6a2dc0`](https://github.com/addaleax/lzma-native/commit/39bd6a2dc0)] - **package**: pin nan to 2.5.1 (Anna Henningsen)

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

