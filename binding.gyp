{
  'includes': [ 'deps/common-libzim.gypi' ],
  'variables': {
      'libzim%':'internal',
  },
  'targets': [
    {
      'target_name': 'zim',
      "include_dirs": ["<!(node -e \"require('nan')\")","."],
      'conditions': [
        ['libzim != "internal"', {
            'libraries': [ "<!@(pkg-config --ldflags libzim)" ],
            'cflags': [ "<!@(pkg-config --cppflags libzim)" ]
        },
        {
            'dependencies': [
              'deps/libzim.gyp:libzim'
            ]
        }
        ]
      ],
      'sources': [
        'src/node_libzim.cc',
        'src/node_libzim_writer.cc',
        'src/node_libzim_writer.h',
      ],
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}
