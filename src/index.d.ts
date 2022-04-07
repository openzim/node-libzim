
export class IntegrityCheck {
  static CHECKSUM: symbol;
  static DIRENT_PTRS: symbol;
  static DIRENT_ORDER: symbol;
  static TITLE_INDEX: symbol;
  static CLUSTER_PTRS: symbol;
  static DIRENT_MIMETYPES: symbol;
  static COUNT: symbol;  // DO NOT USE THIS. See libzim docs.
}

export class Compression {
  static None: symbol;
  static Lzma: symbol;
  static Zstd: symbol;
}

export class Blob {
  constructor(buf?: ArrayBuffer | Buffer | string);
  get data(): Buffer;
  get size(): number | bigint;
  toString(): string;
}

export type ContentProvider = {
  size: number | bigint;
  feed() : Blob;
}

export class StringProvider {
  constructor(content: string);
  get size(): number;
  feed() : Blob;
}

export class FileProvider {
  constructor(filepath: string);
  get size(): number | bigint;
  feed() : Blob;
}

export type Hint = {
  COMPRESS?: number;
  FRONT_ARTICLE?: number;
}

export interface IndexData {
  hasIndexData?: boolean;
  title?: string;
  content?: string;
  keywords?: string;
  wordcount?: number;
  position?: [boolean, number, number];
}

export interface WriterItem {
  path: string;
  title: string;
  mimeType: string;
  contentProvider : ContentProvider;
  hints: Hint;
  indexData?: IndexData;
}

export class StringItem {
  constructor(path: string, mimeType: string, title: string, hint: Hint, content: string);
  get path(): string;
  get title(): string;
  get mimeType(): string;
  get contentProvider() : StringProvider;
  get hints(): Hint;
}

export class FileItem {
  get path(): string;
  get title(): string;
  get mimeType(): string;
  get contentProvider() : StringProvider;
  get hints(): Hint;
}

export class Creator {
  constructor();
  configVerbose(verbose: boolean) : this;
  configCompression(value: Compression) : this;
  configClusterSize(size: number) : this;
  configIndexing(indexing: boolean, language: string) : this;
  configNbWorkers(num: number) : this;
  startZimCreation(filepath: string): this;
  addItem(item: WriterItem): void;
  addMetadata(name: string, content: string | ContentProvider, mimetype?: string): void;
  addIllustration(size: number, content: string | ContentProvider): void;
  addRedirection(path: string, title: string, targetPath: string, hints?: Hint): void;
  setMainPath(mainPath: string): void;
  setUuid(uuid: string): void;
  finishZimCreation() : Promise<void>;
}

export class Item {
  get title(): string;
  get path(): string;
  get mimetype(): string;
  get data(): Blob;
  getData(offset?: number | bigint, limit?: number | bigint) : Blob;
  get size(): number | bigint;
  get directAccessInformation(): {
    filename: string,
      offset: number,
  };
  get index(): number | bigint;
}

export class Entry {
  get isRedirect(): boolean;
  get title(): string;
  get path(): string;
  get item(): Item;
  getItem(followRedirect?: boolean) : Item;
  get redirect(): Item;
  get redirectEntry(): Entry;
  get index(): number;
}

export interface EntryRange extends Iterable<Entry> {
  size: number;
  offset(start: number, maxResults: number) : EntryRange;
}

export class Archive {
  constructor(filepath: string);
  get filename(): string;
  get filesize(): number | bigint;
  get allEntryCount(): number;
  get entryCount(): number;
  get articleCount(): number;
  get uuid(): string;
  getMetadata(name: string) : string;
  getMetadataItem(name: string) : Item;
  get metadataKeys(): string[];
  getIllustrationItem(size: number) : Item;
  get illustrationSizes() : Set<number>;
  getEntryByPath(path_or_idx: string | number) : Entry;
  getEntryByTitle(title_or_idx: string | number) : Entry;
  getEntryByClusterOrder(idx: number) : Entry;
  get mainEntry(): Entry;
  get randomEntry(): Entry;
  hasEntryByPath(path: string) : boolean;
  hasEntryByTitle(title: string) : boolean;
  hasMainEntry(): boolean;
  hasIllustration(size: number) : boolean;
  hasFulltextIndex() : boolean;
  hasTitleIndex(): boolean;
  iterByPath() : EntryRange;
  iterByTitle() : EntryRange;
  iterEfficient() : EntryRange;
  findByPath(path: string) : EntryRange;
  findByTitle(title: string) : EntryRange;
  get hasChecksum(): boolean;
  get checksum(): string;
  check() : boolean;
  checkIntegrity(checkType: symbol) : boolean;  // one of IntegrityCheck
  get isMultiPart(): boolean;
  get hasNewNamespaceScheme(): boolean;

  static validate(zimPath: string, checksToRun: symbol[]) : boolean; // list of IntegrityCheck
}

interface Georange {
  latitude: number;
  longitude: number;
  distance: number;
}

export class Query {
  constructor(query: string);
  setQuery(query: string): this;
  setGeorange(latitude: number, longitude: number, distance: number): this;
  get query() : string;
  set query(query: string);
  toString(): string;
  get georange() : Georange;
  set georange(range: Georange);
}

export class SearchIterator {
  get path(): string;
  get title(): string;
  get score(): number;
  get snippet(): string;
  get wordCount(): number;
  get size() : number;
  get fileIndex(): number;
  get zimId(): string;
  get entry(): Entry;
}

export class SearchResultSet extends Iterable<SearchIterator> {
  get size(): number;
}

export class Search {
  getResults(start: number, maxResults: number) : SearchResultSet;
  get estimatedMatches(): number;
}

export class Searcher {
  constructor(archives: Archive | Archive[]);
  addArchive(archive: Archive): this;
  search(query: string | Query) : Search;
  setVerbose(verbose: boolean) : this;
}
