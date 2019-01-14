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
			"libraries": [ "<(libzim)" ],
		}
	],
	"includes": [
		"../auto-top.gypi"
	]
}
