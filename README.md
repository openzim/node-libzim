node-libzim
===========

This is the Node.js binding to the
[libzim](https://github.com/openzim/libzim/issues/264). Read and write
[ZIM](https://openzim.org) files easily in Javascript.

[![npm](https://img.shields.io/npm/v/@openzim/libzim.svg)](https://www.npmjs.com/package/@openzim/libzim)
[![Build Status](https://travis-ci.com/openzim/node-libzim.svg?branch=master)](https://travis-ci.com/openzim/node-libzim)
[![CodeFactor](https://www.codefactor.io/repository/github/openzim/node-libzim/badge)](https://www.codefactor.io/repository/github/openzim/node-libzim)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

## Dependencies

This package relies on [nbind](https://github.com/charto/nbind).

On Linux, the package will download a `libzim` binary. On MacOS you'll need to install `libzim` separately ([see here](https://github.com/openzim/libzim/)).

## Usage

```bash
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

    const zimFile = new ZimReader(path.join(__dirname, '../test.zim'));

    const suggestResults = await zimFile.suggest('laborum');
    console.info(`Suggest Results:`, suggestResults);

    const searchResults = await zimFile.search('rem');
    console.info(`Search Results:`, searchResults);

    const readArticleContent = await zimFile.getArticleByUrl('A/laborum');
    console.info(`Article by url (laborum):`, readArticleContent);

    await zimFile.destroy();

})();

```

## License

[GPLv3](https://www.gnu.org/licenses/gpl-3.0) or later, see
[LICENSE](LICENSE) for more details.
