{
	"variables": {
      "libzim_dir": "<(module_root_dir)/../../download/lib64",
	  "libzim_include": "<(module_root_dir)/../../download/include",
  	},
	"targets": [
		{
			"conditions": [
				["OS=='linux'", {
					"libraries": [ 
						"-Wl,-rpath,<(libzim_dir)",
						"-L<(libzim_dir)",
						"<(libzim_dir)/libzim.so.4",
					],
				}],
				["OS=='mac'", {
					"libraries": [ "-lzim" ]
				}],
			],
			"include_dirs": [
				"<(libzim_include)",
			],
			"includes": [
				"../auto.gypi",
				"../../sources.gypi"
			],
			'cflags': [ '-D_GLIBCXX_USE_CXX11_ABI=0' ],
		}
	],
	"includes": [
		"../auto-top.gypi"
	]
}
