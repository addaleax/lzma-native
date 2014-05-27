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
				"-std=c++11",
			],
			"link_settings": {
				"libraries": [
					"-llzma"
				]
			}
		}
	]
}
