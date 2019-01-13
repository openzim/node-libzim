libzim-binding
=======================

[![Build Status](https://travis-ci.org/ISNIT0/node-libzim.svg?branch=master)](https://travis-ci.org/ISNIT0/node-libzim)

> This package was build using [nbind](https://github.com/charto/nbind)

## Dependencies
You will need a copy of Libzim installed on your machine.
Libzim be be included in this library at some point in the future, but for now install it ([see here](https://github.com/openzim/libzim/)).

## Usage

```
npm i libzim-binding
```

### Building a Zim file
```typescript
// write.ts
import { ZimArticle, ZimCreator } from "libzim-binding";

(async () => {

    console.info('Starting');
    const creator = new ZimCreator('test.zim', { welcome: 'index.html', favicon: './favicon.png' });

    for (let i = 100; i > 0; i--) {
        const a = new ZimArticle(`file${i}`, `Content ${i}`);
        await creator.addArticle(a);
    }

    const welcome = new ZimArticle(`index.html`, `<h1>Welcome!</h1>`);
    await creator.addArticle(a);

    await creator.finalise();

    console.log('Done Writing');

})();
```

### Reading a Zim file
```typescript
// read.ts

import { ZimArticle, ZimReader } from "libzim-binding";

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
MIT - [./LICENSE](./LICENSE)
