import * as fs from "fs/promises";
import * as path from "path";
import { Creator, Archive, StringItem } from "../src";

const tqdm = require("tqdm");

const numArticles = 1000000;
// const numArticles = 10000;
const outFile = path.join(__dirname, "../largeZim.zim");

console.log(`Making ZIM file with [${numArticles}] articles`);

(async () => {
  console.info("Starting");
  const creator = new Creator()
    .configNbWorkers(1)
    .configIndexing(true, "en")
    .configClusterSize(2048)
    .startZimCreation(outFile);

  console.info("Created Zim, writing items...");

  function* rangeGenerator(N = numArticles) {
    for (let i = 0; i < N; i++) {
      yield i;
    }
  }

  for (const i of tqdm(rangeGenerator(numArticles), { total: numArticles })) {
    // for (let i = 0; i < numArticles; i++) {
    // const title = `${i}_${faker.lorem.words(faker.random.number({min: 1, max: 4}))}`;
    const title = `test ${i}`;
    const url = title.replace(/ /g, "_");
    // const data = faker.lorem.paragraphs(10);
    const data = `hello world ${i}`;

    const stringItem = new StringItem(
      url,
      "text/html",
      title,
      { FRONT_ARTICLE: 1 },
      data
    );

    /*
    const customItem = { // custom item
      path: url,
      mimeType: "text/html",
      title: title,
      hints: {FRONT_ARTICLE: 1},
      getContentProvider() { // custom content provider
        let sent = false;
        return {
          size: data.length,
          feed() {
            if(!sent) {
              sent = true;
              return new Blob(data);
            }
            return new Blob();
          }
        };
      },
    };
    */

    await creator.addItem(stringItem);
  }

  console.log("Finalizing...");
  await creator.finishZimCreation();
  console.log("Done Writing");

  const archive = new Archive(outFile);
  console.info("Count Articles:", archive.articleCount);

  await fs.unlink(outFile);
})();
