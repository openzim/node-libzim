{
	"variables": {
      "libzim_dir": "<(module_root_dir)/../../download",
	  "libzim_include": "../../download/include",
  	},
	"targets": [
		{
			"libraries": [ 
				"-Wl,-rpath,<(libzim_dir)",
				"-L<(libzim_dir)",
				"-lzim",
			],
			"include_dirs": [
				"<(libzim_include)",
			],
			"includes": [
				"../auto.gypi",
				"../../sources.gypi"
			],
		}
	],
	"includes": [
		"../auto-top.gypi"
	]
}
