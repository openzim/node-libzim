import { ZimArticle, ZimCreator } from "./";

(async function () {
    console.info('Starting');
    const creator = new ZimCreator('test.zim', { welcome: 'file4', favicon:'./file2' });
    console.info(`Created Zim`);

    for (let i = 100; i > 0; i--) {
        const a = new ZimArticle(`file${i}`, `Content ${i} asdf as dfasd f`);
        await creator.addArticle(a);
    }

    await creator.finalise();

    console.log('Done');
})();