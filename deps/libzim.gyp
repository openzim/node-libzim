{
  'includes': [ 'common-libzim.gypi' ],
  'target_defaults': {
    'default_configuration': 'Release',
    'configurations': {
      'Debug': {
        'variables': {
          'configure_options': ['--enable-debug']
        },
        'defines': [ 'DEBUG', '_DEBUG' ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 1, # static debug
          },
        },
      },
      'Release': {
        'variables': {
          'configure_options': ['--enable-release']
        },
        'defines': [ 'NDEBUG' ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 0, # static release
          },
        },
      }
    },
    'msvs_settings': {
      'VCCLCompilerTool': {
      },
      'VCLibrarianTool': {
      },
      'VCLinkerTool': {
        'GenerateDebugInformation': 'true',
      },
    },
    'conditions': [
      ['OS == "win"', {
        'defines': [
          'WIN32'
        ],
      }]
    ],
  },

  'targets': [
    {
      'target_name': 'action_before_build',
      'type': 'none',
      'hard_dependency': 1,
      'actions': [
        {
          'action_name': 'configure_libzim',
          'inputs': [
            './openzim/zimlib/autogen.sh',
            './openzim/zimlib/configure.ac',
            './openzim/zimlib/Makefile.am'
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/Makefile'
          ],
          'action': [
                './cdconfigure.js',
                '<(SHARED_INTERMEDIATE_DIR)/',
                #'CFLAGS=-fPIC', 'CXXFLAGS=-fPIC', # required on x64
                '--enable-static', '--disable-shared',
                '<@(configure_options)'
          ]
        }
      ]
    },
    {
      'target_name': 'build',
      'dependencies': [
        'action_before_build'
      ],
      'actions': [
        {
          'action_name': 'build_libzim',
          'inputs': [
            '<(SHARED_INTERMEDIATE_DIR)/Makefile'
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/src/.libs/libzim.a'
          ],
          'action': ['make', '-C', '<(SHARED_INTERMEDIATE_DIR)/',
                     '-j', '2']
        }
      ]
    },
    {
      'target_name': 'libzim',
      'type': 'none',
      'dependencies': [
        'build'
      ],
      'direct_dependent_settings': {
        'include_dirs': [ './openzim/zimlib/include' ],
      },
      'link_settings': {
        'libraries': [
            '<(SHARED_INTERMEDIATE_DIR)/src/.libs/libzim.a'
        ]
      },
      'sources': [
            '<(SHARED_INTERMEDIATE_DIR)/src/.libs/libzim.a'
      ]
    }
  ]
}
