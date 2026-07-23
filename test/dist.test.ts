import { expect } from "expect";
import { describe, it } from "node:test";
import {
  Archive,
  Blob,
  Compression,
  Creator,
  Entry,
  FileItem,
  FileProvider,
  IntegrityCheck,
  Query,
  Searcher,
  StringItem,
  StringProvider,
  SuggestionSearcher,
} from "../dist/index.js";

describe("libzim dist", () => {
  it("should have all the functions", () => {
    expect(Archive).toBeDefined();
    expect(Entry).toBeDefined();
    expect(IntegrityCheck).toBeDefined();
    expect(Compression).toBeDefined();
    expect(Blob).toBeDefined();
    expect(Searcher).toBeDefined();
    expect(Query).toBeDefined();
    expect(SuggestionSearcher).toBeDefined();
    expect(Creator).toBeDefined();
    expect(StringProvider).toBeDefined();
    expect(FileProvider).toBeDefined();
    expect(StringItem).toBeDefined();
    expect(FileItem).toBeDefined();
  });
});
