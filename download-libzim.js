import { exec } from "node:child_process";
import fs from "node:fs";
import os from "node:os";
import { basename, dirname, join } from "node:path";
import { loadEnvFile } from "node:process";
import { Readable } from "node:stream";
import { pipeline } from "node:stream/promises";
import { fileURLToPath } from "node:url";
import { promisify } from "node:util";

const PROJECT_ROOT_DIR = dirname(fileURLToPath(import.meta.url));
const DOWNLOAD_DIR = join(PROJECT_ROOT_DIR, "download");

loadEnvFile(join(PROJECT_ROOT_DIR, ".env"));

fs.mkdirSync(DOWNLOAD_DIR, { recursive: true });

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

const osPrefix = isMacOS ? "macos" : "linux";
let osArch = isLinux ? "x86_64" : "x86_64";

if (rawArch !== "x64") {
  if (isLinux) {
    osArch = rawArch === "arm64" ? "aarch64-manylinux" : "armhf";
  } else {
    osArch = rawArch;
  }
}

const url = `https://download.openzim.org/release/libzim/libzim_${osPrefix}-${osArch}-${process.env.LIBZIM_VERSION}.tar.gz`;

console.info(`Downloading Libzim from: `, url);
const filename = basename(new URL(url).pathname);
const dlFile = join(DOWNLOAD_DIR, filename);

try {
  fs.statSync(dlFile);
  console.warn(`File [${dlFile}] already exists, not downloading`);
} catch {
  //
}

const response = await fetch(url);

if (!response.body) {
  throw new Error("Response body is missing");
}

await pipeline(Readable.fromWeb(response.body), fs.createWriteStream(dlFile));

const cmd = `tar --strip-components 1 -xf ${dlFile} -C ${DOWNLOAD_DIR}`;
console.log(`Running Extract:`, `[${cmd}]`);

try {
  await promisify(exec)(cmd);
  console.info(`Successfully downloaded and extracted file`);
} catch (err) {
  console.error(`Failed to download and extract file:`, err);
}
