'use strict';
var binary = require('node-pre-gyp');
var path = require('path');
var bindingPath =
  binary.find(path.resolve(path.join(__dirname, '..', 'package.json')));
var bindings = require(bindingPath);

Object.keys(bindings).forEach(function(k) {
  Object.defineProperty(
    exports, k, Object.getOwnPropertyDescriptor(bindings, k)
  );
});

// Reimplement default shouldCompress fallback to ensure we get a chance
// to dispatch through the JS proxy.
exports.writer.Article.prototype.shouldCompress = function() {
  var mimeType = this.getMimeType(); // Virtual dispatch!
  return mimeType !== 'image/jpeg' &&
    mimeType !== 'image/png' &&
    mimeType !== 'image/tiff' &&
    mimeType !== 'image/gif' &&
    mimeType !== 'application/zip';
};

// Implement iterator for zim::File
var FileIterator = function FileIterator(file, idx, type) {
  this._file = file;
  this._idx = idx;
  this._type = type;
};
FileIterator.prototype.getIndex = function getIndex() { return this._idx; };
FileIterator.prototype.getFile = function getFile() { return this._file; };
FileIterator.prototype.hasNext = function() {
  return this._idx < this._file.getCountArticles();
};
FileIterator.prototype.get = function() {
  if (this._type === 'article') {
    return this._file.getArticle(this._idx);
  }
  return this._file.getArticleByTitle(this._idx);
};
FileIterator.prototype.next = function() {
  if (!this.hasNext()) {
    return { value: undefined, done: true };
  }
  var a = this.get();
  this._idx += 1;
  return { value: a, done: false };
};
exports.File.prototype.iterator = function iterator(idx) {
  return new FileIterator(this, idx || 0, 'article');
};
exports.File.prototype.iteratorByTitle = function iteratorByTitle(idx) {
  return new FileIterator(this, idx || 0, 'title');
};
// Our iterators return undefined if the value is not found.
exports.File.prototype.find = function find(ns, url) {
  if (ns && !url) {
    if (url.length < 2 || url[1] !== '/') { return; /* "not found" */ }
    return this.find(url[0], url.slice(2));
  }
  return this._find(ns, url, 'article');
};
exports.File.prototype.findByTitle = function findByTitle(ns, title) {
  return this._find(ns, title, 'title');
};
exports.File.prototype._find = function(ns, url, type) {
  var l = this.getNamespaceBeginOffset(ns);
  var u = this.getNamespaceEndOffset(ns);
  var cmp = function(a,b) { return (a === b) ? 0 : (a < b) ? -1 : +1; };
  var self = this;
  var d, tyget, tycmp;
  if (type === 'article') {
    tyget = function(i) { return self.getDirent(i); };
    tycmp = function(d) { return cmp(url, d.getUrl()); };
  } else {
    tyget = function(i) { return self.getDirentByTitle(i); };
    tycmp = function(d) { return cmp(url, d.getTitle()); };
  }
  if (l === u) { return; /* Namespace not found */ }
  while (u - l > 1) {
    var p = (u + l) >> 1;
    d = tyget(p);
    var c = cmp(ns, d.getNamespace());
    if (c === 0) {
      c = tycmp(d);
    }
    if (c < 0) {
      u = p;
    } else if (c > 0) {
      l = p;
    } else {
      return new FileIterator(this, p, type);
    }
  }
  d = tyget(l);
  if (tycmp(d) === 0) {
    return new FileIterator(this, l, type);
  }
  return;
};
if (global.Symbol && global.Symbol.iterator) {
  exports.File.prototype[global.Symbol.iterator] =
    exports.File.prototype.iterator;
}

// Workaround for old node.
if (/v1[.]8[.]/.test(process.version) || /v0[.]/.test(process.version)) {
  exports._noConstruct_ = true;
} else {
  try {
    require('./node6hack.js')(exports);
  } catch (e) { /* Pre-ES6 node, ignore */ }
}
