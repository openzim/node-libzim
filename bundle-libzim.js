import dotenv from 'dotenv'
dotenv.config()
import { sync } from 'mkdirp';
import exec from 'exec-then';
import { type } from 'os';

sync('./build/Release');

const isMacOS = type() === 'Darwin'
const isLinux = type() === 'Linux'

if (!isMacOS && !isLinux) {
    console.warn('\x1b[41m\n================================ README \n\nlibzim bundle with prebuilt binaries only available for macOS and Linux:\n\n\thttps://github.com/openzim/libzim/\n\n================================\x1b[0m\n');
}

if (isLinux) {
    console.info('Copying libzim.so.8 to build folder')
    exec('cp download/lib/x86_64-linux-gnu/libzim.so.8 build/Release/libzim.so.8')
    exec('ln -sf build/Release/libzim.so.8 build/Release/libzim.so')  // convienience only, not required
}
if (isMacOS) {
    console.info('Copying libzim.8.dylib to build folder');
    exec('cp download/lib/libzim.8.dylib build/Release/libzim.8.dylib')
    exec('ln -sf build/Release/libzim.8.dylib build/Release/libzim.dylib')   // convienience only, not required
    console.info('Fixing rpath')
    exec('install_name_tool -change libzim.8.dylib @loader_path/libzim.8.dylib build/Release/zim_binding.node')
}
