# libzim
[![NPM][NPM1]][NPM2]

[![Build Status][1]][2] [![dependency status][3]][4] [![dev dependency status][5]][6]

The node `libzim` package binds to the `zimlib` library from [OpenZIM][]
to allow read/write access to ZIM files.

# USAGE
We follow the `zimlib` API closely where possible.

```js
var zim = require('libzim');

class TestArticle extends zim.writer.Article {
  constructor(id) {
    super();
    this._id = id;
    this._data = new Buffer("this is article " + id);
  }
  getAid() { return this._id; }
  getNamespace() { return 'A'; }
  getUrl() { return this._id; }
  getTitle() { return this._id; }
  isRedirect() { return false; }
  getMimeType() { return "text/plain"; }
  getRedirectAid() { return ""; }
  getData() {
    return new zim.Blob(this._data);
  }
}

class TestArticleSource extends zim.writer.ArticleSource {
  constructor(max = 16, szfunc) {
    super();
    this._next = 0;
    this._articles = [];
    this.getCurrentSize = szfunc;
    for (var n = 0; n < max ; n++) {
      this._articles[n] = new TestArticle("" + (n+1));
    }
  }
  getNextArticle() {
    console.log('After ' + this._next + ' articles:',
                this.getCurrentSize(), 'bytes');
    return this._articles[this._next++];
  }
}

c = new zim.writer.ZimCreator();
src = new TestArticleSource(8, () => c.getCurrentSize());
c.create("foo.zim", src);
```

# API

# INSTALLING
You can use [`npm`](https://github.com/isaacs/npm) to download and install:

* The latest `libzim` package: `npm install libzim`

* GitHub's `master` branch: `npm install https://github.com/cscott/node-libzim/tarball/master`

In both cases the module is automatically built with npm's internal
version of `node-gyp`, and thus your system must meet [node-gyp's
requirements](https://github.com/TooTallNate/node-gyp#installation).

It is also possible to make your own build of `libzim` from its source
instead of its npm package ([see below](#building-from-the-source)).

# BUILDING FROM THE SOURCE

Unless building via `npm install` (which uses its own `node-gyp`) you
will need `node-gyp` installed globally:

    npm install node-gyp -g

The `libzim` module depends only on the `zimlib` library from
OpenZIM. By default, an internal/bundled copy of OpenZIM's `zimlib`
will be built and statically linked, so an externally installed
`zimlib` is not required.

If you wish to install against an external `zimlib` then you need to
pass the `--libzim` argument to `node-gyp`, `npm install` or the
`configure` wrapper.

    ./configure --libzim=external
    make

Or, using the node-gyp directly:

     node-gyp --libzim=external rebuild

Or, using npm:

     npm install --libzim=external

If building against an external `zimlib` make sure to have the
development headers available. Mac OS X ships with these by
default. If you don't have them installed, install the `-dev` package
with your package manager, e.g. `apt-get install libzim-dev` for
Debian/Ubuntu.

# TESTING

[mocha](https://github.com/visionmedia/mocha) is required to run unit tests.

    npm install mocha
    npm test


# CONTRIBUTORS

* [C. Scott Ananian](https://github.com/cscott)

# RELATED PROJECTS

* [OpenZIM][]

# LICENSE
Copyright (c) 2016 C. Scott Ananian.

`libzim` is licensed using the same license as the zimlib library
in [OpenZIM][]: GPLv2.

[OpenZIM]:     http://openzim.org/

[NPM1]: https://nodei.co/npm/libzim.png
[NPM2]: https://nodei.co/npm/libzim/

[1]: https://travis-ci.org/cscott/node-libzim.png
[2]: https://travis-ci.org/cscott/node-libzim
[3]: https://david-dm.org/cscott/node-libzim.png
[4]: https://david-dm.org/cscott/node-libzim
[5]: https://david-dm.org/cscott/node-libzim/dev-status.png
[6]: https://david-dm.org/cscott/node-libzim#info=devDependencies
