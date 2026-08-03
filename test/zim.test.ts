import assert from "node:assert/strict";
import crypto from "node:crypto";
import * as fs from "node:fs";
import {
  after,
  afterEach,
  before,
  beforeEach,
  describe,
  it,
  mock,
} from "node:test";
import {
  Archive,
  Blob,
  Compression,
  Creator,
  IllustrationInfo,
  IntegrityCheck,
  OpenConfig,
  Query,
  Searcher,
  StringItem,
  StringProvider,
  SuggestionSearcher,
  type WriterItem,
  getClusterCacheCurrentSize,
  getClusterCacheMaxSize,
  setClusterCacheMaxSize,
} from "../src/index.js";

describe("IntegrityCheck", () => {
  it("is exported with symbols", () => {
    const integrityCheckKeys = Object.keys(IntegrityCheck);

    assert.equal(integrityCheckKeys.length, 7);

    for (const key of integrityCheckKeys) {
      const keyTyped = key as keyof typeof IntegrityCheck;
      const sym = IntegrityCheck[keyTyped];
      assert.equal(typeof sym, "symbol");
    }
  });
});

describe("Compression", () => {
  it("is exported with symbols", () => {
    const compressionKeys = Object.keys(Compression);

    assert.equal(compressionKeys.length, 2);

    for (const key of compressionKeys) {
      const keyTyped = key as keyof typeof Compression;
      const sym = Compression[keyTyped];
      assert.equal(typeof sym, "symbol");
    }
  });
});

describe("Blob", () => {
  it("constructs a blob", () => {
    const blob = new Blob();
    assert(blob);
  });

  it("returns proper data", () => {
    const str = "hello world";
    const blob = new Blob(str);
    assert.equal(blob.size, str.length);
    assert.equal(blob.data.length, str.length);
    assert.equal(blob.data.toString(), str);
  });
});

describe("StringProvider", () => {
  it("constructs a StringProvider", () => {
    const str = "hello world";
    const provider = new StringProvider(str);
    assert(provider);
    assert.equal(provider.size, str.length);

    let feed = provider.feed();
    assert(feed);
    assert.equal(feed.size, str.length);
    assert.equal(feed.data.toString(), str);

    feed = provider.feed();
    assert(feed);
    assert.equal(feed.size, 0);
    assert.equal(feed.data.toString(), "");
  });
});

describe("StringItem", () => {
  const path = "test";
  const mimeType = "text/plain";
  const title = "Hello world";
  const hints = { COMPRESS: 0, FRONT_ARTICLE: 10 };
  const content = "Hello world 1!";

  it("constructs a StringItem with proper data", () => {
    const item = new StringItem(path, mimeType, title, hints, content);
    assert(item);
    assert.equal(item.path, path);
    assert.equal(item.mimeType, mimeType);
    assert.equal(item.title, title);
    assert.deepEqual(item.hints, hints);

    assert.equal(typeof item.getContentProvider, "function");

    const contentProvider = item.getContentProvider();
    assert(contentProvider);
    assert.equal(contentProvider.size, content.length);

    let feed = contentProvider.feed();
    assert(feed);
    assert.equal(feed.size, content.length);
    assert.equal(feed.data.toString(), content);

    feed = contentProvider.feed();
    assert(feed);
    assert.equal(feed.size, 0);
    assert.equal(feed.data.toString(), "");
  });

  it("constructs a StringItem from a Buffer", () => {
    const content = Buffer.from("abc\0def");
    assert.equal(content.length, 7);

    const item = new StringItem(path, mimeType, title, hints, content);
    assert(item);
    assert.equal(item.path, path);
    assert.equal(item.mimeType, mimeType);
    assert.equal(item.title, title);
    assert.deepEqual(item.hints, hints);

    const contentProvider = item.getContentProvider();
    assert(contentProvider);
    assert.equal(contentProvider.size, content.length);

    let feed = contentProvider.feed();
    assert(feed);
    assert.equal(feed.size, content.length);
    assert.equal(content.equals(feed.data), true);

    feed = contentProvider.feed();
    assert(feed);
    assert.equal(feed.size, 0);
    assert.equal(feed.data.toString(), "");
  });
});

