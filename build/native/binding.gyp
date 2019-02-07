{
	"variables": {
      "libzim": "-L../../../download/",
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
