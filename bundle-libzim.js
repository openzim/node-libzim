require('dotenv').config();
const mkdirp = require('mkdirp');
const exec = require('exec-then');
const os = require('os');

mkdirp.sync('./build/Release');

const isMacOS = os.type() === 'Darwin'
const isLinux = os.type() === 'Linux'

if (!isMacOS && !isLinux) {
    console.warn(`\x1b[41m\n================================ README \n\nlibzim bundle with prebuilt binaries only available for macOS and Linux:\n\n\thttps://github.com/openzim/libzim/\n\n================================\x1b[0m\n`);
}

if (isLinux) {
    console.info(`Copying libzim.so.6 to build folder`)
    exec(`cp download/lib/x86_64-linux-gnu/libzim.so.6 build/Release/libzim.so.6`)
    exec(`ln -sf build/Release/libzim.so.6 build/Release/libzim.so`)  // convienience only, not required
}
if (isMacOS) {
    console.info(`Copying libzim.6.dylib to build folder`);
    exec(`cp download/lib/libzim.6.dylib build/Release/libzim.6.dylib`)
    exec(`ln -sf build/Release/libzim.6.dylib build/Release/libzim.dylib`)   // convienience only, not required
    console.info(`Fixing rpath`)
    exec(`install_name_tool -change libzim.6.dylib @loader_path/libzim.6.dylib build/Release/zim_binding.node`)
}
