{
	"targets": [
		{
			"includes": [
				"../auto.gypi",
				"../../sources.gypi"
			],
			'libraries': [ "-L/usr/local/lib", "-lzim" ],
		}
	],
	"includes": [
		"../auto-top.gypi"
	]
}
