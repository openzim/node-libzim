import bindings from "bindings";

export const {
  Archive,
  OpenConfig,
  Entry,
  IntegrityCheck,
  Compression,
  IllustrationInfo,
  Blob,
  Searcher,
  Query,
  SuggestionSearcher,
  Creator,
  StringProvider,
  FileProvider,
  StringItem,
  FileItem,
  getClusterCacheMaxSize,
  getClusterCacheCurrentSize,
  setClusterCacheMaxSize,
} = bindings("zim_binding");