describe("Creator", () => {
  const outFile = "./test.zim";

  const removeOutFile = () => {
    try {
      fs.unlinkSync(outFile);
    } catch {
      // noop
    }
  };

  beforeEach(() => {
    removeOutFile();
  });

  afterEach(() => {
    removeOutFile();
  });

  it("Configures", () => {
    const creator = new Creator();
    assert.equal(creator.configVerbose(true), creator);
    assert.equal(creator.configCompression(Compression.Zstd), creator);
    assert.equal(creator.configClusterSize(100), creator);
    assert.equal(creator.configIndexing(true, "en"), creator);
    assert.equal(creator.configNbWorkers(10), creator);
  });

  it("Creates a zim file", async () => {
    const creator = new Creator();
    try {
      assert.equal(creator.configIndexing(true, "en"), creator);
      assert.equal(creator.startZimCreation(outFile), creator);
      for (let i = 0; i < 10; i++) {
        const item = new StringItem(
          `test${i}`,
          "text/plain",
          `Hello world ${i}`,
          { FRONT_ARTICLE: 1, COMPRESS: 1 },
          `Hello world ${i}!`,
        );
        assert.equal(await creator.addItem(item), undefined);
      }

      await creator.addItem({
        // custom item
        path: "customContentProvider",
        mimeType: "text/plain",
        title: "Custom content provider",
        hints: {},
        getContentProvider() {
          // custom content provider
          const content = "ABCDEFG";
          let dataSent = false;
          return {
            size: content.length,
            feed() {
              if (!dataSent) {
                dataSent = true;
                return new Blob(content);
              }
              return new Blob();
            },
          };
        },
      });

      creator.addMetadata("test string", "A test string");
      creator.addMetadata(
        "test provider",
        new StringProvider("A string provider"),
      );

      const png = Buffer.from(
        "789c626001000000ffff030000060005",
        "hex",
      ).toString("utf8");
      creator.addIllustration(1, png);
      creator.addIllustration(
        new IllustrationInfo({ width: 10, height: 10 }),
        png,
      );
      creator.addRedirection("redirect/test1", "Redirect to test 1", "test1", {
        COMPRESS: 1,
      });
      creator.addAlias("alias/test1", "Alias to test 1", "test1", {
        COMPRESS: 1,
      });
      creator.setMainPath("redirect/test1");
      creator.setUuid("1234567890ABCDEF");
    } finally {
      await creator.finishZimCreation();
    }
  });
});

describe("IllustrationInfo", () => {
  it("Creates a default IllustrationInfo", () => {
    const info = new IllustrationInfo();
    assert(info);
    assert.equal(info.width, 0);
    assert.equal(info.height, 0);
    assert.equal(info.scale, 0);
    assert.deepEqual(info.extraAttributes, {});
  });

  it("Creates an empty IllustrationInfo", () => {
    const info = new IllustrationInfo({});
    assert(info);
    assert.equal(info.width, 0);
    assert.equal(info.height, 0);
    assert.equal(info.scale, 0);
    assert.deepEqual(info.extraAttributes, {});
  });

  it("Creates an IllustrationInfo which functions", () => {
    const info = new IllustrationInfo({
      width: 100,
      height: 200,
      scale: 2.0,
      extraAttributes: { test: "value" },
    });
    assert(info);
    assert.equal(info.width, 100);
    assert.equal(info.height, 200);
    assert.equal(info.scale, 2.0);
    assert.deepEqual(info.extraAttributes, { test: "value" });
  });
});

describe("OpenConfig", () => {
  it("Creates an OpenConfig which functions", () => {
    const config = new OpenConfig();
    assert(config);

    // Integration tests, these are set to the defaults
    assert.equal(config.m_preloadXapianDb, true);
    assert.equal(config.m_preloadDirentRanges, 1024);

    let res = config.preloadXapianDb(false);
    assert.equal(res, config);
    assert.equal(config.m_preloadXapianDb, false);

    res = config.preloadDirentRanges(5);
    assert.equal(res, config);
    assert.equal(config.m_preloadDirentRanges, 5);
  });
});

