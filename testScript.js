
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
    testFunc,
} = require("bindings")('zim_binding');

const archive = new Archive({ fileName: "./vegetarianism.stackexchange.com_en_all_2021-05.zim" });
//const entry1 = archive.getEntryByPath(0);
//console.log(entry1.path);
//console.log(archive.getEntryByPath(1).path);

//global.gc();

// console.log({ t1: archive.getEntryByPath(0), t2: archive.getEntryByPath(1) });

let val = archive.iterByPath();
if(val instanceof Entry) {
  console.log("val instanceof Entry");
  console.log("val:", val);
  console.log("val.valid:", val.valid);
  console.log("val.title:", val.title);
} else if(val instanceof Object && "test1" in val) {
  console.log("val instanceof Object && test1 in val");
  console.log("val:", val);
  console.log("val.test1:", val.test1);
  console.log("val.test1.valid:", val.test1.valid);
  console.log("val.test1.title:", val.test1.title);
  return 0;
} else if(val instanceof Object && Symbol.iterator in val) {
  console.log("val instanceof Object && Symbole.iterator in val");
  console.log("val:", val);
  console.log("val[Symbol.iterator]:", val[Symbol.iterator]);
  console.log("val.size:", val.size);

  const iter = val[Symbol.iterator]();
  console.log("iter:", iter);
  console.log("iter.next:", iter.next);
  let next = iter.next();
  console.log("next:", next);
  console.log("next.value:", next.value);
  console.log("next.value.valid:", next.value.valid);
  console.log("next.value.title:", next.value.title);

  const values = [];
  if(next.value.valid) {
    let count = 0;
    for(const entry of val) {
      if(count >= 10) {
        break;
      }
      console.log(`entry ${count}: title=${entry.title}`);
      values.push(entry);
      count++;
    }
  }

  console.log("\n================");
  let offset = 3;
  let range = val.offset(3, 7);
  console.log("range:", range);
  range.size = 100;
  console.log("range.size:", range.size)
  let count = 0;
  for(const entry of range) {
    console.log(`entry ${count+offset}: title=${entry.title}`);
    count++;
  }
}

console.log("\n===================");
(() => {
  const range = archive.findByPath("-/favicon");
  console.log("findByPath('favicon'):", range);
  let count  = 0;
  for(const entry of range) {
    if(count >=10) break;
    console.log(`entry ${count}: title=${entry.title}`);
    count++;
  }
})();

console.log("\n===================");
(() => {
  console.log("archive.checksum:", archive.checksum);
  //console.log("archive.check:", archive.check());
})();

console.log("\n===================");
(() => {
  console.log("IntegrityCheck", IntegrityCheck);
  console.log("archive", archive);
  console.log("Archive", Archive);
  console.log("blob:", new Blob());
  console.log("Blob:", Blob);
})();

console.log("\n==================");
(() => {
  let result = null;
  //result = archive.checkIntegrity(IntegrityCheck.CHECKSUM);
  console.log("archive.checkIntegrity:", result);
  //console.log("Archive.validate:", Archive.validate(archive.filename, [ IntegrityCheck.COUNT, IntegrityCheck.CHECKSUM ]));
})();

console.log("\n=====================");
(() => {
  let item = archive.getIllustrationItem();
  console.log("item:", item);
  console.log("item.index:", item.index);
  console.log("item.directAccessInformation:", item.directAccessInformation);
})();


