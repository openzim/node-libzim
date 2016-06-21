'use strict';
var binary = require('node-pre-gyp');
var path = require('path');
var bindingPath =
  binary.find(path.resolve(path.join(__dirname, '..', 'package.json')));
var bindings = require(bindingPath);

Object.keys(bindings).forEach(function(k) {
  exports[k] = bindings[k];
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

// Workaround for old node.
if (/v1[.]8[.]/.test(process.version) || /v0[.]/.test(process.version)) {
  exports._noConstruct_ = true;
} else {
  try {
    require('./node6hack.js')(exports);
  } catch (e) { /* Pre-ES6 node, ignore */ }
}
