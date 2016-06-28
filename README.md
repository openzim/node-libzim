# libzim
[![NPM][NPM1]][NPM2]

[![Build Status][1]][2] [![dependency status][3]][4] [![dev dependency status][5]][6]

The node `libzim` package binds to the `zimlib` library from [OpenZIM][]
to allow read/write access to ZIM files.

## USAGE

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

## INSTALLING
You can use [`npm`](https://github.com/isaacs/npm) to download and install:

* The latest `libzim` package: `npm install libzim`

* GitHub's `master` branch: `npm install https://github.com/cscott/node-libzim/tarball/master`

In both cases the module is automatically built with npm's internal
version of `node-gyp`, and thus your system must meet [node-gyp's
requirements](https://github.com/TooTallNate/node-gyp#installation).

It is also possible to make your own build of `libzim` from its source
instead of its npm package ([see below](#building-from-the-source)).

## BUILDING FROM THE SOURCE

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

## TESTING

[mocha](https://github.com/visionmedia/mocha) is required to run unit tests.

    npm install mocha
    npm test


## API
We follow the `zimlib` API closely where possible.

<hr>
### `zim.Article`
[`zim.Article`]: #zimarticle
A read-only object representing an individual article in a ZIM file.
#### `new zim.Article()`
Create a new placeholder `Article` object for which the `good` method will
return false.
#### `zim.Article#getDirent()` → [`zim.Dirent`][]
#### `zim.Article#getParameter()` → [`Buffer`]
#### `zim.Article#getTitle()` → `string`
#### `zim.Article#getUrl()` → `string`
#### `zim.Article#getLongUrl()` → `string`
#### `zim.Article#getLibraryMimeType()` → `number`
#### `zim.Article#getMimeType()` → `string`
#### `zim.Article#isRedirect()` → `boolean`
#### `zim.Article#isLinktarget()` → `boolean`
#### `zim.Article#isDeleted()` → `boolean`
#### `zim.Article#getNamespace()` → `string`
#### `zim.Article#getRedirectIndex()` → `number`
#### `zim.Article#getRedirectArticle()` → [`zim.Article`][]
#### `zim.Article#getArticleSize()` → `number`
#### `zim.Article#getCluster()` → [`zim.Cluster`][]
#### `zim.Article#getData()` → [`zim.Blob`][]
#### `zim.Article#getPage([boolean layout [, number maxRecurse]])` → `string`
#### `zim.Article#getIndex()` → `number`
#### `zim.Article#good()` → `boolean`
<hr>
### `zim.Blob`
[`zim.Blob`]: #zimblob
[`Buffer`]: https://nodejs.org/api/buffer.html

#### `new zim.Blob(Buffer data)`
Creates an object representing the data in the supplied node
[`Buffer`][].
#### `zim.Blob#data` → [`Buffer`][]
Returns a buffer object containing the data represented by this blob.
This buffer is live: changing the values in the buffer will change the
data in the blob.
#### `zim.Blob#size` → `number`
Returns the (uncompressed) number of bytes of data in this blob.
<hr>
### `zim.Cluster`
[`zim.Cluster`]: #zimcluster
#### `new zim.Cluster()`
#### `zim.Cluster#setCompression(number compressionType)` → `undefined`
#### `zim.Cluster#getCompression()` → `number`
#### `zim.Cluster#isCompressed()` → `boolean`
#### `zim.Cluster#getBlob(number n)` → [`zim.Blob`]
#### `zim.Cluster#getBlobSize(number n)` → `number`
#### `zim.Cluster#count()` → `number`
#### `zim.Cluster#size()` → `number`
#### `zim.Cluster#clear()` → `undefined`
#### `zim.Cluster#addBlob(zim.Blob|Buffer data)` → `undefined`
#### `zim.Cluster#good()` → `boolean`
<hr>
### `zim.Dirent`
[`zim.Dirent`]: #zimdirent
#### `new zim.Dirent()`
Create a new mutable directory entry object.
#### `zim.Dirent#isRedirect()` → `boolean`
#### `zim.Dirent#isLinktarget()` → `boolean`
#### `zim.Dirent#isDeleted()` → `boolean`
#### `zim.Dirent#isArticle()` → `boolean`
#### `zim.Dirent#getMimeType()` → `number`
See `zim.File#getMimeType(n)` to convert this to a string.
#### `zim.Dirent#getVersion()` → `number`
#### `zim.Dirent#setVersion(number v)` → `undefined`
#### `zim.Dirent#getClusterNumber()` → `number`
#### `zim.Dirent#getBlobNumber()` → `number`
#### `zim.Dirent#setCluster(number cluster, number blob)` → `undefined`
#### `zim.Dirent#getRedirectIndex()` → `number`
#### `zim.Dirent#getNamespace()` → `string`
#### `zim.Dirent#getTitle()` → `string`
#### `zim.Dirent#getUrl()` → `string`
#### `zim.Dirent#getLongUrl()` → `string`
#### `zim.Dirent#getParameter()` → [`Buffer`]
#### `zim.Dirent#getDirentSize()` → `number`
#### `zim.Dirent#setTitle(string title)` → `undefined`
#### `zim.Dirent#setUrl(string ns, string url)` → `undefined`
#### `zim.Dirent#setParameter(Buffer b)` → `undefined`
#### `zim.Dirent#setRedirect(number idx)` → `undefined`
#### `zim.Dirent#setMimeType(number mime)` → `undefined`
#### `zim.Dirent#setLinktarget()` → `undefined`
#### `zim.Dirent#setDeleted()` → `undefined`
#### `zim.Dirent#setArticle(number mime, number cluster, number blob)` → `undefined`

#### `number zim.Dirent.redirectMimeType`
Integer index used as the mime type for redirect articles.
#### `number zim.Dirent.linktargetMimeType`
Integer index used as the mime type for link target articles.
#### `number zim.Dirent.deletedMimeType`
Integer index used as the mime type for deleted articles.
<hr>
### `zim.File`
[`zim.File`]: #zimfile
#### `new zim.File(string filename)`
Open the given `filename` and return a `zim.File` object describing it.
#### `zim.File#getFilename()` → `string`
#### `zim.File#getFileheader()` → [`zim.Fileheader`]
#### `zim.File#getFilesize()` → `number`
#### `zim.File#getDirent(number idx)` → [`zim.Dirent`]
Returns the dirent with the given index in the URL index.
#### `zim.File#getDirentByTitle(number idx)` → [`zim.Dirent`]
Returns the dirent with the given index in the title index.
#### `zim.File#getCountArticles()` → `number`
The number of articles in the index.
(This will be the same value for both URL and title indices.)
#### `zim.File#getArticle(number idx)` → [`zim.Article`]
Returns the article with the given index in the URL index.
#### `zim.File#getArticle(string ns, string url)` → [`zim.Article`]
#### `zim.File#getArticleByUrl(string url)` → [`zim.Article`]
#### `zim.File#getArticleByTitle(number idx)` → [`zim.Article`]
Returns the article with the given index in the title index.
#### `zim.File#getArticleByTitle(string ns, string title)` → [`zim.Article`]
#### `zim.File#getCluster(number idx)` → [`zim.Cluster`]
#### `zim.File#getClusterOffset(number idx)` → `number`
#### `zim.File#getCountClusters()` → `number`
#### `zim.File#getBlob(number cluster, number blob)` → [`zim.Blob`]
#### `zim.File#getNamespaceBeginOffset(string ns)` → `number`
Index of the first entry in the given namespace.
(This will be the same value for both URL and title indices.)
#### `zim.File#getNamespaceEndOffset(string ns)` → `number`
Index of the first entry *not* in the given namespace.
(This will be the same value for both URL and title indices.)
#### `zim.File#getNamespaceCount(string ns)` → `number`
The number of articles in the given namespace.
#### `zim.File#getNamespaces()` → `string`
A string with one character per namespace in the file.
#### `zim.File#hasNamespace(string ns)` → `boolean`
#### `zim.File#good()` → `boolean`
#### `zim.File#getMTime()` → `number`
#### `zim.File#getChecksum()` → `string`
Returns the MD5 checksum as a 32 character hexadecimal number.
#### `zim.File#verify()` → `boolean`
Returns true iff the checksum validates.
#### `zim.File#iterator([number idx])` → [`zim.FileIterator`]
#### `zim.File#iteratorByTitle([number idx])` → [`zim.FileIterator`]
#### `zim.File#find(string url)` → [`zim.FileIterator`]
#### `zim.File#find(string ns, string url)` → [`zim.FileIterator`]
#### `zim.File#findByTitle(string ns, string title)` → [`zim.FileIterator`]
<hr>
### `zim.FileIterator`
[`zim.FileIterator`]: #zimfileiterator
#### `zim.FileIterator#getIndex()` → `number`
#### `zim.FileIterator#getFile()` → [`zim.File`]
#### `zim.FileIterator#hasNext()` → `boolean`
#### `zim.FileIterator#get()` → [`zim.Article`]
#### `zim.FileIterator#next()` → { [`zim.Article`] value, boolean done }
<hr>
### `zim.Fileheader`
[`zim.Fileheader`]: #zimfileheader
`Fileheader` objects can't be constructed directly; they are obtained only via
`zim.File#getFileheader()`.
#### `zim.Fileheader#getUuid()` → [`zim.Uuid`]
#### `zim.Fileheader#setUuid(zim.Uuid uuid)` → `undefined`
#### `zim.Fileheader#getArticleCount()` → `number`
#### `zim.Fileheader#setArticleCount(number count)` → `undefined`
#### `zim.Fileheader#getTitleIdxPos()` → `number`
#### `zim.Fileheader#setTitleIdxPos(number pos)` → `undefined`
#### `zim.Fileheader#getUrlPtrPos()` → `number`
#### `zim.Fileheader#setUrlPtrPos(number pos)` → `undefined`
#### `zim.Fileheader#getMimeListPos()` → `number`
#### `zim.Fileheader#setMimeListPos(number pos)` → `undefined`
#### `zim.Fileheader#getClusterCount()` → `number`
#### `zim.Fileheader#setClusterCount(number count)` → `undefined`
#### `zim.Fileheader#getClusterPtrPos()` → `number`
#### `zim.Fileheader#setClusterPtrPos(number pos)` → `undefined`
#### `zim.Fileheader#hasMainPage()` → `boolean`
#### `zim.Fileheader#getMainPage()` → `number`
#### `zim.Fileheader#setMainPage(number idx)` → `undefined`
#### `zim.Fileheader#hasLayoutPage()` → `boolean`
#### `zim.Fileheader#getLayoutPage()` → `number`
#### `zim.Fileheader#setLayoutPage(number idx)` → `undefined`
#### `zim.Fileheader#hasChecksum()` → `boolean`
#### `zim.Fileheader#getChecksumPos()` → `number`
#### `zim.Fileheader#setChecksumPos(number pos)` → `undefined`
<hr>
### `zim.Uuid`
[`zim.Uuid`]: #zimuuid
#### `new zim.Uuid()`
Create a new UUID which is all zeros.
#### `zim.Uuid.generate()` → `number`
Create a new UUID based on a hash of the time of day.
#### `zim.Uuid#data()` → [`Buffer`]
Returns a buffer with 16 bytes of UUID data.
#### `zim.Uuid#size()` → `number`
Returns 16.
#### `zim.Uuid#toString()` → `string`
Returns a pretty-printed UID.
<hr>
### `zim.writer.Article`
[`zim.writer.Article`]: #zimwriterarticle
An abstract base class describing an article for use by
[`zim.writer.ZimCreator`].
#### `zim.writer.Article#getAid()` → `string`
Abstract method.
#### `zim.writer.Article#getNamespace()` → `string`
Abstract method.
#### `zim.writer.Article#getUrl()` → `string`
Abstract method.
#### `zim.writer.Article#getTitle()` → `string`
Abstract method.
#### `zim.writer.Article#getVersion()` → `number`
Default implementation returns `0`.
#### `zim.writer.Article#isRedirect()` → `boolean`
Default implementation returns `false`.
#### `zim.writer.Article#isLinktarget()` → `boolean`
Default implementation returns `false`.
#### `zim.writer.Article#isDeleted()` → `boolean`
Default implementation returns `false`.
#### `zim.writer.Article#getMimeType()` → `string`
Abstract method.
#### `zim.writer.Article#shouldCompress()` → `boolean`
Default implementation returns `true` iff mime type is `image/jpeg`,
`image/png`, `image/tiff`, `image/gif`, or `application/zip`.
#### `zim.writer.Article#getRedirectAid()` → `string`
Default implemention returns empty string.
#### `zim.writer.Article#getParameter()` → [`Buffer`]
Default implemention returns empty buffer.
#### `zim.writer.Article#getNextCategory()` → `string`
Default implemention returns empty string.
#### `zim.writer.Article#getData()` → [`zim.Blob`]
Abstract method.
<hr>
### `zim.writer.ArticleSource`
[`zim.writer.ArticleSource`]: #zimwriterarticlesource
An abstract base class describing a collection of articles for use by
[`zim.writer.ZimCreator`].
#### `zim.writer.ArticleSource#setFilename(string filename)` → `undefined`
Default implemention ignores the provided filename.
#### `zim.writer.ArticleSource#getNextArticle()` → [`zim.writer.Article`]
Abstract method.
#### `zim.writer.ArticleSource#getUuid()` → [`zim.Uuid`]
Default implementation calls `zim.Uuid.generate()`.
#### `zim.writer.ArticleSource#getMainPage()` → `string`
Default implementation returns the empty string.
#### `zim.writer.ArticleSource#getLayoutPage()` → `string`
Default implementation returns the empty string.
#### `zim.writer.ArticleSource#getCategory()` → [`zim.writer.Category`]
Default implementation returns `null`.
<hr>
### `zim.writer.Category`
[`zim.writer.Category`]: #zimwritercategory
An abstract base class describing a category for use by
[`zim.writer.ZimCreator`].
#### `zim.writer.Category#getData()` → [`zim.Blob`]
Abstract method.
#### `zim.writer.Category#getUrl()` → `string`
Abstract method.
#### `zim.writer.Category#getTitle()` → `string`
Abstract method.
<hr>
### `zim.writer.ZimCreator`
[`zim.writer.ZimCreator`]: #zimwriterzimcreator
#### `new zim.writer.ZimCreator()`
#### `zim.writer.ZimCreator#create(string filename, zim.writer.ArticleSource src)` → `undefined`
Write a ZIM file with the given `filename` using the articles described
by `src`.
#### `zim.writer.ZimCreator#getMinChunkSize()` → `number`
#### `zim.writer.ZimCreator#setMinChunkSize(number s)` → `undefined`
#### `zim.writer.ZimCreator#getCurrentSize()` → `number`
Returns an approximation to the current size of the ZIM file.

## CONTRIBUTORS

* [C. Scott Ananian](https://github.com/cscott)

## RELATED PROJECTS

* [OpenZIM][]

## LICENSE
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
