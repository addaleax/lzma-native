'use strict';

// Builds a static liblzma from the xz source tarball using CMake and MSVC.
// This runs as a node-gyp action, so MSBuild has already put the compiler
// for the target architecture on PATH.
//
// Usage: node liblzma-build-win.js <output dir> <xz tarball> <configuration>

const childProcess = require('child_process');
const fs = require('fs');
const path = require('path');

const outDir = path.resolve(process.argv[2]);
const tarball = path.resolve(process.argv[3]);
const isDebug = /^debug$/i.test(process.argv[4] || '');

const srcRoot = path.join(outDir, 'src');
const buildDir = path.join(outDir, 'cmake-build');

function run(file, args) {
  const result = childProcess.spawnSync(file, args, { stdio: 'inherit' });
  if (result.error) {
    throw result.error;
  }
  if (result.status !== 0) {
    throw new Error(`${path.basename(file)} exited with code ${result.status}`);
  }
}

function findCMake() {
  const onPath = childProcess.spawnSync('cmake', ['--version'], { stdio: 'ignore' });
  if (!onPath.error && onPath.status === 0) {
    return 'cmake';
  }

  // Fall back to the CMake that ships with Visual Studio's C++ CMake tools.
  const programFiles = process.env['ProgramFiles(x86)'] || process.env.ProgramFiles;
  const vswhere = path.join(programFiles, 'Microsoft Visual Studio', 'Installer', 'vswhere.exe');
  if (fs.existsSync(vswhere)) {
    const result = childProcess.spawnSync(vswhere, [
      '-latest', '-products', '*', '-prerelease',
      '-find', 'Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin\\cmake.exe'
    ], { encoding: 'utf8' });
    const found = (result.stdout || '').split(/\r?\n/).filter(Boolean)[0];
    if (found && fs.existsSync(found)) {
      return found;
    }
  }

  throw new Error('CMake 3.20 or newer is required to build liblzma on Windows. ' +
    'Install the "C++ CMake tools for Windows" Visual Studio component or ' +
    'add cmake to PATH.');
}

function main() {
  // Retry because Windows may briefly hold handles to files from a previous build.
  fs.rmSync(outDir, { recursive: true, force: true, maxRetries: 10, retryDelay: 200 });
  fs.mkdirSync(srcRoot, { recursive: true });

  // Use the bsdtar bundled with Windows, which handles drive-letter paths and
  // .tar.bz2 without needing any other tools.
  const tar = path.join(process.env.SystemRoot || 'C:\\Windows', 'System32', 'tar.exe');
  run(tar, ['-xjf', tarball, '-C', srcRoot]);
  const sourceDirs = fs.readdirSync(srcRoot);
  if (sourceDirs.length !== 1) {
    throw new Error(`Unexpected contents in ${tarball}: ${sourceDirs.join(', ')}`);
  }
  const xzDir = path.join(srcRoot, sourceDirs[0]);

  const cmake = findCMake();
  run(cmake, [
    '-S', xzDir,
    '-B', buildDir,
    '-G', 'NMake Makefiles',
    '-DCMAKE_BUILD_TYPE=Release',
    // Node.js addons link the static CRT (/MT or /MTd), so liblzma must too.
    `-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded${isDebug ? 'Debug' : ''}`,
    `-DCMAKE_INSTALL_PREFIX=${outDir.replace(/\\/g, '/')}`,
    '-DBUILD_SHARED_LIBS=OFF',
    '-DBUILD_TESTING=OFF',
    '-DXZ_NLS=OFF',
    '-DXZ_DOC=OFF',
    '-DXZ_TOOL_XZ=OFF',
    '-DXZ_TOOL_XZDEC=OFF',
    '-DXZ_TOOL_LZMADEC=OFF',
    '-DXZ_TOOL_LZMAINFO=OFF'
  ]);
  run(cmake, ['--build', buildDir, '--target', 'liblzma']);
  run(cmake, ['--install', buildDir]);
}

try {
  main();
} catch (err) {
  console.error(`Failed to build liblzma: ${err.message}`);
  process.exit(1);
}
