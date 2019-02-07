require('dotenv').config();
const axios = require('axios');
const mkdirp = require('mkdirp');
const exec = require('exec-then');
const fs = require('fs');

mkdirp.sync('./download');

// const url = `http://download.openzim.org/release/libzim/libzim-${process.env.LIBZIM_VERSION}.tar.xz`;
const url = `https://download.openzim.org/nightly/2019-02-06/libzim_linux-x86_64-2019-02-06.tar.gz`;

console.info(`Downloading Libzim Binaries from: `, url);
const dlFile = `./download/libzim-${process.env.LIBZIM_VERSION}.tar.gz`;

try {
    fs.statSync(dlFile);
    console.warn(`File [${dlFile}] already exists, not downloading`);
    process.exit(0);
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