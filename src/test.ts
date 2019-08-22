import { ZimArticle, ZimCreator, ZimReader } from "../";
import * as path from 'path';
import * as faker from 'faker';

const argv = require('minimist')(process.argv.slice(2));

const numArticles = argv.numArticles || 1000;

if (!argv.numArticles) {
    console.info(`No [--numArticles=] present, defaulting to 100`);
}

console.log(`Making ZIM file with [${numArticles}] articles`);

(async function () {

    console.info('Starting');
    const creator = new ZimCreator({
        fileName: 'test.zim',
        welcome: 'welcome',
        fullTextIndexLanguage: 'eng',
        minChunkSize: 2048,
    }, {});
    console.info(`Created Zim`);

    let prevPercentage = 0;

    let totalArticles = 0;

    const imageContent = Buffer.from(faker.image.image());
    const articleContent = faker.lorem.paragraphs(3);

    for (let i = 0; i < numArticles; i++) {
        const articleTitle = faker.lorem.words(faker.random.number({ min: 1, max: 4 }));
        const articleUrl = articleTitle.replace(/ /g, '_');

        const a = new ZimArticle({ url: articleUrl, data: articleContent, title: articleTitle, mimeType: 'text/html', shouldIndex: true, ns: 'A' });
        await creator.addArticle(a);
        totalArticles += 1;

        // const numCategories = faker.random.number({ min: 0, max: 10 });
        // for (let i = 0; i < numCategories; i++) {
        //     const articleTitle = 'Category:' + faker.lorem.words(faker.random.number({ min: 1, max: 4 }));
        //     const articleUrl = articleTitle.replace(/ /g, '_');
        //     const a = new ZimArticle({ url: articleUrl, data: articleContent, title: articleTitle, mimeType: 'text/html', shouldIndex: true, ns: 'A' });
        //     await creator.addArticle(a);
        //     totalArticles += 1;
        // }

        // const numRedirects = faker.random.number({ min: 0, max: 10 });
        // for (let i = 0; i < numRedirects; i++) {
        //     const articleTitle = faker.lorem.words(faker.random.number({ min: 1, max: 4 }));
        //     const redirectUrl = articleTitle.replace(/ /g, '_');
        //     const a = new ZimArticle({ url: redirectUrl, redirectUrl: articleUrl, data: '', title: articleTitle, mimeType: 'text/html', shouldIndex: true, ns: 'A' });
        //     await creator.addArticle(a);
        //     totalArticles += 1;
        // }

        // const numImgs = faker.random.number({ min: 0, max: 5 });
        // for (let i = 0; i < numImgs; i++) {
        //     const imgUrl = faker.random.alphaNumeric(faker.random.number({ min: 10, max: 25 }));
        //     const a = new ZimArticle({ url: imgUrl, data: imageContent, mimeType: 'image/png', ns: 'I' });
        //     await creator.addArticle(a);
        //     totalArticles += 1;
        // }

        const percentage = Math.floor(i / numArticles * 1000) / 10;
        if (percentage !== prevPercentage) {
            console.info(`Made article and dependencies [${i}/${numArticles}] [${Math.floor(i / numArticles * 1000) / 10}%] - total of [${totalArticles}] articles`);
            prevPercentage = percentage;
        }
    }

    console.log(`Finalising...`);

    await creator.finalise();

    console.log('Done Writing');


    const zimFile = new ZimReader(path.join(__dirname, '../test.zim'));

    const numberOfArticles = await zimFile.getCountArticles();
    console.info(`Count Articles:`, numberOfArticles);

    const firstArticle = await zimFile.getArticleById(0);
    console.info(`First Article:`, firstArticle);

    const suggestResults = await zimFile.suggest('laborum');
    console.info(`Suggest Results:`, suggestResults);

    const searchResults = await zimFile.search('rem');
    console.info(`Search Results:`, searchResults);

    const readArticleContent = await zimFile.getArticleByUrl('A/laborum');
    console.info(`Article by url (laborum):`, readArticleContent);

   
    await zimFile.destroy();
})();
