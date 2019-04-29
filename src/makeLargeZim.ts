import { ZimArticle, ZimCreator, ZimReader } from ".";

(async function () {

    console.info('Starting');
    const creator = new ZimCreator({
        fileName: 'test.zim',
        welcome: 'file4',
        fullTextIndexLanguage: 'eng',
        minChunkSize: 2048,
    }, { favicon: './file2' });
    console.info(`Created Zim`);

    for (let i = 100000 * 100; i > 0; i--) {
        const a = new ZimArticle({ url: `file${i}`, data: `Content ${i} asdf as dfasd f` });
        await creator.addArticle(a);
    }

    await creator.finalise();

    console.log('Done Writing');
})();