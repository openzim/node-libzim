import { binding, lib } from './binding';
import { ZimArticle } from "./Article";

import * as rimraf from 'rimraf';
import { statSync } from 'fs';

export interface ZimCreatorOpts {
    fileName: string;
    welcome?: string;
    fullTextIndexLanguage?: string;
    minChunkSize?: number;
}

class ZimCreatorWrapper {
    constructor(opts: ZimCreatorOpts) {

    }
}

binding.bind('ZimCreatorWrapper', ZimCreatorWrapper);

export interface ZimMetadata {
    Counter?: string,
    Creator?: string,
    Date?: string,
    Description?: string,
    Flavour?: string,
    Language?: string,
    Name?: string,
    Publisher?: string,
    Tags?: string,
    Title?: string,
}

class ZimCreator {
    isAlive = true;
    tmpDir: string;
    _c: any;
    fileName: string;
    articleCounter: { [mimeType: string]: number } = {};

    constructor(opts: ZimCreatorOpts, metadata: ZimMetadata = {}) {
        const fileName = opts.fileName;
        this.tmpDir = fileName.split('.').slice(0, -1).join('.') + '.tmp';
        this.fileName = fileName;

        const metadataDefaults = {
            Date: (new Date()).toJSON().split('T')[0],
            Language: 'eng',
        };
        metadata = Object.assign(metadataDefaults, metadata);

        try {
            const statRes = statSync(this.tmpDir);
            console.info(`Tmp directory already exists, deleting [${this.tmpDir}]`);
            rimraf.sync(this.tmpDir);
        } catch (err) {
        }

        this._createZimCreator(opts);

        this.setMetadata(metadata);
    }

    _createZimCreator(opts: ZimCreatorOpts) {
        this._c = lib.ZimCreatorWrapper.create(
            opts.fileName,
            opts.welcome || '',
            opts.fullTextIndexLanguage || '',
            opts.minChunkSize || 2048
        );
    }

    addArticle(article: ZimArticle) {
        if(!this.isAlive) throw new Error(`This Creator has been destroyed`);
        const self = this;
        return new Promise((resolve, reject) => {
            this._c.addArticle(article, (err: any) => {
                if (err) reject(err);
                else {
                    if (!article.redirectUrl) {
                        self.articleCounter[article.mimeType] = self.articleCounter[article.mimeType] || 0
                        self.articleCounter[article.mimeType] += 1
                    }
                    resolve();
                }
            });
        });
    }

    async setMetadata(metadata: ZimMetadata) {
        if(!this.isAlive) throw new Error(`This Creator has been destroyed`);
        const keys = Object.keys(metadata).filter(key => (metadata as any)[key]);

        for (let key of keys) {
            const content = (metadata as any)[key];
            const article = new ZimArticle({ url: key, data: content, ns: 'M' });
            await this.addArticle(article);
        }
    }

    async finalise() {
        if(!this.isAlive) throw new Error(`This Creator has been destroyed`);
        const self = this;
        const counterString = Object.keys(self.articleCounter).map(mimeType => `${mimeType}=${self.articleCounter[mimeType]}`).join(';')
        await self.setMetadata({
            Counter: counterString
        });
        this.isAlive = false;
        return new Promise((resolve, reject) => {
            this._c.finalise(() => {
                resolve();
            });
        });
    }
}

export { ZimCreator }