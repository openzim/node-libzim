// noinspection ES6UnusedImports
import {} from "ts-jest";
import crypto from "crypto";
import * as fs from "fs";
import {
  Archive,
  IntegrityCheck,
  Compression,
  Blob,
  StringItem,
  StringProvider,
  Creator,
  Query,
  Searcher,
  SuggestionSearcher,
  WriterItem,
} from "../src";

describe("IntegrityCheck", () => {
  it("is exported with symbols", () => {
    expect(Object.keys(IntegrityCheck)).toHaveLength(7);
    for (const key of Object.keys(IntegrityCheck)) {
      const keyTyped = key as keyof typeof IntegrityCheck;
      const sym = IntegrityCheck[keyTyped];
      expect(typeof sym).toBe("symbol");
    }
  });
});

describe("Compression", () => {
  it("is exported with symbols", () => {
    expect(Object.keys(Compression)).toHaveLength(2);
    for (const key of Object.keys(Compression)) {
      const keyTyped = key as keyof typeof Compression;
      const sym = Compression[keyTyped];
      expect(typeof sym).toBe("symbol");
    }
  });
});

describe("Blob", () => {
  it("constructs a blob", () => {
    const blob = new Blob();
    expect(blob).toBeDefined();
  });

  it("returns proper data", () => {
    const str = "hello world";
    const blob = new Blob(str);
    expect(blob).toBeDefined();
    expect(blob.size).toBe(str.length);
    expect(blob.data.length).toBe(str.length);
    expect(blob.data.toString()).toBe(str);
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
    expect(item).toBeDefined();
    expect(item.path).toBe(path);
    expect(item.mimeType).toBe(mimeType);
    expect(item.title).toBe(title);
    expect(item.hints).toEqual(hints);

    expect(typeof item.getContentProvider).toBe("function");

    const contentProvider = item.getContentProvider();
    expect(contentProvider).toBeDefined();
    expect(contentProvider.size).toBe(content.length);

    let feed = contentProvider.feed();
    expect(feed).toBeDefined();
    expect(feed.size).toBe(content.length);
    expect(feed.data.toString()).toBe(content);

    feed = contentProvider.feed();
    expect(feed).toBeDefined();
    expect(feed.size).toBe(0);
    expect(feed.data.toString()).toBe("");
  });

  it("constructs a StringItem from a Buffer", () => {
    const content = Buffer.from("abc\0def");
    expect(content.length).toEqual(7);

    const item = new StringItem(path, mimeType, title, hints, content);
    expect(item).toBeDefined();
    expect(item.path).toBe(path);
    expect(item.mimeType).toBe(mimeType);
    expect(item.title).toBe(title);
    expect(item.hints).toEqual(hints);

    const contentProvider = item.getContentProvider();
    expect(contentProvider).toBeDefined();
    expect(contentProvider.size).toBe(content.length);

    let feed = contentProvider.feed();
    expect(feed).toBeDefined();
    expect(feed.size).toBe(content.length);
    expect(content.equals(feed.data)).toBe(true);

    feed = contentProvider.feed();
    expect(feed).toBeDefined();
    expect(feed.size).toBe(0);
    expect(feed.data.toString()).toBe("");
  });
});

