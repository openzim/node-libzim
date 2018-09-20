// Cluster tests.
'use strict';
var assert = require('assert');

describe('zim::Cluster', function() {
  var zim = require('../');
  if (zim._noConstruct_) { return; }
  it('should add blobs correctly', function() {
    var c = new zim.Cluster();
    c.setCompression(zim.zimcompNone);
    assert.equal(c.getCompression(), zim.zimcompNone);
    assert.equal(c.isCompressed(), false);
    assert.equal(c.count(), 0);
    assert.equal(c.size(), 4);
    assert.equal(c.good(), true);
    c.addBlob(new Buffer('abcd', 'utf8'));
    assert.equal(c.getCompression(), zim.zimcompNone);
    assert.equal(c.count(), 1);
    assert.equal(c.size(), 12);
    assert.equal(c.good(), true);
    assert.equal(c.getBlobSize(0), 4);
    c.addBlob(new Buffer('1234567890', 'utf8'));
    assert.equal(c.getCompression(), zim.zimcompNone);
    assert.equal(c.count(), 2);
    assert.equal(c.size(), 26);
    assert.equal(c.good(), true);
    assert.equal(c.getBlobSize(0), 4);
    assert.equal(c.getBlobSize(1), 10);
    c.clear();
    assert.equal(c.getCompression(), zim.zimcompNone);
    assert.equal(c.count(), 0);
    assert.equal(c.size(), 4);
  });
  it('should preserve compression type across clear', function() {
    var c = new zim.Cluster();
    c.setCompression(zim.zimcompLzma);
    c.addBlob(new Buffer('testtest', 'utf8'));
    c.clear();
    assert.equal(c.getCompression(), zim.zimcompLzma);
  });
});
