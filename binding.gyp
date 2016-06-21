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
            'libraries': [ "<!@(sh -c 'pkg-config --exists libzim && pkg-config --libs libzim || echo -lzim')" ],
            'cflags': [ "<!@(sh -c 'pkg-config --exists libzim && pkg-config --cppflags libzim || true')", '-std=c++11' ],
            'defines': [ 'EXTERNAL_LIBZIM' ],
        },
        {
            'cflags': [ '-std=c++11' ],
            'dependencies': [
              'deps/libzim.gyp:libzim'
            ]
        }
        ]
      ],
      'xcode_settings': {
        'MACOSX_DEPLOYMENT_TARGET': '10.7',

        'OTHER_CFLAGS': [
          '-std=c++11',
          '-stdlib=libc++'
        ],
      },
      'sources': [
        'src/blob.h',
        'src/uuid.h',
        'src/writer.h',
        'src/writer.cc',
        'src/node_libzim.cc',
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
