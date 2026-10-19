{
  "variables": {
    "xz_tarball%": "<(module_root_dir)/deps/xz-5.8.4.tar.bz2",
    "liblzma_win_dir": "<(SHARED_INTERMEDIATE_DIR)/liblzma"
  },
  "targets": [
    {
      "target_name": "lzma_native",
      "sources": [
        "src/util.cpp",
        "src/liblzma-functions.cpp",
        "src/filter-array.cpp",
        "src/lzma-stream.cpp",
        "src/module.cpp",
        "src/mt-options.cpp",
        "src/index-parser.cpp"
      ],
      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")"],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")", "liblzma"],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7',
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      },
      "conditions" : [
        [ 'OS!="win"' , {
          "include_dirs" : [ "<(module_root_dir)/build/liblzma/build/include" ],
          "libraries" : [ "<(module_root_dir)/build/liblzma/build/lib/liblzma.a" ],
          "cflags": ['-O3 -std=c++11']
        }, {
          "include_dirs" : [ "<(liblzma_win_dir)/include" ],
          "defines" : [ "LZMA_API_STATIC" ],
          "libraries" : [ "<(liblzma_win_dir)/lib/lzma.lib" ]
        } ],
      ],
    },
    {
      "target_name" : "liblzma",
      "type" : "none",
      "conditions" : [
        [ 'OS!="win"' , {
          "actions" : [
            {
              "action_name" : "build",
               # a hack to run deps/xz-*/configure during `node-gyp configure`
              'inputs': ['<!@(sh liblzma-config.sh "<(module_root_dir)/build" "<(xz_tarball)")'],
              'outputs': [''],
              'action': [
                'sh', '<(module_root_dir)/liblzma-build.sh', '<(module_root_dir)/build'
              ]
            }
          ]
        }, {
          "actions" : [
            {
              "action_name" : "build",
              'inputs': ['<(xz_tarball)', '<(module_root_dir)/liblzma-build-win.js'],
              'outputs': ['<(liblzma_win_dir)/lib/lzma.lib'],
              'action': [
                'node', '<(module_root_dir)/liblzma-build-win.js',
                '<(liblzma_win_dir)', '<(xz_tarball)', '<(CONFIGURATION_NAME)'
              ]
            }
          ]
        } ],
      ]
    }
  ]
}
