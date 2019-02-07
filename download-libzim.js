require('dotenv').config();
const axios = require('axios');
const mkdirp = require('mkdirp');
const exec = require('exec-then');
const fs = require('fs');
const urlParser = require('url');

mkdirp.sync('./download');

const urls = [
    `http://download.openzim.org/release/libzim/libzim-${process.env.LIBZIM_VERSION}.tar.xz`, // Headers
    `http://download.openzim.org/nightly/2019-02-07/libzim_linux-x86_64-2019-02-07.tar.gz`, // Binary
];

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

exec(`ln -s libzim.so.4.0.4 download/libzim.so.4`);