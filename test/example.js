// Example from the README.
'use strict';
var assert = require('assert');
var path = require('path');

describe('Example from the README', function() {
  var zim = require('../');
  var dump = require('../bin/zimdump.js');

  // Construct-only functions are not supported in old node versions.
  if (zim._noConstruct_) { return; }

  var padding = new Buffer(512);

  class TestArticle extends zim.writer.Article {
    constructor(id) {
      super();
      this._id = id;
      var data = 'this is article ' + id + '\n';
      while (data.length < 512) {
        data += Math.random() + '\n';
      }
      this._data = new Buffer(data, 'utf8');
    }
    getAid() { return this._id; }
    getNamespace() { return 'A'; }
    getUrl() { return this._id; }
    getTitle() { return this._id; }
    isRedirect() { return false; }
    getMimeType() { return 'text/plain'; }
    getRedirectAid() { return ''; }
    getData() {
      return new zim.Blob(this._data);
    }
  }

  class TestArticleSource extends zim.writer.ArticleSource {
    constructor(max, szfunc) {
      super();
      var maxx = (max === undefined) ? 16 : max;
      this._next = 0;
      this._articles = [];
      this.getCurrentSize = szfunc;
      for (var n = 0; n < maxx ; n++) {
        this._articles[n] = new TestArticle('' + (n + 1));
      }
    }
    getNextArticle() {
      console.log('After ' + this._next + ' articles:',
                  this.getCurrentSize(), 'bytes');
      return this._articles[this._next++];
    }
    getUuid() {
      var uuid = zim.Uuid.generate();
      console.log('Generating UUID: ' + uuid);
      return uuid;
    }
  }
  it('should write a file named foo.zim', function() {
    var c = new zim.writer.ZimCreator();
    c.setMinChunkSize(1);
    var szfunc = function() { return c.getCurrentSize(); };
    var src = new TestArticleSource(8, szfunc);
    var fname = path.join(__dirname, 'foo.zim');
    c.create(fname, src);
    // Verify file is present.
    dump.main(['-F', '-l', '-v', fname]);
    // Verify that files are accurate.
    var zf = new zim.File(fname);
    src._articles.forEach(function(a) {
      var data = a._data;
      var aa = zf.getArticleByTitle(a.getNamespace(), a.getTitle());
      assert.deepEqual(aa.getData().data(), data);
    });
  });
});
