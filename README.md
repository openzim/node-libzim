node-libzim
=======================

[![Build Status](https://travis-ci.org/openzim/node-libzim.svg?branch=master)](https://travis-ci.org/openzim/node-libzim)
[![CodeFactor](https://www.codefactor.io/repository/github/openzim/node-libzim/badge)](https://www.codefactor.io/repository/github/openzim/node-libzim)

> This package was built using [nbind](https://github.com/charto/nbind)

## Dependencies
On Linux, the package will download a `libzim` binary. On MacOS you'll need to install `libzim` separately ([see here](https://github.com/openzim/libzim/)).

## Usage

```
npm i libzim
```

### Building a Zim file
```typescript
// write.ts
import { ZimArticle, ZimCreator } from "libzim";

(async () => {

    console.info('Starting');
    const creator = new ZimCreator('test.zim', { welcome: 'index.html', favicon: './favicon.png' });

    for (let i = 100; i > 0; i--) {
        const a = new ZimArticle(`file${i}`, `Content ${i}`);
        await creator.addArticle(a);
    }

    const welcome = new ZimArticle(`index.html`, `<h1>Welcome!</h1>`);
    await creator.addArticle(welcome);

    await creator.finalise();

    console.log('Done Writing');

})();
```

### Reading a Zim file
```typescript
// read.ts

import { ZimArticle, ZimReader } from "libzim";

(async () => {

    const zimFile = new ZimReader('test.zim');

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
GPLv2 - [./LICENSE](./LICENSE)
