node-libzim
===========

This is the Node.js binding to the
[libzim](https://github.com/openzim/libzim/). Read and write
[ZIM](https://openzim.org) files easily in Javascript.

[![npm](https://img.shields.io/npm/v/@openzim/libzim.svg)](https://www.npmjs.com/package/@openzim/libzim)
[![Build Status](https://github.com/openzim/node-libzim/workflows/CI/badge.svg?branch=main)](https://github.com/openzim/node-libzim/actions?query=branch%3Amain)
[![codecov](https://codecov.io/gh/openzim/node-libzim/branch/main/graph/badge.svg)](https://codecov.io/gh/openzim/node-libzim)
[![CodeFactor](https://www.codefactor.io/repository/github/openzim/node-libzim/badge)](https://www.codefactor.io/repository/github/openzim/node-libzim)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Join Slack](https://img.shields.io/badge/Join%20us%20on%20Slack%20%23mwoffliner-2EB67D)](https://slack.kiwix.org)

## Dependencies

This package relies on [node-addon-api](https://github.com/nodejs/node-addon-api) / n-api.

On GNU/Linux & macOS, the package will download a `libzim` binary. On
other OSes you will need to install `libzim` separately ([see
here](https://github.com/openzim/libzim/)).

## Usage

```bash
npm i @openzim/libzim
```

### Writing a ZIM file
```javascript
// write.js
import { Creator, StringItem } from "@openzim/libzim";

(async () => {
    console.info('Starting');
    const outFile = "./test.zim";
    const creator = new Creator()
        .configNbWorkers(1)
        .configIndexing(true, "en")
        .configClusterSize(2048)
        .startZimCreation(outFile);

    for (let i = 0; i < 100; i++) {
        const item = new StringItem(
            `file${i}`,                       // path url
            "text/plain",                     // content-type
            `Title ${i}`,                     // title
            {FRONT_ARTICLE: 1, COMPRESS: 1},  // hint option flags
            `<h1>Content / Data ${i}</h1>`    // content
        );
        await creator.addItem(item);
    }

    creator.setMainPath("file0");
    await creator.finishZimCreation();

    console.log('Done Writing');
})();
```

### Reading a ZIM file
```javascript
// read.js
import { Archive, SuggestionSearcher, Searcher } from "@openzim/libzim";

(async () => {
    const outFile = "./test.zim";
    const archive = new Archive(outFile);
    console.log(`Archive opened: main entry path - ${archive.mainEntry.path}`);

    for (const entry of archive.iterByPath()) {
        console.log(`entry: ${entry.path} - ${entry.title}`);
    }

    const suggestionSearcher = new SuggestionSearcher(archive);
    const suggestion = suggestionSearcher.suggest('laborum');
    let results = suggestion.getResults(0, 10);
    console.log("Suggestion results:");
    for(const entry of results) {
        console.log(`\t- ${entry.path} - ${entry.title}`);
    }

    const searcher = new Searcher(archive);
    const search = searcher.search(new Query('rem'));
    results = search.getResults(0, 10);
    console.log("Search results:");
    for(const entry of results) {
        console.log(`\t- ${entry.path} - ${entry.title}`);
    }


    const entry = await archive.getEntryByPath("A/laborum");
    const item = entry.item;
    const blob = item.data;
    console.info(`Entry by url (laborum):`, blob.data);
    delete archive;
})();

```

## License

[GPLv3](https://www.gnu.org/licenses/gpl-3.0) or later, see
[LICENSE](LICENSE) for more details.
