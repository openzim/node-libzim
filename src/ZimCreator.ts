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
    Language?: string,
    Name?: string,
    Publisher?: string,
    Tags?: string,
    Title?: string,
    favicon?: string
}

class ZimCreator {
    tmpDir: string;
    _c: any;
    fileName: string;
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
        this._c = lib.ZimCreatorManager.create(
            opts.fileName,
            opts.welcome || '',
            opts.fullTextIndexLanguage || '',
            opts.minChunkSize || 2048
        );
    }

    addArticle(article: ZimArticle) {
        return new Promise((resolve, reject) => {
            lib.ZimCreatorManager.addArticle(this._c, article, (err: any) => {
                if (err) reject(err);
                else resolve();
            });
        });
    }

    async setFaviconMetadata(favicon: string) {
        const article = new ZimArticle('', '', '-', 'image/png', favicon, '/-/Favicon');
        await this.addArticle(article);
    }

    async setMetadata(metadata: ZimMetadata) {
        const keys = Object.keys(metadata).filter(key => (metadata as any)[key]);

        for (let key of keys) {
            const content = (metadata as any)[key];
            if (key === 'favicon') {
                await this.setFaviconMetadata(content);
            } else {
                const article = new ZimArticle(key, content, 'M');
                await this.addArticle(article);
            }
        }
    }

    finalise() {
        return new Promise((resolve, reject) => {
            lib.ZimCreatorManager.finalise(this._c, () => {
                resolve();
            });
        });
    }
}

export { ZimCreator }