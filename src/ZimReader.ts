import { binding, lib } from './binding';
import { ZimArticle } from "./Article";

import * as rimraf from 'rimraf';
import { statSync } from 'fs';


class ZimReaderWrapper {
    constructor() {

    }
}

binding.bind('ZimReaderWrapper', ZimReaderWrapper);

class ZimReader {
    _reader: any;
    fileName: string;
    constructor(fileName: string) {
        this.fileName = fileName;

        try {
            const statRes = statSync(fileName);
        } catch (err) {
            console.warn(`File [${fileName}] does not exist`);
            throw err;
        }

        this._reader = lib.ZimReaderManager.create(
            fileName
        );
    }

    getArticleByUrl(articleId: string) {
        return new Promise<ZimArticle>((resolve, reject) => {
            lib.ZimReaderManager.getArticleByUrl(this._reader, articleId, (err: any, result: ZimArticle, floatArray: Float64Array) => {
                if (err) reject(err);
                else {
                    const arrayBufferData = (floatArray.buffer as any).slice();
                    const bufferData = Buffer.from(arrayBufferData);
                    result.bufferData = bufferData;
                    resolve(result);
                }
            });
        });
    }

    suggest(query: string) {
        return new Promise((resolve, reject) => {
            lib.ZimReaderManager.suggest(this._reader, query, (err: any, result: any) => {
                if (err) reject(err);
                else {
                    resolve(result);
                }
            });
        });
    }

    search(query: string) {
        return new Promise((resolve, reject) => {
            lib.ZimReaderManager.search(this._reader, query, (err: any, result: any) => {
                if (err) reject(err);
                else {
                    resolve(result);
                }
            });
        });
    }
}

export { ZimReader }