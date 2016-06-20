// Example from the README.
'use strict';

describe('Example from the README', function() {
  var zim = require('../');
  class TestArticle extends zim.writer.Article {
    constructor(id) {
      super();
      this._id = id;
      this._data = new Buffer('this is article ' + id);
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
    constructor(max) {
      super();
      var maxx = (max === undefined) ? 16 : max;
      this._next = 0;
      this._articles = [];
      for (var n = 0; n < maxx ; n++) {
        this._articles[n] = new TestArticle('' + (n + 1));
      }
    }
    getNextArticle() {
      return this._articles[this._next++];
    }
    getData(aid) {
      return this._articles[(+aid) - 1].data();
    }
  }
  it('should write a file named foo.zim', function() {
    var c = new zim.writer.ZimCreator();
    var src = new TestArticleSource();
    c.create('foo.zim', src);
    // XXX verify file is present.
  });
});
