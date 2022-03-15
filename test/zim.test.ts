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
    await creator.finishZimCreation();
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
  });
});

