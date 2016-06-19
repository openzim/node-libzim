'use strict';
var binary = require('node-pre-gyp');
var path = require('path');
var bindingPath =
  binary.find(path.resolve(path.join(__dirname, '..', 'package.json')));
var bindings = require(bindingPath);

Object.keys(bindings).forEach(function(k) {
  exports[k] = bindings[k];
});
