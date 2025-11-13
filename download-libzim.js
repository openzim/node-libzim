import dotenv from "dotenv";
dotenv.config();
import axios from "axios";
import { mkdirp } from "mkdirp";
import exec from "exec-then";
import os from "os";
import fs from "fs";

mkdirp.sync("./download");

console.info("os.type() is:", os.type());
console.info("os.arch() is:", os.arch());
const isMacOS = os.type() === "Darwin";
const isLinux = os.type() === "Linux";
const rawArch = os.arch();

const isAvailableArch =
  rawArch === "x64" || rawArch === "arm" || rawArch === "arm64";

if (!isMacOS && !isLinux) {
  console.warn(
    `\x1b[41m\n================================ README \n\nPre-built binaries only available on Linux and MacOS for now...\nPlease ensure you have libzim installed globally on this machine:\n\n\thttps://github.com/openzim/libzim/\n\n================================\x1b[0m\n`,
  );
}
if (!isAvailableArch) {
  console.warn(
    `\x1b[41m\n================================ README \n\nPre-built binaries only available on x86_64, arm and arm64 for now...\nPlease ensure you have libzim installed globally on this machine:\n\n\thttps://github.com/openzim/libzim/\n\n================================\x1b[0m\n`,
  );
}

let osPrefix = isMacOS ? "macos" : "linux";
let osArch = isLinux ? "x86_64" : "x86_64";

if (rawArch !== "x64") {
  if (isLinux) {
    osArch = rawArch === "arm64" ? "aarch64-manylinux" : "armhf";
  } else {
    osArch = rawArch;
  }
}

const urls = [
  `https://download.openzim.org/release/libzim/libzim_${osPrefix}-${osArch}-${process.env.LIBZIM_VERSION}.tar.gz`,
].filter((a) => a);

for (let url of urls) {
  console.info(`Downloading Libzim from: `, url);
  const filename = new URL(url).pathname.split("/").slice(-1)[0];
  const dlFile = `./download/${filename}`;

  try {
    fs.statSync(dlFile);
    console.warn(`File [${dlFile}] already exists, not downloading`);
    break;
  } catch (err) {
    //
  }

  axios({
    url,
    method: "get",
    responseType: "stream",
  })
    .then(function (response) {
      const ws = fs.createWriteStream(dlFile);
      return new Promise((resolve, reject) => {
        response.data.pipe(ws).on("error", reject).on("close", resolve);
      });
    })
    .then(() => {
      const cmd = `tar --strip-components 1 -xf ${dlFile} -C ./download`;
      console.log(`Running Extract:`, `[${cmd}]`);
      return exec(cmd);
    })
    .then(() => {
      console.info(`Successfully downloaded and extracted file`);
    })
    .catch((err) => {
      console.error(`Failed to download and extract file:`, err);
    });
}