console.log("\n=====================");
(() => {
    console.log("Searcher:", Searcher);
    console.log("Query:", Query);
    console.log();

    let searcher = new Searcher([archive]);
    console.log("searcher:", searcher);
    console.log("setVerbose(true):",  searcher.setVerbose(1));
    console.log("addArchive:", searcher.addArchive(archive));

    let query = new Query("hello world");
    console.log("query: ", query, query.query);

    let search = searcher.search(query);
    console.log("search:", search);
    //console.log("search.estimatedMatches:", search.estimatedMatches);

    search = searcher.search("hello world");
    console.log("search:", search);
    console.log("search.estimatedMatches:", search.estimatedMatches);

    let resultSet = search.getResults(0, 10);
    console.log("resultSet:", resultSet);
    console.log("resultSet.size:", resultSet.size);
    console.log("resultSet.iterator:", resultSet[Symbol.iterator]);

    let iter  = resultSet[Symbol.iterator]();
    console.log("iter:", iter);

    let count = 0;
    for(const res of resultSet) {
        if(count >= 10) break;
        const entry = res.entry;
        console.log();
        console.log(`${count} res:`, res);
        console.log(`${count} res.zimId:`, res.zimId);
        console.log(`${count} res.snippet:`, res.snippet);
        console.log(`${count} entry:`, entry);
        console.log(`${count} entry.title: ${entry.title}`);
        count++;
    }
})();

console.log("\n=====================");
(() => {
    console.log("SuggestionSearcher:", SuggestionSearcher);
    console.log();

    let searcher = new SuggestionSearcher(archive);
    console.log("searcher:", searcher);
    console.log("setVerbose(true):",  searcher.setVerbose(1));

    let search = searcher.suggest("vegan");
    console.log("search:", search);
    console.log("search.estimatedMatches:", search.estimatedMatches);

    let results = search.getResults(0, 10);
    console.log("results:", results);
    console.log("results.size:", results.size);

    let count = 0;
    for(const res of results) {
        if(count >= 10) break;
        console.log();
        console.log(`${count} res:`, res);
        console.log(`${count} res.title`, res.title);
        console.log(`${count} res.snippet`, res.snippet);
        console.log(`${count} res.entry`, res.entry);
        console.log(`${count} res.entry.title`, res.entry.title);
        count++;
    }
})();

console.log("\n=====================");
(() => {
    console.log("Compression", Compression);
    console.log("Compression.Zstd", Compression.Zstd);
    console.log("Creator", Creator);
    console.log("StringProvider", StringProvider);
    console.log("FileProvider", FileProvider);

    (() => {
        const sp1 = new StringProvider("hello");
        console.log(sp1, sp1.size, sp1.feed().data.toString());

        const fp1 = new FileProvider("CPPLINT.cfg");
        console.log(fp1, fp1.size, fp1.feed().size);
    })();

    const sp = new StringProvider("hello");
    console.log("testFunc:", testFunc(sp));

    // Does my custom provider work? Yes
    (() => {
        const data = "hello world";
        let feeded = false;
        const customProvier = {
            getSize: () => data.length,
            feed: () => {
                if(!feeded) {
                    feeded = true;
                    return new Blob(data);
                };
                return new Blob();
            },
        };
        console.log("testFunc:", testFunc(customProvier));
    })();

    // can I override class bindings? Yes.
    (() => {
        class MyContentProvider extends StringProvider {
            get size() {
                return 0;
            }
        };

        const mycp = new MyContentProvider("hello world");
        console.log("mycp:", mycp, mycp.size, mycp.feed().data.toString());
    })();

    console.log("");
    console.log("StringItem:", StringItem);
    console.log("FileItem:", FileItem);

    const si1 = new StringItem("A/test", "text/plain", "Hello world", {}, "Hello world 2!");
    console.log("si1:", si1);
    console.log("si1.contentProvider", si1.contentProvider, si1.contentProvider.size, si1.contentProvider.feed().data.toString());

    console.log("");
    const creator = (new Creator()).configVerbose(true).configIndexing(true, "en").startZimCreation("./test.zim");
    console.log("creator:", creator);
    for(let i = 0; i < 10; i++) {
        const item = new StringItem(`A/test${i}`, "text/plain", `Hello world ${i}`, {}, `Hello world ${i}!`);;
        creator.addItem(item);
    }
    creator.finishZimCreation();
})();
