
export class IntegrityCheck {
  static CHECKSUM: symbol;
  static DIRENT_PTRS: symbol;
  static DIRENT_ORDER: symbol;
  static TITLE_INDEX: symbol;
  static CLUSTER_PTRS: symbol;
  static DIRENT_MIMETYPES: symbol;
  static COUNT: symbol;
}

export class Compression {
  static None: symbol;
  static Lzma: symbol;
  static Zstd: symbol;
}

export class Blob {
  constructor(buf?: ArrayBuffer | Buffer | string);
  data: Buffer;
  size: number | bigint;
  toString(): string;
}


export type ContentProvider {
  size: number | bigint;
  feed() : Blob;
}

export class StringProvider {
  constructor(content: string);
  size: number;
  feed() : Blob;
}

export class FileProvider {
  constructor(filepath: string);
  size: number | bigint;
  feed() : Blob;
}

export type Hint {
  COMPRESS?: number;
  FRONT_ARTICLE?: number;
}

export type WriterItem {
  readonly path: string;
  readonly title: string;
  readonly mimeType: string;
  readonly contentProvider : ContentProvider;
  readonly hints: Hint;
}

export class StringItem {
  constructor(path: string, mimetype: string, title: string, hint: Hint, content: string);
  readonly path: string;
  readonly title: string;
  readonly mimeType: string;
  readonly contentProvider : StringProvider;
  readonly hints: Hint;
}

export class FileItem {
  readonly path: string;
  readonly title: string;
  readonly mimeType: string;
  readonly contentProvider : FileProvider;
  readonly hints: Hint;
}

export class Creator {
  constructor();
  configVerbose(verbose: boolean) : this;
  configCompression(value: Compression) : this;
  configClusterSize(size: number) : this;
  configIndexing(indexing: boolean, language: string) : this;
  configNbWorkers(num: number) : this;
  startZimCreation(filepath: string) : this;
  addItem(item: WriterItem);
  addMetadata(name: string, content: string | ContentProvider, mimetype?: string);
  addIllustration(size: number, content: string | ContentProvider);
  addRedirection(path: string, title: string, targetPath: string, hints?: Hint);
  setMainPath(mainPath: string);
  setUuid(uuid: string);
  finishZimCreation() : Promise;
}

export class Item {
  title: string;
  path: string;
  mimetype: string;
  data: Blob;
  getData(offset?: number | bigint, limit?: number | bigint) : Blob;
  size: number | bigint;
  directAccessInformation: {
    filename: string,
      offset: number,
  };
  index: number | bigint;
}

export class Entry {
  isRedirect: boolean;
  title: string;
  path: string;
  item: Item;
  getItem(followRedirect?: boolean=false) : Item;
  redirect: Item;
  redirectEntry: Entry;
  index: number;
}

export class Archive {
  constructor(filepath: string);
  filename: string;
  filesize: number | bigint;
  allEntryCount: number;
  entryCount: number;
  articleCount: number;
  uuid: string;
  getMetadata(name: string) : string;
  getMetadataItem(name: string) : Item;
  metadataKeys: string[];
  getIllustrationItem(size: number) : Item;
  illustrationSizes : Set<number>;
  getEntryByPath(path_or_idx: string | number) : Entry;
  getEntryByTitle(title_or_idx: string | number) : Entry;
  getEntryByClusterOrder(idx: number) : Entry;
  mainEntry: Entry;
  randomEntry: Entry;
  hasEntryByPath(path: string) : boolean;
  hasEntryByTitle(title: string) : boolean;
  hasMainEntry(): boolean;
  hasIllustration(size: number) : boolean;
  hasFulltextIndex() : boolean;
  hasTitleIndex(): boolean;
}

