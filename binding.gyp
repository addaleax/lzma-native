{
	"targets": [
		{
			"target_name": "lzma_native",
			"sources": [
				"src/util.cpp",
				"src/liblzma-functions.cpp",
				"src/filter-array.cpp",
				"src/lzma-stream.cpp",
				"src/module.cpp"
			],
			"include_dirs" : [
				"<!(node -e \"require('nan')\")"
			],
			"conditions" : [
				[ 'OS!="win"' , {
					"include_dirs" : [ "<(module_root_dir)/build/liblzma/build/include" ],
					"libraries" : [ "<(module_root_dir)/build/liblzma/build/lib/liblzma.a" ],
					"dependencies" : [ "liblzma" ],
				}, {
					"include_dirs" : [ "<(module_root_dir)/deps/include" ],
					"conditions": [
						['target_arch=="x64"', {
							"libraries" : [ "<(module_root_dir)/deps/liblzma-x86-64.dll" ]
						}, {
							"libraries" : [ "<(module_root_dir)/deps/liblzma-i686.dll" ]
						}]
					]
				} ],
				[ 'gcc_version<=47', {
				}, {
					"cflags": ["-std=c++11"]
				} ]
			],
		},
		{
			"target_name" : "liblzma",
			"type" : "none",
			"conditions" : [
				[ 'OS!="win"' , {
					"actions" :
						[
							{
								"action_name" : "build",
								 # a hack to run deps/xz-5.2.1 ./configure during `node-gyp configure`
								'inputs': ['<!@(sh liblzma-config.sh "<(module_root_dir)/build" "<(module_root_dir)/deps/xz-5.2.1.tar.bz2")'],
								'outputs': [''],
								'action': [
								# run deps/mhash `make`
								'sh', '<(module_root_dir)/liblzma-build.sh', '<(module_root_dir)/build'
								]
							}
						]
				} ],
			]
		}
	]
}