describe("Archive", () => {
  const outFile = "./test-read.zim";

  const testText = "openzim binding";
  const items: WriterItem[] = Array.from(Array(5).keys()).map(
    (i) =>
      new StringItem(
        `test${i}`,
        "text/html",
        `${testText} ${i}`,
        { FRONT_ARTICLE: 1 },
        `Hello world ${i}!`,
      ),
  );

  // test blobs via StringItem
  const blobs = Array.from(Array(3).keys()).map(
    (i) =>
      new StringItem(
        `blob${i}`,
        "application/octet-stream",
        `blob title ${i}`,
        {},
        crypto.createHash("md5").update(`blob title ${i}`).digest(),
      ),
  );

  // test blobs containing null bytes
  const null_blobs = [
    new StringItem(
      "null_blob_1",
      "application/octet-stream",
      "null blob title 1",
      {},
      "\xffabc\x00123",
    ),
    new StringItem(
      "null_blob_2",
      "application/octet-stream",
      "null blob title 2",
      {},
      "abc 123 \x00",
    ),
  ];

  // custom item
  items.push(
    ...Array.from(Array(5).keys())
      .map((i) => i + 5)
      .map((i) => ({
        path: `test${i}`,
        mimeType: "text/html",
        title: `${testText} ${i}`,
        hints: { FRONT_ARTICLE: 1 },
        getContentProvider() {
          let sent = false;
          const data = `Hello world ${i}!`;
          return {
            size: data.length,
            feed() {
              if (!sent) {
                sent = true;
                return new Blob(data);
              }
              return new Blob();
            },
          };
        },
      })),
  );

  // all entries
  const entries = items.concat(blobs, null_blobs);

  const meta = {
    test1: "test string 1",
    test2: "string string 2",
  };

  const png_size = 1;
  const png = Buffer.from("789c626001000000ffff030000060005", "hex").toString(
    "utf8",
  );

  // TODO: DRY
  const removeOutFile = () => {
    try {
      fs.unlinkSync(outFile);
    } catch {
      // noop
    }
  };

  before(async () => {
    removeOutFile();

    const creator = new Creator()
      .configIndexing(true, "en")
      .startZimCreation(outFile);

    for (const item of entries) {
      await creator.addItem(item);
    }

    let i = 0;
    for (const [k, v] of Object.entries(meta)) {
      creator.addMetadata(k, ++i % 2 === 0 ? v : new StringProvider(v));
    }

    creator.addIllustration(png_size, png);
    creator.setMainPath(items[0].path);

    await creator.finishZimCreation();
  });

  after(() => {
    removeOutFile();
  });

  it("Validates an archive", () => {
    const checks = [IntegrityCheck.CHECKSUM];
    assert.equal(Archive.validate(outFile, checks), true);
  });

  it("Opens an archive with OpenConfig", () => {
    const config = new OpenConfig()
      .preloadXapianDb(true)
      .preloadDirentRanges(512);
    const archive = new Archive(outFile, config);
    assert(archive);
    assert.equal(archive.filename, outFile);
    assert.equal(archive.allEntryCount >= items.length, true);
  });

  it("Reads items from an archive", () => {
    const archive = new Archive(outFile);
    assert(archive);
    assert.equal(archive.filename, outFile);
    assert.equal(archive.filesize > items.length, true);
    assert.equal(archive.allEntryCount >= items.length, true);
    assert.equal(archive.entryCount, entries.length);
    assert.equal(archive.articleCount, items.length);
    assert.equal(archive.mediaCount, 0);
    assert(archive.uuid, undefined);
    assert.equal(typeof archive.getDirentCacheMaxSize(), "number");
    assert.equal(typeof archive.getDirentCacheCurrentSize(), "number");

    // test metadata
    assert.partialDeepStrictEqual(archive.metadataKeys, Object.keys(meta));
    for (const [k, v] of Object.entries(meta)) {
      assert.equal(archive.getMetadata(k), v);

      const item = archive.getMetadataItem(k);
      assert.equal(item.title, k);
      assert.equal(item.data.data.toString(), v);
      assert.equal(item.size, v.length);
      assert.equal(item.mimetype.length > 3, true);
    }

    assert.equal(archive.hasIllustration(png_size), true);
    const illustration = archive.getIllustrationItem(png_size);
    assert.equal(illustration.data.data.toString().startsWith(png), true);
    assert.equal(illustration.data.data.toString(), png);
    assert.equal(illustration.size >= png.length, true);
    assert.equal(archive.illustrationSizes.has(png_size), true);

    // Only 1 png added
    const infos = archive.illustrationInfos;
    assert.equal(infos.length, 1);
    assert.equal(infos[0].width > 0, true);
    assert.equal(infos[0].height > 0, true);

    // That 1 png should be retrievable again using the info
    const il1 = archive.getIllustrationItem(infos[0]);
    assert(il1);
    assert.deepEqual(il1, illustration);
    assert.equal(il1.data.data.toString(), png);

    // Matching illustration
    assert.equal(
      archive.getIllustrationInfos(
        infos[0].width,
        infos[0].height,
        infos[0].scale,
      ).length,
      1,
    );

    // Not matching illustrationInfos
    assert.equal(archive.getIllustrationInfos(100, 100, 3).length, 0);

    for (const item of items) {
      const bypath = archive.getEntryByPath(item.path);
      assert(bypath);

      const byidx = archive.getEntryByPath(bypath.index);
      const bytitle = archive.getEntryByTitle(item.title);

      for (const entry of [bypath, byidx, bytitle]) {
        assert(entry);
        assert.equal(entry.path, item.path);
        assert.equal(entry.isRedirect, false);
        assert.equal(entry.index, bypath.index);
      }

      assert.equal(archive.hasEntryByPath(item.path), true);
      assert.equal(archive.hasEntryByTitle(item.title), true);
    }

    assert.equal(archive.hasMainEntry(), true);
    assert(archive.mainEntry);
    assert.equal(archive.mainEntry.isRedirect, true);
    assert.equal(archive.mainEntry.redirect.path, items[0].path);
    assert(archive.randomEntry.path);

    assert.equal(archive.hasFulltextIndex(), true);
    assert.equal(archive.hasTitleIndex(), true);

    items.sort((x, y) => x.path.localeCompare(y.path));
    const iter = archive.iterByPath();
    assert(iter);
    assert.equal(iter.size, entries.length);

    assert.equal(Array.from(iter).length, entries.length);
    assert.equal(typeof iter[Symbol.iterator], "function");
    assert(iter[Symbol.iterator]().next().value.title);
    assert.equal(iter[Symbol.iterator]().next().done, false);

    const itSpy = mock.fn();
    for (const entry of iter) {
      assert(entry);
      const item = entries.find((e) => e.path === entry.path);
      assert(item);
      assert.equal(entry.path, item.path);
      assert.equal(entry.title, item.title);
      itSpy();
    }
    assert.equal(itSpy.mock.callCount(), entries.length);

    // NOTE: expects items to be stored by path still
    assert.equal(Array.from(archive.iterByPath().offset(3, 1)).length, 1);
    // blobs don't have titles but items do.
    assert.equal(Array.from(archive.iterByTitle()).length, items.length);
    assert.equal(Array.from(archive.iterEfficient()).length, entries.length);

    assert.equal(Array.from(archive.findByTitle(items[2].title)).length, 1);
    assert.equal(
      Array.from(archive.findByTitle(items[3].title))[0].title,
      items[3].title,
    );

    assert.equal(Array.from(archive.findByPath(items[2].path)).length, 1);
    assert.equal(
      Array.from(archive.findByPath(items[3].path))[0].path,
      items[3].path,
    );

    assert.equal(archive.hasChecksum, true);
    assert(archive.checksum);

    assert.equal(archive.check(), true);
    assert.equal(archive.checkIntegrity(IntegrityCheck.CHECKSUM), true);

    assert.equal(archive.isMultiPart, false);
    assert.equal(archive.hasNewNamespaceScheme, true);
  });

  it("verifies that blobs were stored / read to / from the archive correctly", () => {
    const archive = new Archive(outFile);
    assert(archive);

    for (const bi of blobs) {
      const entry = Array.from(archive.findByPath(bi.path))[0];
      assert(entry);
      assert.equal(entry.title, bi.title);

      const hash = crypto.createHash("md5").update(entry.title).digest();
      assert.equal(hash.length, 16); // md5 size is 16 bytes

      const data = entry.item.data.data;
      assert.equal(entry.item.data.size, hash.length);
      assert.deepEqual(data, hash);
    }
  });

  it("verifies that blobs containing and ending in null were stored correctly from the archive", () => {
    const archive = new Archive(outFile);
    assert(archive);

    for (const nb of null_blobs) {
      const contentProvider = nb.getContentProvider();
      const feed = contentProvider.feed();
      assert(feed);
      assert.equal(feed.size > 0, true);
      if (nb.path === "null_blob_1") {
        // 4th byte is null, last byte is not
        assert.equal(feed.data[5], 0);
        assert.notEqual(feed.data[feed.data.length - 1], 0);
      } else if (nb.path === "null_blob_2") {
        // last byte is null
        assert.equal(feed.data[feed.data.length - 1], 0);
      }

      const entry = Array.from(archive.findByPath(nb.path))[0];
      assert(entry);
      assert.equal(entry.title, nb.title);

      assert.equal(entry.item.data.size, feed.data.length);
      assert.deepEqual(entry.item.data.data, feed.data);
    }
  });

  describe("Searcher", () => {
    it("searches the archive", () => {
      const archive = new Archive(outFile);
      assert.equal(archive.hasFulltextIndex(), true);
      assert.equal(archive.hasTitleIndex(), true);

      const searcher = new Searcher(archive);
      searcher.setVerbose(true);
      const search = searcher.search(new Query(testText));
      assert(search);
      assert.equal(search.estimatedMatches, items.length);

      const results = search.getResults(0, 100);
      assert(results);
      assert.equal(results.size, items.length);

      const iter = results;
      assert.equal(typeof iter[Symbol.iterator], "function");
      assert.equal(iter[Symbol.iterator]().next().done, false);

      assert.equal(Array.from(iter).length, items.length);
      for (const item of iter) {
        assert(item.entry);
        assert.match(item.title, new RegExp(`^${testText} \\d+\$`));
      }
    });
  });

  describe("Suggestion Search", () => {
    it("searches for suggestions in the archive", () => {
      const archive = new Archive(outFile);
      assert.equal(archive.hasFulltextIndex(), true);
      assert.equal(archive.hasTitleIndex(), true);

      const suggestionSearcher = new SuggestionSearcher(archive);
      suggestionSearcher.setVerbose(true);

      const suggestion = suggestionSearcher.suggest(testText);
      assert(suggestion);
      assert.equal(suggestion.estimatedMatches, items.length);

      const results = suggestion.getResults(0, 100);
      assert(results);
      assert.equal(results.size, items.length);

      const iter = results;
      assert.equal(typeof iter[Symbol.iterator], "function");
      assert.equal(iter[Symbol.iterator]().next().done, false);

      assert.equal(Array.from(iter).length, items.length);
      for (const item of iter) {
        assert(item.entry);
        assert.match(item.title, new RegExp(`^${testText} \\d+\$`));
      }
    });
  });

  describe("Cache sizes", () => {
    it("Manipulate cluster cache max size", () => {
      setClusterCacheMaxSize(10);
      assert.equal(getClusterCacheMaxSize(), 10);
      assert.equal(typeof getClusterCacheCurrentSize(), "number");
      setClusterCacheMaxSize(2);
      assert.equal(getClusterCacheMaxSize(), 2);
      assert.equal(typeof getClusterCacheCurrentSize(), "number");
    });
    it("Manipulate dirent cache max size", () => {
      const archive = new Archive(outFile);
      archive.setDirentCacheMaxSize(10);
      assert.equal(archive.getDirentCacheMaxSize(), 10);
      assert.equal(archive.getDirentCacheCurrentSize(), 10);
      archive.setDirentCacheMaxSize(5);
      assert.equal(archive.getDirentCacheMaxSize(), 5);
      assert.equal(archive.getDirentCacheCurrentSize(), 5);
    });
  });
});

describe("Query", () => {
  it("constructs a query", () => {
    const query = new Query("hello world");
    assert(query);
    assert.equal(query.query, "hello world");
    assert.equal(query.toString(), "hello world");
    assert.equal(query.georange, null);

    const range = { latitude: 1, longitude: 2, distance: 3 };
    query.georange = range;
    assert.deepEqual(query.georange, range);

    range.latitude = 10;
    query.setGeorange(10, 2, 3);
    assert.deepEqual(query.georange, range);
  });
});
