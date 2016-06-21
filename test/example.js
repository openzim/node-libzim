// Example from the README.
'use strict';
var path = require('path');

describe('Example from the README', function() {
  var zim = require('../');

  // Construct-only functions are not supported in old node versions.
  if (zim._noConstruct_) { return; }

  var padding = new Buffer(512);

  class TestArticle extends zim.writer.Article {
    constructor(id) {
      super();
      this._id = id;
      this._data = new Buffer('this is article ' + id + '\n' + padding);
    }
    getAid() { return this._id; }
    getNamespace() { return 'A'; }
    getUrl() { return this._id; }
    getTitle() { return this._id; }
    isRedirect() { return false; }
    getMimeType() { return 'text/plain'; }
    getRedirectAid() { return ''; }
    data() {
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
    getData(aid) {
      return this._articles[(+aid) - 1].data();
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
    c.create(path.join(__dirname, 'foo.zim'), src);
    // XXX verify file is present.
  });
});
