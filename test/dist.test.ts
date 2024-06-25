import {
  Archive,
  Entry,
  IntegrityCheck,
  Compression,
  Blob,
  Searcher,
  Query,
  SuggestionSearcher,
  Creator,
  StringProvider,
  FileProvider,
  StringItem,
  FileItem,
} from "../dist/index";

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
