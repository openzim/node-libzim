import dotenv from 'dotenv'
dotenv.config()
import axios from 'axios';
import { sync } from 'mkdirp';
import exec from 'exec-then';
import { type, arch } from 'os';
import { statSync, createWriteStream } from 'fs';
import { parse } from 'url';

sync('./download');

const isMacOS = type() === 'Darwin'
const isLinux = type() === 'Linux'
const rawArch = arch()
const isAvailableArch = rawArch === 'x64' || rawArch === 'arm' || rawArch === 'arm64'

if (!isMacOS && !isLinux) {
    console.warn('\x1b[41m\n================================ README \n\nPre-built binaries only available on Linux and MacOS for now...\nPlease ensure you have libzim installed globally on this machine:\n\n\thttps://github.com/openzim/libzim/\n\n================================\x1b[0m\n');
}
if (!isAvailableArch) {
    console.warn('\x1b[41m\n================================ README \n\nPre-built binaries only available on x86_64, arm and arm64 for now...\nPlease ensure you have libzim installed globally on this machine:\n\n\thttps://github.com/openzim/libzim/\n\n================================\x1b[0m\n');
}

const osPrefix = (isMacOS) ? 'macos' : 'linux';
let osArch = (isLinux) ? 'x86_64-bionic' : 'x86_64';

if (rawArch !== 'x64'){
    if (isLinux) {
        osArch = rawArch === 'arm64' ? 'aarch64-bionic' : 'armhf'
    } else {
        osArch = rawArch
    }
}

const urls = [
    `https://download.openzim.org/release/libzim/libzim_${osPrefix}-${osArch}-${process.env.LIBZIM_VERSION}.tar.gz`,
].filter(a => a);

for (const url of urls) {
    console.info('Downloading Libzim from: ', url);
    const filename = parse(url).pathname.split('/').slice(-1)[0];
    const dlFile = `./download/${filename}`;

    try {
        statSync(dlFile);
        console.warn(`File [${dlFile}] already exists, not downloading`);
        return;
    } catch (err) {
        //
    }

    axios({
        url,
        method: 'get',
        responseType: 'stream'
    })
        .then(function (response) {
            const ws = createWriteStream(dlFile);
            return new Promise((resolve, reject) => {
                response.data
                    .pipe(ws)
                    .on('error', reject)
                    .on('close', resolve);
            });
        })
        .then(() => {
            const cmd = `tar --strip-components 1 -xf ${dlFile} -C ./download`;
            console.log('Running Extract:', `[${cmd}]`);
            return exec(cmd);
        })
        .then(() => {
            console.info('Successfully downloaded and extracted file');
        })
        .catch(err => {
            console.error('Failed to download and extract file:', err);
        });
}
