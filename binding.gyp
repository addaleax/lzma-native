{
	"targets": [
		{
			"target_name": "lzma_native",
			"sources": ["liblzma-node.cpp"],
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
