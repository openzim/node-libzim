// Fix for broken new.target in node 6.x
module.exports = function(zim) {
  class A extends zim.writer.Article { }
  if (new A() instanceof A)  { return; /* Bug-free! */ }

  zim.writer.Article = class Article extends zim.writer.Article {
    constructor() {
      super();
      Object.setPrototypeOf(this, new.target.prototype);
    }
  };
  zim.writer.ArticleSource =
  class ArticleSource extends zim.writer.ArticleSource {
    constructor() {
      super();
      Object.setPrototypeOf(this, new.target.prototype);
    }
  };
  zim.writer.Category = class Article extends zim.writer.Category {
    constructor() {
      super();
      Object.setPrototypeOf(this, new.target.prototype);
    }
  };
  zim.writer.ZimCreator = class Article extends zim.writer.ZimCreator {
    constructor() {
      super();
      Object.setPrototypeOf(this, new.target.prototype);
    }
  };
};
