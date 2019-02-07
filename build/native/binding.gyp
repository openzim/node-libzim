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
			"libraries": [ "-L<(libzim)", "-llibzim.so" ],
		}
	],
	"includes": [
		"../auto-top.gypi"
	]
}
