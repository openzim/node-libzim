import dotenv from "dotenv";
dotenv.config();
import { mkdirp } from "mkdirp";
import exec from "exec-then";
import os from "os";

mkdirp.sync("./build/Release");

const isMacOS = os.type() === "Darwin";
const isLinux = os.type() === "Linux";

if (!isMacOS && !isLinux) {
  console.warn(
    "\x1b[41m\n================================ README \n\nlibzim bundle with prebuilt binaries only available for macOS and Linux:\n\n\thttps://github.com/openzim/libzim/\n\n================================\x1b[0m\n",
  );
}

if (isLinux) {
  const rawArch = os.arch();
  let libDir;
  if (rawArch === "arm64") {
    libDir = "aarch64-linux-gnu";
  } else if (rawArch === "arm") {
    libDir = "arm-linux-gnueabihf";
  } else {
    libDir = "x86_64-linux-gnu";
  }

  console.info(`Copying libzim.so.9 from ${libDir} to build folder`);
  exec(`cp download/lib/${libDir}/libzim.so.9 build/Release/libzim.so.9`);
  exec("ln -sf build/Release/libzim.so.9 build/Release/libzim.so"); // convenience only, not required
}
if (isMacOS) {
  console.info("Copying libzim.9.dylib to build folder");
  exec("cp download/lib/libzim.9.dylib build/Release/libzim.9.dylib");
  exec("ln -sf build/Release/libzim.9.dylib build/Release/libzim.dylib"); // convienience only, not required
  console.info("Fixing rpath");
  exec(
    "install_name_tool -change libzim.9.dylib @loader_path/libzim.9.dylib build/Release/zim_binding.node",
  );
}
