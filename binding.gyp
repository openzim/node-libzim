{
    "variables": {
      "libzim_dir": "<(module_root_dir)/download",
      "libzim_include": "<(module_root_dir)/download/include",
      "libzim_local": "false",  # change this to use system-installed libzim
    },
    "targets": [
        {
            "conditions": [
              ["libzim_local=='true' and OS=='linux'", {
                "link_settings": {
                  "ldflags": [
                      "<!(pkg-config --libs-only-other --libs-only-L libzim)",
                      "-Wl,-rpath,<!(pkg-config --variable=libdir libzim)>"
                  ],
                  "libraries": [
                      "<!(pkg-config --libs-only-l libzim)",
                  ],
                },
              }],
              ["libzim_local!='true' and OS=='linux' and target_arch=='x64'", {
                "include_dirs": [
                  "<(libzim_include)",
                ],
                "libraries": [
                  "-Wl,-rpath,'$$ORIGIN'",
                  "-L<(libzim_dir)/lib/x86_64-linux-gnu",
                  "<(libzim_dir)/lib/x86_64-linux-gnu/libzim.so.9",
                ],
              }],
              ["libzim_local!='true' and OS=='linux' and target_arch=='arm64'", {
                "include_dirs": [
                  "<(libzim_include)",
                ],
                "libraries": [
                  "-Wl,-rpath,'$$ORIGIN'",
                  "-L<(libzim_dir)/lib/aarch64-linux-gnu",
                  "<(libzim_dir)/lib/aarch64-linux-gnu/libzim.so.9",
                ],
              }],
              ["libzim_local!='true' and OS=='linux' and target_arch=='arm'", {
                "include_dirs": [
                  "<(libzim_include)",
                ],
                "libraries": [
                  "-Wl,-rpath,'$$ORIGIN'",
                  "-L<(libzim_dir)/lib/arm-linux-gnueabihf",
                  "<(libzim_dir)/lib/arm-linux-gnueabihf/libzim.so.9",
                ],
              }],
              ["libzim_local!='true' and OS=='mac'", {
                  "include_dirs": ["<(libzim_include)"],
                  "cflags+": ["-fvisibility=hidden"],
                  "xcode_settings": {
                      "GCC_SYMBOLS_PRIVATE_EXTERN": "YES",  # -fvisibility=hidden
                      "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
                      "LD_RUNPATH_SEARCH_PATHS": "@loader_path/",
                      "OTHER_CFLAGS": [ "-std=c++17", "-fexceptions" ],
                  },
                  "libraries": ["-Wl,-rpath,@loader_path/", "-L<(libzim_dir)/lib", "-lzim"],
              }],
            ],
            "target_name": "zim_binding",
            "cflags!": [ "-fno-exceptions" ],
            "cflags_cc!": [ "-fno-exceptions" ],
            "cflags": [],
            "cflags_cc": [ "-std=c++17", "-fexceptions" ],
            "sources": [
                "src/module.cc",
            ],
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")",
            ],
            "defines": [
                "NAPI_CPP_EXCEPTIONS"
            ],
        }
    ]
}
