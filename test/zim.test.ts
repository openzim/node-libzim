// noinspection ES6UnusedImports
import {} from 'ts-jest';
import path from 'path';
import * as fs from 'fs';
import * as faker from 'faker';
import {
  Archive,
  IntegrityCheck,
  Compression,
  Blob,
  StringItem,
  FileItem,
  StringProvider,
  FileProvider,
  Creator,
} from '../src';



beforeAll(() => {
});

describe('IntegrityCheck', () => {
  it('is exported with symbols', () => {
    expect(Object.keys(IntegrityCheck)).toHaveLength(7);
    for(const key of Object.keys(IntegrityCheck)) {
      const keyTyped = key as keyof typeof IntegrityCheck;
      const sym = IntegrityCheck[keyTyped];
      expect(typeof sym).toBe('symbol');
    }
  });
});

describe('Compression', () => {
  it('is exported with symbols', () => {
    expect(Object.keys(Compression)).toHaveLength(3);
    for(const key of Object.keys(Compression)) {
      const keyTyped = key as keyof typeof Compression;
      const sym = Compression[keyTyped];
      expect(typeof sym).toBe('symbol');
    }
  });
});

describe('Blob', () => {
  it('constructs a blob', () => {
    const blob = new Blob();
    expect(blob).toBeDefined();
  });

  it('returns proper data', () => {
    const str = "hello world";
    const blob = new Blob(str);
    expect(blob).toBeDefined();
    expect(blob.size).toBe(str.length);
    expect(blob.data.length).toBe(str.length);
    expect(blob.data.toString()).toBe(str);
  });
});

describe('StringItem', () => {
  const path = "A/test";
  const mimeType = "text/plain";
  const title = "Hello world";
  const hints = {COMPRESS: 0, FRONT_ARTICLE: 10};
  const content = "Hello world 1!";

  it('constructs a StringItem with proper data', () => {
    const item = new StringItem(path, mimeType, title, hints, content);
    expect(item).toBeDefined();
    expect(item.path).toBe(path);
    expect(item.mimeType).toBe(mimeType);
    expect(item.title).toBe(title);
    expect(item.hints).toEqual(hints);

    const contentProvider = item.contentProvider;
    expect(contentProvider).toBeDefined();
    expect(contentProvider.size).toBe(content.length);

    let feed = contentProvider.feed();
    expect(feed).toBeDefined();
    expect(feed.size).toBe(content.length);
    expect(feed.data.toString()).toBe(content);

    feed = contentProvider.feed();
    expect(feed).toBeDefined();
    expect(feed.size).toBe(0);
    expect(feed.data.toString()).toBe('');
  });
});

describe('Creator', () => {
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

  it('Configures', () => {
    const creator = new Creator();
    expect(creator.configVerbose(true)).toEqual(creator);
    expect(creator.configCompression(Compression.Zstd)).toEqual(creator);
    expect(creator.configClusterSize(100)).toEqual(creator);
    expect(creator.configIndexing(true, "english")).toEqual(creator);
    expect(creator.configNbWorkers(10)).toEqual(creator);
  });

  it('Creates a zim file', async () => {
    const creator = new Creator();
    try {
      expect(creator.startZimCreation(outFile)).toEqual(creator);
      for(let i = 0; i < 10; i++) {
        const item = new StringItem(
          `A/test${i}`,
          "text/plain",
          `Hello world ${i}`,
          {},
          `Hello world ${i}!`
        );
        expect(creator.addItem(item)).toEqual(undefined);
      }

      let content = 'ABCDEFG';
      let dataSent = false;
      creator.addItem({ // custom item
        path: "A/customContentProvider",
        mimeType: "text/plain",
        title: "Custom content provider",
        hints: {},
        contentProvider: { // custom content provider
          size: content.length,
          feed() {
            if(!dataSent) {
              dataSent = true;
              return new Blob(content);
            }
            return new Blob();
          },
        },
      });

      creator.addMetadata("test string", "A test string");
      creator.addMetadata(
        "test provider",
        new StringProvider("A string provider"));

      const png = Buffer.from('789c626001000000ffff030000060005', 'hex')
        .toString('utf8');
      creator.addIllustration(1, png);
      creator.addRedirection("A/redirect/test1", "Redirect to test 1", "A/test1", {COMPRESS: 1});
      creator.setMainPath("A/redirect/test1");
      creator.setUuid("1234567890ABCDEF");
    } finally {
      await creator.finishZimCreation();
    }
  });
});

describe('Archive', () => {
  const outFile = './test-read.zim';

  const items = Array.from(Array(10).keys()).map(i => new StringItem(
    `A/test${i}`,
    "text/plain",
    `Hello world ${i}`,
    {},
    `Hello world ${i}!`
  ));

  const meta = {
    test1: 'test string 1',
    test2: 'string string 2',
  };

  const png_size = 1;
  const png = Buffer.from('789c626001000000ffff030000060005', 'hex')
    .toString('utf8');

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

    const creator = new Creator().startZimCreation(outFile);
    items.forEach(item => creator.addItem(item));

    let i = 0;
    for(const [k, v] of Object.entries(meta)) {
      creator.addMetadata(k, (++i %2 == 0) ? v : new StringProvider(v));
    }

    creator.addIllustration(png_size, png);
    creator.setMainPath(items[0].path);

    await creator.finishZimCreation();
  });

  afterEach(() => {
    removeOutFile();
  });

  it('Reads items from an archive', () => {
    const archive = new Archive(outFile);
    expect(archive).toBeDefined();
    expect(archive.filename).toBe(outFile);
    expect(archive.filesize).toBeGreaterThan(items.length);
    expect(archive.allEntryCount).toBeGreaterThanOrEqual(items.length);
    expect(archive.entryCount).toBe(items.length);
    expect(archive.articleCount).toBe(items.length);
    expect(archive.uuid).toBeDefined();

    // test metadata
    expect(archive.metadataKeys).toEqual(expect.arrayContaining(Object.keys(meta)));
    for(const [k, v] of Object.entries(meta)) {
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

    for(const item of items) {
      const bypath = archive.getEntryByPath(item.path);
      expect(bypath).toBeDefined();

      const byidx = archive.getEntryByPath(bypath.index);
      const bytitle = archive.getEntryByTitle(item.title);
      const byCluster = archive.getEntryByClusterOrder(bypath.index);

      for(const entry of [bypath, byidx, bytitle, byCluster]) {
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

    expect(archive.hasFulltextIndex()).toBe(false);
    expect(archive.hasTitleIndex()).toBe(false);

    items.sort((x, y) => x.path.localeCompare(y.path));
    const iter = archive.iterByPath();
    expect(iter).toBeDefined();
    expect(iter.size).toEqual(items.length);

    expect(Array.from(iter).length).toEqual(items.length);
    expect(iter[Symbol.iterator]).toEqual(expect.any(Function));
    expect(iter[Symbol.iterator]().next().value.title).toBe(items[0].title);
    expect(iter[Symbol.iterator]().next().done).toBe(false);

    const itSpy = jest.fn();
    let i = 0;
    for(const entry of iter) {
      expect(entry).toBeDefined();
      expect(entry.path).toEqual(items[i].path);
      expect(entry.title).toEqual(items[i].title);
      i++;
      itSpy();
    }
    expect(itSpy).toHaveBeenCalledTimes(items.length);
  });
});

