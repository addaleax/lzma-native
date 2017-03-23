{
  "variables": {
    "dlldir%": "<(module_path)"
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
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      "dependencies" : [ "liblzma" ],
      "conditions" : [
        [ 'OS!="win"' , {
          "include_dirs" : [ "<(module_root_dir)/build/liblzma/build/include" ],
          "libraries" : [ "<(module_root_dir)/build/liblzma/build/lib/liblzma.a" ],
          "cflags": ['<!@(sh ./cflags.sh)']
        }, {
          "include_dirs" : [ "<(module_root_dir)\\deps\\include" ],
          "link_settings": {
            "libraries" : [ "-llzma" ],
            "conditions": [
              [ 'target_arch=="x64"', {
                "library_dirs" : [ "<(module_root_dir)\\deps\\bin_x86-64" ]
              }, {
                "library_dirs" : [ "<(module_root_dir)\\deps\\bin_i686" ]
              } ]
            ]
          }
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
               # a hack to run deps/xz-5.2.3 ./configure during `node-gyp configure`
              'inputs': ['<!@(sh liblzma-config.sh "<(module_root_dir)/build" "<(module_root_dir)/deps/xz-5.2.3.tar.bz2")'],
              'outputs': [''],
              'action': [
                'sh', '<(module_root_dir)/liblzma-build.sh', '<(module_root_dir)/build'
              ]
            }
          ]
        }, {
          "conditions": [
            [ 'target_arch=="x64"', {
              'variables': {
                "arch_lib_path" : 'bin_x86-64',
                "arch_lib_code" : 'x64'
              }
            }, {
              'variables': {
                "arch_lib_path" : 'bin_i686',
                "arch_lib_code" : 'ix86'
              }
            } ]
          ],
          "actions" : [
            {
              "msvs_quote_cmd": 0,
              "action_name" : "build",
              'inputs': ['deps/doc/liblzma.def'],
              'outputs': [''],
              'action': ['lib.exe -def:"<(module_root_dir)/deps/doc/liblzma.def" -out:"<(module_root_dir)/deps/<(arch_lib_path)/lzma.lib" -machine:<(arch_lib_code)']
            },
            {
              "msvs_quote_cmd": 0,
              "action_name" : "deploy",
              'inputs': ['deps/<(arch_lib_path)/liblzma.dll'],
              'outputs': ['<(dlldir)/liblzma.dll'],
              'action': ['mkdir <(dlldir) > nul 2>&1 & copy "<(module_root_dir)/deps/<(arch_lib_path)/liblzma.dll" <(dlldir)/liblzma.dll']
            }
          ]
        } ],
      ]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}
