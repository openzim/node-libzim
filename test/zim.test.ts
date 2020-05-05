// noinspection ES6UnusedImports
import {} from 'ts-jest';
import path from "path";
import * as faker from 'faker';

import {ZimArticle, ZimCreator, ZimReader} from "../src";
import * as fs from "fs";


let creator: ZimCreator;
let actualNumberOfArticles = 0;

const nthArticleId = 77;
let nthArticleUrl: string;

const targetNumberOfArticles = 100;
const targetNumberOfRecords = targetNumberOfArticles + 6;
const targetNumberOfRecordsWithMeta = targetNumberOfRecords + 2;

const outFile = path.join(__dirname, '../../test.zim');


beforeAll(() => {
  removeOutFile();
  creator = new ZimCreator({
    fileName: outFile,
    welcome: 'welcome',
    fullTextIndexLanguage: 'eng',
    minChunkSize: 2048,
  }, {});
});


describe('ZimCreator', () => {

  beforeAll(async () => {
    const articleContent = faker.lorem.paragraphs(1);
    for (let i = 0; i < targetNumberOfArticles; i++) {
      const articleTitle = faker.lorem.words(faker.random.number({min: 1, max: 3}));
      const articleUrl = `${articleTitle.replace(/ /g, '_')}_${i}`;

      if (i == nthArticleId) nthArticleUrl = articleUrl;

      const a = new ZimArticle({
        url: articleUrl,
        data: articleContent,
        title: articleTitle,
        mimeType: 'text/html',
        shouldIndex: true,
        ns: 'A'
      });
      await creator.addArticle(a);
      actualNumberOfArticles++;
    }

    await creator.addArticle(new ZimArticle({ns: 'M', data: `meta test`, url: 'meta_test'}));
    await creator.addArticle(new ZimArticle({ns: 'I', data: 'image test', url: 'image_test', mimeType: 'image/png'}));
    await creator.addArticle(new ZimArticle({ns: '-', data: 'common namespace test', url: 'common_namespace_test'}));

    await creator.finalise();
  });


  test('Zim file exists', () => {
    const isExists = fs.existsSync(outFile);
    expect(isExists).toBeTruthy();
  });

  test(`Created ${targetNumberOfArticles} articles`, () => {
    const count = creator.articleCounter['text/html'];
    expect(count).toEqual(targetNumberOfArticles);
  });

  test(`Created ${targetNumberOfArticles} records`, () => {
    const count = Object.values(creator.articleCounter).reduce((a, c) => a + c, 0);
    expect(count).toEqual(targetNumberOfRecords);
  });


});


describe('ZimReader', () => {

  let zimFile: ZimReader;

  beforeAll(async () => {
    zimFile = new ZimReader(outFile);
  });


  test(`Count records`, async () => {
    const articlesCount = await zimFile.getCountArticles();
    expect(articlesCount).toEqual(targetNumberOfRecordsWithMeta);
  });

  test(`Get Nth (${nthArticleId}) article`, async () => {
    const article = await zimFile.getArticleById(nthArticleId);
    expect(article).toBeDefined();
  });

  test(`Get article by url`, async () => {
    const article = await zimFile.getArticleByUrl(`A/${nthArticleUrl}`);
    expect(article).toBeDefined();
  });

  test(`Get meta record`, async () => {
    const article = await zimFile.getArticleByUrl(`M/meta_test`);
    expect(article).toBeDefined();
    expect(article.mimeType).toEqual('text/plain');
  });

  test(`Get common record`, async () => {
    const article = await zimFile.getArticleByUrl(`-/common_namespace_test`);
    expect(article).toBeDefined();
    expect(article.mimeType).toEqual('text/plain');
  });

  test(`Get image`, async () => {
    const article = await zimFile.getArticleByUrl(`I/image_test`);
    expect(article).toBeDefined();
    expect(article.mimeType).toEqual('image/png');
  });

  test(`Suggest`, async () => {
    const article = await zimFile.suggest('laborum');
    expect(article).toBeDefined();
  });

  test(`Search`, async () => {
    const article = await zimFile.suggest('rem');
    expect(article).toBeDefined();
  });


  afterAll(async () => {
    await zimFile.destroy();
  })
});


afterAll(() => {
  removeOutFile();
});


const removeOutFile = () => {
  try {
    fs.unlinkSync(outFile);
  } catch (e) {}
}
