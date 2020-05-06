import * as path from 'path';
import * as faker from 'faker';
import {ZimArticle, ZimCreator, ZimReader} from '../src';

const numArticles = 1000000;
const outFile = path.join(__dirname, '../largeZim.zim');

console.log(`Making ZIM file with [${numArticles}] articles`);


(async () => {

  console.info('Starting');
  const creator = new ZimCreator({
    fileName: outFile,
    welcome: 'welcome',
    fullTextIndexLanguage: 'eng',
    minChunkSize: 2048,
  }, {});
  console.info(`Created Zim`);

  for (let i = 0; i < numArticles; i++) {
    const articleTitle = `${i}_${faker.lorem.words(faker.random.number({min: 1, max: 4}))}`;
    const articleUrl = articleTitle.replace(/ /g, '_');

    const a = new ZimArticle({
      url: articleUrl,
      title: articleTitle,
      data: faker.lorem.paragraphs(10),
      mimeType: 'text/html',
      shouldIndex: true,
      ns: 'A'
    });
    await creator.addArticle(a);
  }

  console.log(`Finalising...`);
  await creator.finalise();
  console.log('Done Writing');

  const zimFile = new ZimReader(outFile);
  const numberOfArticles = await zimFile.getCountArticles();
  console.info(`Count Articles:`, numberOfArticles);

  // await zimFile.destroy();
})();
