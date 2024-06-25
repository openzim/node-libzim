import bindings from "bindings";

export const {
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
} = bindings("zim_binding");
