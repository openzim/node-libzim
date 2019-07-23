node-libzim
=======================

![npm](https://img.shields.io/npm/v/@openzim/libzim.svg)
[![Build Status](https://travis-ci.org/openzim/node-libzim.svg?branch=master)](https://travis-ci.org/openzim/node-libzim)
[![CodeFactor](https://www.codefactor.io/repository/github/openzim/node-libzim/badge)](https://www.codefactor.io/repository/github/openzim/node-libzim)
![NPM](https://img.shields.io/npm/l/@openzim/libzim.svg)

> This package was built using [nbind](https://github.com/charto/nbind)

## Dependencies
On Linux, the package will download a `libzim` binary. On MacOS you'll need to install `libzim` separately ([see here](https://github.com/openzim/libzim/)).

## Usage

```
npm i libzim
```

### Writing a Zim file
```javascript
// write.js
const { ZimArticle, ZimCreator } = require("@openzim/libzim");

(async () => {

    console.info('Starting');
    const creator = new ZimCreator({ fileName: 'test.zim' }, { welcome: 'index.html' });

    for (let i = 100; i > 0; i--) {
        const a = new ZimArticle({ url: `file${i}`, data: `Content ${i}` });
        await creator.addArticle(a);
    }

    const welcome = new ZimArticle({ url: `index.html`, data: `<h1>Welcome!</h1>` });
    await creator.addArticle(welcome);

    await creator.finalise();

    console.log('Done Writing');

})();
```

### Reading a Zim file
```javascript
// read.js

const { ZimArticle, ZimReader } = require("@openzim/libzim");

(async () => {

    const zimFile = new ZimReader({ fileName: 'test.zim' });

    const suggestResults = await zimFile.suggest('Content');
    console.info(`Suggest Results:`, suggestResults);

    const searchResults = await zimFile.search('Content');
    console.info(`Search Results:`, searchResults);

    const article3Content = await zimFile.getArticleByUrl('./file3');
    console.info(`Article by url (./file3):`, article3Content);

})();

```

License
=======
GPLv3 - [./LICENSE](./LICENSE)
