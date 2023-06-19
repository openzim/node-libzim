require("dotenv").config();
const axios = require("axios");
const mkdirp = require("mkdirp");
const exec = require("exec-then");
const os = require("os");
const fs = require("fs");
const urlParser = require("url");

mkdirp.sync("./download");

const isMacOS = os.type() === "Darwin";
const isLinux = os.type() === "Linux";
const rawArch = os.arch();
const isAvailableArch =
  rawArch === "x64" || rawArch === "arm" || rawArch === "arm64";

if (!isMacOS && !isLinux) {
  console.warn(
    `\x1b[41m\n================================ README \n\nPre-built binaries only available on Linux and MacOS for now...\nPlease ensure you have libzim installed globally on this machine:\n\n\thttps://github.com/openzim/libzim/\n\n================================\x1b[0m\n`
  );
}
if (!isAvailableArch) {
  console.warn(
    `\x1b[41m\n================================ README \n\nPre-built binaries only available on x86_64, arm and arm64 for now...\nPlease ensure you have libzim installed globally on this machine:\n\n\thttps://github.com/openzim/libzim/\n\n================================\x1b[0m\n`
  );
}

let osPrefix = isMacOS ? "macos" : "linux";
let osArch = isLinux ? "x86_64-bionic" : "x86_64";

if (rawArch !== "x64") {
  if (isLinux) {
    osArch = rawArch === "arm64" ? "aarch64-bionic" : "armhf";
  } else {
    osArch = rawArch;
  }
}

const urls = [
  `https://download.openzim.org/release/libzim/libzim_${osPrefix}-${osArch}-${process.env.LIBZIM_VERSION}.tar.gz`,
].filter((a) => a);

for (let url of urls) {
  console.info(`Downloading Libzim from: `, url);
  const filename = urlParser.parse(url).pathname.split("/").slice(-1)[0];
  const dlFile = `./download/${filename}`;

  try {
    fs.statSync(dlFile);
    console.warn(`File [${dlFile}] already exists, not downloading`);
    return;
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
