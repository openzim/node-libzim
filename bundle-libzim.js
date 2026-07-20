import { exec as childProcessExec } from "node:child_process";
import { loadEnvFile } from "node:process";
import { promisify } from "node:util";
import fs from "node:fs";
import os from "node:os";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const PROJECT_ROOT_DIR = dirname(fileURLToPath(import.meta.url));

loadEnvFile(join(PROJECT_ROOT_DIR, ".env"));

fs.mkdirSync(join(PROJECT_ROOT_DIR, "build", "Release"), { recursive: true });

const isMacOS = os.type() === "Darwin";
const isLinux = os.type() === "Linux";

if (!isMacOS && !isLinux) {
  console.warn(
    "\x1b[41m\n================================ README \n\nlibzim bundle with prebuilt binaries only available for macOS and Linux:\n\n\thttps://github.com/openzim/libzim/\n\n================================\x1b[0m\n",
  );
}

const exec = promisify(childProcessExec);

if (isLinux) {
  const rawArch = os.arch();
  let libDir;
  if (rawArch === "arm64") {
    libDir = "lib64";
  } else if (rawArch === "arm") {
    libDir = "lib/arm-linux-gnueabihf";
  } else {
    libDir = "lib/x86_64-linux-gnu";
  }

  console.info(`Copying libzim.so.9 from ${libDir} to build folder`);
  await exec(`cp download/${libDir}/libzim.so.9 build/Release/libzim.so.9`, {
    cwd: PROJECT_ROOT_DIR,
  });
  await exec("ln -sf build/Release/libzim.so.9 build/Release/libzim.so", {
    cwd: PROJECT_ROOT_DIR,
  }); // convenience only, not required
}
if (isMacOS) {
  console.info("Copying libzim.9.dylib to build folder");
  await exec("cp download/lib/libzim.9.dylib build/Release/libzim.9.dylib", {
    cwd: PROJECT_ROOT_DIR,
  });
  await exec("ln -sf build/Release/libzim.9.dylib build/Release/libzim.dylib", {
    cwd: PROJECT_ROOT_DIR,
  }); // convienience only, not required
  console.info("Fixing rpath");
  await exec(
    "install_name_tool -change libzim.9.dylib @loader_path/libzim.9.dylib build/Release/zim_binding.node",
    { cwd: PROJECT_ROOT_DIR },
  );
}
