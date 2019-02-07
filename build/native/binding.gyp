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
			"libraries": [ "<(libzim)" ],
		}
	],
	"includes": [
		"../auto-top.gypi"
	]
}
