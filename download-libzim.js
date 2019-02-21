require('dotenv').config();
const axios = require('axios');
const mkdirp = require('mkdirp');
const exec = require('exec-then');
const os = require('os');
const fs = require('fs');
const urlParser = require('url');

mkdirp.sync('./download');

const isLinux = os.type() === 'Linux';

const urls = [
    `http://download.openzim.org/release/libzim/libzim_linux-x86_64-${process.env.LIBZIM_VERSION}.tar.gz`,
].filter(a => a);

if (!isLinux) {
    console.warn(`\x1b[41m\n================================ README \n\nPre-built binaries only available on Linux for now...\nPlease ensure you have libzim installed globally on this machine:\n\n\thttps://github.com/openzim/libzim/\n\n================================\x1b[0m\n`);
}

for (let url of urls) {
    console.info(`Downloading Libzim from: `, url);
    const filename = urlParser.parse(url).pathname.split('/').slice(-1)[0];
    const dlFile = `./download/${filename}`;

    try {
        fs.statSync(dlFile);
        console.warn(`File [${dlFile}] already exists, not downloading`);
        return;
    } catch (err) { }

    axios({
        url,
        method: 'get',
        responseType: 'stream'
    })
        .then(function (response) {
            const ws = fs.createWriteStream(dlFile);
            return new Promise((resolve, reject) => {
                response.data
                    .pipe(ws)
                    .on('error', reject)
                    .on('close', resolve);
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
        .catch(err => {
            console.error(`Failed to download and extract file:`, err);
        });
}

exec(`ln -s lib/x86_64-linux-gnu/libzim.so.4 download/libzim.so.4`);