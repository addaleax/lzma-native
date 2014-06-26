{
	"targets": [
		{
			"target_name": "lzma_native",
			"sources": [
				"src/util.cpp",
				"src/liblzma-functions.cpp",
				"src/filter-array.cpp",
				"src/lzma-stream.cpp",
				"src/lzma-stream-asynccode.cpp",
				"src/module.cpp"
			],
			"cflags": [
				"-std=c++0x",
			],
			"include_dirs" : [ "build/liblzma/build/include"],
			"dependencies" : [ "liblzma" ],
			"libraries" : [ "<(module_root_dir)/build/liblzma/build/lib/liblzma.a" ]
		},
		{
			"target_name" : "liblzma",
			"type" : "none",
			"actions" :
			[
				{
					"action_name" : "build",
					 # a hack to run deps/xz-5.0.5 ./configure during `node-gyp configure`
					'inputs': ['<!@(sh liblzma-config.sh <(module_root_dir)/build <(module_root_dir)/deps/xz-5.0.5.tar.bz2)'],
					'outputs': [''],
					'action': [
					# run deps/mhash `make`
					'sh', '<(module_root_dir)/liblzma-build.sh', '<(module_root_dir)/build'
					]
				}
			]
		}
	]
}