describe("Creator", () => {
  const outFile = "./test.zim";

  const removeOutFile = () => {
    try {
      fs.unlinkSync(outFile);
    } catch (e) {
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
    expect(creator.configVerbose(true)).toEqual(creator);
    expect(creator.configCompression(Compression.Zstd)).toEqual(creator);
    expect(creator.configClusterSize(100)).toEqual(creator);
    expect(creator.configIndexing(true, "en")).toEqual(creator);
    expect(creator.configNbWorkers(10)).toEqual(creator);
  });

  it("Creates a zim file", async () => {
    const creator = new Creator();
    try {
      expect(creator.configIndexing(true, "en")).toEqual(creator);
      expect(creator.startZimCreation(outFile)).toEqual(creator);
      for (let i = 0; i < 10; i++) {
        const item = new StringItem(
          `test${i}`,
          "text/plain",
          `Hello world ${i}`,
          { FRONT_ARTICLE: 1, COMPRESS: 1 },
          `Hello world ${i}!`,
        );
        await expect(creator.addItem(item)).resolves.toEqual(undefined);
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
      creator.addRedirection("redirect/test1", "Redirect to test 1", "test1", {
        COMPRESS: 1,
      });
      creator.setMainPath("redirect/test1");
      creator.setUuid("1234567890ABCDEF");
    } finally {
      await creator.finishZimCreation();
    }
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
    } catch (e) {
      // noop
    }
  };

  beforeAll(async () => {
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

  afterAll(() => {
    removeOutFile();
  });

  it("Validates an archive", () => {
    const checks = [IntegrityCheck.CHECKSUM];
    expect(Archive.validate(outFile, checks)).toBe(true);
  });

  it("Reads items from an archive", () => {
    const archive = new Archive(outFile);
    expect(archive).toBeDefined();
    expect(archive.filename).toBe(outFile);
    expect(archive.filesize).toBeGreaterThan(items.length);
    expect(archive.allEntryCount).toBeGreaterThanOrEqual(items.length);
    expect(archive.entryCount).toBe(entries.length);
    expect(archive.articleCount).toBe(items.length);
    expect(archive.mediaCount).toBe(0);
    expect(archive.uuid).toBeDefined();
    expect(archive.getClusterCacheMaxSize()).toBeDefined();
    expect(archive.getClusterCacheCurrentSize()).toBeDefined();
    expect(archive.getDirentCacheMaxSize()).toBeDefined();
    expect(archive.getDirentCacheCurrentSize()).toBeDefined();
    expect(archive.getDirentLookupCacheMaxSize()).toBeDefined();

    // test metadata
    expect(archive.metadataKeys).toEqual(
      expect.arrayContaining(Object.keys(meta)),
    );
    for (const [k, v] of Object.entries(meta)) {
      expect(archive.getMetadata(k)).toEqual(v);

      const item = archive.getMetadataItem(k);
      expect(item.title).toEqual(k);
      expect(item.data.data.toString()).toEqual(v);
      expect(item.size).toEqual(v.length);
      expect(item.mimetype.length).toBeGreaterThan(3);
    }

    expect(archive.hasIllustration(png_size)).toBe(true);
    const illustration = archive.getIllustrationItem(png_size);
    expect(illustration.data.data.toString().startsWith(png)).toBe(true);
    expect(illustration.data.data.toString()).toEqual(png);
    expect(illustration.size).toBeGreaterThanOrEqual(png.length);
    expect(archive.illustrationSizes).toContain(png_size);

    for (const item of items) {
      const bypath = archive.getEntryByPath(item.path);
      expect(bypath).toBeDefined();

      const byidx = archive.getEntryByPath(bypath.index);
      const bytitle = archive.getEntryByTitle(item.title);

      for (const entry of [bypath, byidx, bytitle]) {
        expect(entry).toBeDefined();
        expect(entry.path).toEqual(item.path);
        expect(entry.isRedirect).toBe(false);
        expect(entry.index).toEqual(bypath.index);
      }

      expect(archive.hasEntryByPath(item.path)).toBe(true);
      expect(archive.hasEntryByTitle(item.title)).toBe(true);
    }

    expect(archive.hasMainEntry()).toBe(true);
    expect(archive.mainEntry).toBeDefined();
    expect(archive.mainEntry.isRedirect).toBe(true);
    expect(archive.mainEntry.redirect.path).toEqual(items[0].path);
    expect(archive.randomEntry.path).toBeDefined();

    expect(archive.hasFulltextIndex()).toBe(true);
    expect(archive.hasTitleIndex()).toBe(true);

    items.sort((x, y) => x.path.localeCompare(y.path));
    const iter = archive.iterByPath();
    expect(iter).toBeDefined();
    expect(iter.size).toEqual(entries.length);

    expect(Array.from(iter).length).toEqual(entries.length);
    expect(iter[Symbol.iterator]).toEqual(expect.any(Function));
    expect(iter[Symbol.iterator]().next().value.title).toBeDefined();
    expect(iter[Symbol.iterator]().next().done).toBe(false);

    const itSpy = jest.fn();
    for (const entry of iter) {
      expect(entry).toBeDefined();
      const item = entries.find((e) => e.path === entry.path);
      expect(item).toBeDefined();
      expect(entry.path).toEqual(item.path);
      expect(entry.title).toEqual(item.title);
      itSpy();
    }
    expect(itSpy).toHaveBeenCalledTimes(entries.length);

    // NOTE: expects items to be stored by path still
    expect(Array.from(archive.iterByPath().offset(3, 1)).length).toEqual(1);
    // blobs don't have titles but items do.
    expect(Array.from(archive.iterByTitle()).length).toEqual(items.length);
    expect(Array.from(archive.iterEfficient()).length).toEqual(entries.length);

    expect(Array.from(archive.findByTitle(items[2].title)).length).toEqual(1);
    expect(Array.from(archive.findByTitle(items[3].title))[0].title).toEqual(
      items[3].title,
    );

    expect(Array.from(archive.findByPath(items[2].path)).length).toEqual(1);
    expect(Array.from(archive.findByPath(items[3].path))[0].path).toEqual(
      items[3].path,
    );

    expect(archive.hasChecksum).toBe(true);
    expect(archive.checksum).toBeDefined();

    expect(archive.check()).toBe(true);
    expect(archive.checkIntegrity(IntegrityCheck.CHECKSUM)).toBe(true);

    expect(archive.isMultiPart).toBe(false);
    expect(archive.hasNewNamespaceScheme).toBe(true);
  });

  it("verifies that blobs were stored / read to / from the archive correctly", () => {
    const archive = new Archive(outFile);
    expect(archive).toBeDefined();

    for (const bi of blobs) {
      const entry = Array.from(archive.findByPath(bi.path))[0];
      expect(entry).toBeDefined();
      expect(entry.title).toEqual(bi.title);

      const hash = crypto.createHash("md5").update(entry.title).digest();
      expect(hash.length).toEqual(16); // md5 size is 16 bytes

      const data = entry.item.data.data;
      expect(entry.item.data.size).toEqual(hash.length);
      expect(data).toEqual(hash);
    }
  });

  it("verifies that blobs containing and ending in null were stored correctly from the archive", () => {
    const archive = new Archive(outFile);
    expect(archive).toBeDefined();

    for (const nb of null_blobs) {
      const contentProvider = nb.getContentProvider();
      const feed = contentProvider.feed();
      expect(feed).toBeDefined();
      expect(feed.size).toBeGreaterThan(0);
      if (nb.path === "null_blob_1") {
        // 4th byte is null, last byte is not
        expect(feed.data[5]).toEqual(0);
        expect(feed.data[feed.data.length - 1]).not.toEqual(0);
      } else if (nb.path === "null_blob_2") {
        // last byte is null
        expect(feed.data[feed.data.length - 1]).toEqual(0);
      }

      const entry = Array.from(archive.findByPath(nb.path))[0];
      expect(entry).toBeDefined();
      expect(entry.title).toEqual(nb.title);

      expect(entry.item.data.size).toEqual(feed.data.length);
      expect(entry.item.data.data).toEqual(feed.data);
    }
  });

  describe("Searcher", () => {
    it("searches the archive", () => {
      const archive = new Archive(outFile);
      expect(archive.hasFulltextIndex()).toBe(true);
      expect(archive.hasTitleIndex()).toBe(true);

      const searcher = new Searcher(archive);
      searcher.setVerbose(true);
      const search = searcher.search(new Query(testText));
      expect(search).toBeDefined();
      expect(search.estimatedMatches).toEqual(items.length);

      const results = search.getResults(0, 100);
      expect(results).toBeDefined();
      expect(results.size).toEqual(items.length);

      const iter = results;
      expect(iter[Symbol.iterator]).toEqual(expect.any(Function));
      expect(iter[Symbol.iterator]().next().done).toBe(false);

      expect(Array.from(iter).length).toEqual(items.length);
      for (const item of iter) {
        expect(item.entry).toBeDefined();
        expect(item.title).toMatch(new RegExp(`^${testText} \\d+\$`));
      }
    });
  });

  describe("Suggestion Search", () => {
    it("searches for suggestions in the archive", () => {
      const archive = new Archive(outFile);
      expect(archive.hasFulltextIndex()).toBe(true);
      expect(archive.hasTitleIndex()).toBe(true);

      const suggestionSearcher = new SuggestionSearcher(archive);
      suggestionSearcher.setVerbose(true);

      const suggestion = suggestionSearcher.suggest(testText);
      expect(suggestion).toBeDefined();
      expect(suggestion.estimatedMatches).toEqual(items.length);

      const results = suggestion.getResults(0, 100);
      expect(results).toBeDefined();
      expect(results.size).toEqual(items.length);

      const iter = results;
      expect(iter[Symbol.iterator]).toEqual(expect.any(Function));
      expect(iter[Symbol.iterator]().next().done).toBe(false);

      expect(Array.from(iter).length).toEqual(items.length);
      for (const item of iter) {
        expect(item.entry).toBeDefined();
        expect(item.title).toMatch(new RegExp(`^${testText} \\d+\$`));
      }
    });
  });

  describe("Cache sizes", () => {
    it("Manipulate cluster cache max size", () => {
      const archive = new Archive(outFile);
      archive.setClusterCacheMaxSize(10);
      expect(archive.getClusterCacheMaxSize()).toBe(10);
      expect(archive.getClusterCacheCurrentSize()).toBe(1); // there is only one cluser in test ZIM
      archive.setClusterCacheMaxSize(2);
      expect(archive.getClusterCacheMaxSize()).toBe(2);
      expect(archive.getClusterCacheCurrentSize()).toBe(1);
    });
    it("Manipulate dirent cache max size", () => {
      const archive = new Archive(outFile);
      archive.setDirentCacheMaxSize(10);
      expect(archive.getDirentCacheMaxSize()).toBe(10);
      expect(archive.getDirentCacheCurrentSize()).toBe(10);
      archive.setDirentCacheMaxSize(5);
      expect(archive.getDirentCacheMaxSize()).toBe(5);
      expect(archive.getDirentCacheCurrentSize()).toBe(5);
    });
    it("Manipulate dirent lookup cache max size", () => {
      const archive = new Archive(outFile);
      archive.setDirentLookupCacheMaxSize(6);
      expect(archive.getDirentLookupCacheMaxSize()).toBe(6);
    });
  });
});

describe("Query", () => {
  it("constructs a query", () => {
    const query = new Query("hello world");
    expect(query).toBeDefined();
    expect(query.query).toEqual("hello world");
    expect(query.toString()).toEqual("hello world");
    expect(query.georange).toBe(null);

    const range = { latitude: 1, longitude: 2, distance: 3 };
    query.georange = range;
    expect(query.georange).toEqual(range);

    range.latitude = 10;
    query.setGeorange(10, 2, 3);
    expect(query.georange).toEqual(range);
  });
});
