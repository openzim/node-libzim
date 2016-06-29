#!/usr/bin/env node
var zim = require('../');
var fs = require('fs');
var path = require('path');

/**
 * (Re-)Implementation of ZIM file dump tool, based on the node binding.
 */

var ZimDumper = module.exports = function ZimDumper(fname, titleSort) {
  this.file = new zim.File(fname);
  this.verbose = false;
  this.pos = titleSort ? this.file.iteratorByTitle() : this.file.iterator();
};
ZimDumper.prototype.setVerbose = function(sw) {
  this.verbose = (sw === undefined ? true : !!sw);
};
ZimDumper.prototype.log = function() {
  console.log.apply(console, arguments);
};
ZimDumper.prototype.printInfo = function() {
  var log = this.log.bind(this);
  var f = this.file;
  var fh = f.getFileheader();
  var i;
  log('count-articles:', f.getCountArticles());
  if (this.verbose) {
    var ns = f.getNamespaces();
    log('namespaces:', ns);
    for (i = 0; i < ns.length;i++) {
      log('namespace', ns[i], 'size:', f.getNamespaceCount(ns[i]));
    }
  }
  log('uuid:', '' + fh.getUuid());
  log('article count:', fh.getArticleCount());
  log('mime list pos:', fh.getMimeListPos());
  log('url ptr pos:', fh.getUrlPtrPos());
  log('title idx pos:', fh.getTitleIdxPos());
  log('cluster count:', fh.getClusterCount());
  log('cluster ptr pos:', fh.getClusterPtrPos());
  if (fh.hasChecksum()) {
    log('checksum pos:', fh.getChecksumPos());
    log('checksum:', f.getChecksum());
  } else {
    log('no checksum');
  }

  log('main page:', fh.hasMainPage() ? fh.getMainPage() : '-');
  log('layout page:', fh.hasLayoutPage() ? fh.getLayoutPage() : '-');
};
ZimDumper.prototype.printNsInfo = function(ch) {
  var log = this.log.bind(this);
  log('namespace', ch);
  log('lower bound idx:', this.file.getNamespaceBeginOffset(ch));
  log('upper bound idx:', this.file.getNamespaceEndOffset(ch));
};
ZimDumper.prototype.locateArticle = function(idx) {
  this.pos = this.file.iterator(idx);
};
ZimDumper.prototype.findArticle = function(ns, expr, title) {
  if (title) {
    this.pos = this.file.findByTitle(ns, expr);
  } else {
    this.pos = this.file.find(ns, expr);
  }
};
ZimDumper.prototype.findArticleByUrl = function(url) {
  this.pos = this.file.find(url);
};
ZimDumper.prototype.printPage = function() {
  if (this.pos.hasNext()) {
    this.log(this.pos.get().getPage());
  }
};
ZimDumper.prototype.dumpArticle = function() {
  if (this.pos.hasNext()) {
    this.log('%s', this.pos.get().getData().toString('utf8'));
  }
};
ZimDumper.prototype.dumpIndex = function() {
  if (this.pos.get().getNamespace() === 'X') {
    // XXX
  } else {
    this.log('no index article');
  }
};
ZimDumper.prototype.listArticles = function(info, listTable, extra) {
  for (var it = this.pos; it.hasNext();) {
    var a = it.next().value;
    if (listTable) {
      this.listArticleT(a, extra);
    } else if (info) {
      this.listArticle(a, extra);
    } else {
      this.log(a.getUrl());
    }
  }
};
ZimDumper.prototype.listArticle = function(article, extra) {
  var dirent = article.getDirent();
  this.log('url:', dirent.getUrl());
  this.log('\ttitle:    ', dirent.getTitle());
  this.log('\tidx:      ', article.getIndex());
  this.log('\tnamespace:', dirent.getNamespace());
  this.log('\ttype:     ', dirent.isRedirect() ? 'redirect' :
           dirent.isLinktarget() ? 'linktarget' :
           dirent.isDeleted() ? 'deleted' :
           'article');
  if (dirent.isRedirect()) {
    this.log('\tredirect index:', dirent.getRedirectIndex());
  } else if (dirent.isLinktarget()) {
    // Nothing else
  } else if (dirent.isDeleted()) {
    // Nothing else
  } else {
    this.log('\tmime-type:', article.getMimeType());
    this.log('\tarticle size:', article.getArticleSize());
    if (this.verbose) {
      var cluster = article.getCluster();

      this.log('\tcluster number: ', dirent.getClusterNumber());
      this.log('\tcluster count:  ', cluster.count());
      this.log('\tcluster size:   ', cluster.size());
      this.log('\tcluster offset: ',
               this.file.getClusterOffset(dirent.getClusterNumber()));
      this.log('\tblob number:    ', dirent.getBlobNumber());
      var c;
      switch (cluster.getCompression()) {
      case zim.zimcompDefault: { c = 'default'; break; }
      case zim.zimcompNone:    { c = 'none'; break; }
      case zim.zimcompZip:     { c = 'zip'; break; }
      case zim.zimcompBzip2:   { c = 'bzip2'; break; }
      case zim.zimcompLzma:    { c = 'lzma'; break; }
      default: { c = 'unknown (' + cluster.getCompression() + ')'; break; }
    }
      this.log('\tcompression:    ', c);
    }
    if (extra) {
      this.log('\textra:          ');
      // XXX implement parameter support.
    }
  }
};
ZimDumper.prototype.listArticleT = function(article, extra) {
  var dirent = article.getDirent();
  var str = dirent.getNamespace() +
      '\t' + dirent.getUrl() +
      '\t' + dirent.getTitle() +
      '\t' + article.getIndex() +
      '\t' + (dirent.isRedirect() ? 'R' :
              dirent.isLinktarget() ? 'L' :
              dirent.isDeleted() ? 'D' :
              'A');
  if (dirent.isRedirect()) {
    str += '\t' + dirent.getRedirectIndex();
  } else if (dirent.isLinktarget()) {
    // Nothing else
  } else if (dirent.isDeleted()) {
    // Nothing else
  } else {
    str +=
      '\t' + dirent.getMimeType() +
      '\t' + article.getArticleSize();
    if (verbose) {
      cluster = article.getCluster();
      str +=
        '\t' + dirent.getClusterNumber() +
        '\t' + cluster.count() +
        '\t' + cluster.size() +
        '\t' + this.file.getClusterOffset(dirent.getClusterNumber()) +
        '\t' + dirent.getBlobNumber() +
        '\t' + cluster.getCompression();
    }
  }

  if (extra) {
    var parameter = dirent.getParameter();
    str += '\t';
    // XXX
  }
  this.log(str);
};
ZimDumper.prototype.dumpFiles = function(directory) {
  fs.mkdirSync(directory, 0777);
  var ns = Object.create(null);
  for (var it = this.pos; it.hasNext();) {
    var article = it.next().value;
    var d = path.join(directory, article.getNamespace());
    if (!ns[article.getNamespace()]) {
      fs.mkdirSync(d, 0777);
      ns[article.getNamespace()] = true;
    }
    var t = article.getTitle();
    t = t.replace('/', '%2f');
    var f = path.join(d, t);
    fs.writeFileSync(f, article.getData().data());
  }
};
ZimDumper.prototype.verifyChecksum = function() {
  if (this.file.verify()) {
    this.log('checksum ok');
  } else {
    this.log('no checksum');
  }
};
ZimDumper.main = function(args) {
  var yargs = require('yargs');
  var argv = (args ? yargs(args) : yargs)
    .usage('Usage: $0 [options] zimfile')
    .demand(1, 1, 'The name of a zimfile is required.')
    .option('F', {
      describe: 'Print fileinfo',
      type: 'boolean',
      alias: 'fileinfo',
    })
    .option('N', {
      describe: 'Print info about given namespace',
      type: 'string',
      requiresArg: true,
      nargs: 1,
      alias: 'nsinfo',
    })
    .option('i', {
      describe: 'Print info about articles',
      type: 'boolean',
      alias: 'info',
    })
    .option('d', {
      describe: 'Print data of articles',
      type: 'boolean',
      alias: 'data',
    })
    .option('p', {
      describe: 'Print page',
      type: 'boolean',
      alias: 'page',
    })
    .option('f', {
      describe: 'Find article with given title',
      type: 'string',
      requiresArg: true,
      nargs: 1,
      alias: 'find',
    })
    .option('u', {
      describe: 'Find article with given url',
      type: 'string',
      requiresArg: true,
      nargs: 1,
      alias: 'url',
    })
    .option('l', {
      describe: 'List articles',
      type: 'boolean',
      alias: 'list',
    })
    .option('L', {
      describe: 'List articles as table',
      type: 'boolean',
      alias: 'tableList',
    })
    .option('o', {
      describe: 'Find article with given index',
      type: 'number',
      requiresArg: true,
      nargs: 1,
      alias: 'indexOffset',
    })
    .option('x', {
      describe: 'Print extra parameters',
      type: 'boolean',
      alias: 'extra',
    })
    .option('n', {
      describe: 'Specify namespace',
      type: 'string',
      requiresArg: true,
      nargs: 1,
      alias: 'ns',
      default: 'A',
    })
    .option('D', {
      describe: 'Dump all files into specified directory',
      type: 'string',
      requiresArg: true,
      nargs: 1,
      alias: 'dumpAll',
    })
    .option('v', {
      describe: 'Verbose\n' +
        '(print uncompressed length of articles when -i is set)\n' +
        '(print namespaces with counts with -F)',
      type: 'boolean',
      alias: 'verbose',
    })
    .option('Z', {
      describe: 'Dump index data',
      type: 'boolean',
      alias: 'zint',
    })
    .option('t', {
      describe: 'Sort (and find) articles by title instead of url',
      type: 'boolean',
      alias: 'titleSort',
    })
    .option('C', {
      describe: 'Verify checksum',
      type: 'boolean',
      alias: 'verifyChecksum',
    })
    .help('h').alias('h', 'help')
    .example('$0 -F wikipedia.zim')
    .example('$0 -l wikipedia.zim')
    .example('$0 -f Auto -i wikipedia.zim')
    .example('$0 -f Auto -d wikipedia.zim')
    .example('$0 -f Auto -l wikipedia.zim')
    .example('$0 -f Auto -l -i -v wikipedia.zim')
    .example('$0 -o 123159 -l -i wikipedia.zim')
    .argv;
  var app = new ZimDumper(argv._[0], argv.titleSort);
  app.setVerbose(argv.verbose);

  // global info
  if (argv.fileinfo) { app.printInfo(); }

  // Namespace info
  if (argv.nsinfo) { app.printNsInfo(argv.nsinfo); }

  // Locate article
  if (argv.indexOffset !== undefined) {
    app.locateArticle(argv.indexOffset);
  } else if (argv.find !== undefined) {
    app.findArticle(argv.ns, argv.find, argv.titleSort);
  } else if (argv.url !== undefined) {
    app.findArticleByUrl(argv.url);
  }

  // Dump files
  if (argv.dumpAll !== undefined) {
    app.dumpFiles(argv.dumpAll);
  }

  // Print requested info
  if (argv.data) {
    app.dumpArticle();
  } else if (argv.page) {
    app.printPage();
  } else if (argv.list || argv.tableList) {
    app.listArticles(argv.info, argv.tableList, argv.extra);
  } else if (argv.info) {
    app.listArticle(argv.extra);
  } else if (argv.zint) {
    app.dumpIndex();
  }

  if (argv.verifyChecksum) {
    app.verifyChecksum();
  }
};

if (require.main === module) {
  ZimDumper.main();
}
