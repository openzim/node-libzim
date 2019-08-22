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
    isAlive = true;
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

        this._reader = lib.ZimReaderWrapper.create(
            fileName
        );
    }

    destroy() {
        this.isAlive = false;
        this._reader.destroy();
    }

    getCountArticles() {

        if (!this.isAlive) throw new Error(`This Reader has been destroyed`);
        return new Promise<number>((resolve, reject) => {
            this._reader.getCountArticles((err: any, count: number) => {
                if (err) reject(err);
                else resolve(count);
            });
        });
    }

    getArticleByUrl(articleId: string) {
        if (!this.isAlive) throw new Error(`This Reader has been destroyed`);
        return new Promise<ZimArticle>((resolve, reject) => {
            this._reader.getArticleByUrl(articleId, (err: any, result: ZimArticle, floatArray?: Float64Array) => {
                if (err) reject(err);
                else {
                    if (floatArray) {
                        const arrayBufferData = (floatArray.buffer as any).slice();
                        const bufferData = Buffer.from(arrayBufferData);
                        result.bufferData = bufferData;
                    }
                    resolve(result);
                }
            });
        });
    }

    getArticleById(articleId: number) {
        if (!this.isAlive) throw new Error(`This Reader has been destroyed`);
        return new Promise<ZimArticle>((resolve, reject) => {
            this._reader.getArticleById(articleId, (err: any, result: ZimArticle, floatArray?: Float64Array) => {
                if (err) reject(err);
                else {
                    if (floatArray) {
                        const arrayBufferData = (floatArray.buffer as any).slice();
                        const bufferData = Buffer.from(arrayBufferData);
                        result.bufferData = bufferData;
                    }
                    resolve(result);
                }
            });
        });
    }

    suggest(query: string) {
        if (!this.isAlive) throw new Error(`This Reader has been destroyed`);
        return new Promise((resolve, reject) => {
            this._reader.suggest(query, (err: any, result: any) => {
                if (err) reject(err);
                else {
                    resolve(result);
                }
            });
        });
    }

    search(query: string) {
        if (!this.isAlive) throw new Error(`This Reader has been destroyed`);
        return new Promise((resolve, reject) => {
            this._reader.search(query, (err: any, result: any) => {
                if (err) reject(err);
                else {
                    resolve(result);
                }
            });
        });
    }
}

export { ZimReader }