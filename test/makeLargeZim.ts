import * as fs from 'fs/promises';
import * as path from 'path';
import * as faker from 'faker';
import { Creator, Archive, StringItem } from '../src';

const numArticles = 1000000;
const outFile = path.join(__dirname, '../largeZim.zim');

console.log(`Making ZIM file with [${numArticles}] articles`);


(async () => {
  console.info('Starting');
  const creator = new Creator()
    .configIndexing(true, "en")
    .configClusterSize(2048)
    .startZimCreation(outFile);

  console.info(`Created Zim, writing items...`);

  for (let i = 0; i < numArticles; i++) {
    const title = `${i}_${faker.lorem.words(faker.random.number({min: 1, max: 4}))}`;
    const url = title.replace(/ /g, '_');
    const data = faker.lorem.paragraphs(10);

    const item = new StringItem(
      url,
      "text/html",
      title,
      {FRONT_ARTICLE: 1},
      data,
    );
    creator.addItem(item);
  }

  console.log(`Finalising...`);
  await creator.finishZimCreation();
  console.log('Done Writing');

  const archive = new Archive(outFile);
  console.info(`Count Articles:`, archive.articleCount);

  await fs.unlink(outFile);
})();
