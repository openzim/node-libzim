import assert from "node:assert/strict";
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
    assert(Archive);
    assert(Entry);
    assert(IntegrityCheck);
    assert(Compression);
    assert(Blob);
    assert(Searcher);
    assert(Query);
    assert(SuggestionSearcher);
    assert(Creator);
    assert(StringProvider);
    assert(FileProvider);
    assert(StringItem);
    assert(FileItem);
  });
});
