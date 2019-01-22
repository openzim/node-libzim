{
	"variables": {
      "libzim": "-L../../../download/include",
  	},
	"targets": [
		{
			"includes": [
				"../auto.gypi",
				"../../sources.gypi"
			],
			"libraries": [ "<(libzim)", "-lzim" ],
		}
	],
	"includes": [
		"../auto-top.gypi"
	]
}
