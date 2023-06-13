import bindings from "bindings";

const {
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

module.exports = {
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
};
