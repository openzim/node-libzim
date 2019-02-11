import { ZimArticle, ZimCreator, ZimReader } from "./";

(async function () {

    console.info('Starting');
    const creator = new ZimCreator({
        fileName: 'test.zim',
        welcome: 'file4',
        fullTextIndexLanguage: 'eng',
        minChunkSize: 2048,
    }, { favicon: './file2' });
    console.info(`Created Zim`);

    for (let i = 200; i > 0; i--) {
        const a = new ZimArticle(`file${i}`, `Content ${i} asdf as dfasd f`);
        await creator.addArticle(a);
    }

    await creator.finalise();

    console.log('Done Writing');


    // const zimFile = new ZimReader('test.zim');

    // const suggestResults = await zimFile.suggest('Content');
    // console.info(`Suggest Results:`, suggestResults);

    // const searchResults = await zimFile.search('Content');
    // console.info(`Search Results:`, searchResults);

    // const article3Content = await zimFile.getArticleByUrl('./file3');
    // console.info(`Article by url (article3):`, article3Content);

})();