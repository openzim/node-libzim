import {statSync} from 'fs';
import * as rimraf from 'rimraf';

import {ZimArticle, ZimCreatorWrapper} from './zim';

export interface ZimCreatorOpts {
  fileName: string;
  welcome?: string;
  fullTextIndexLanguage?: string;
  minChunkSize?: number;
}

export interface ZimMetadata {
  Counter?: string, Creator?: string, Date?: string, Description?: string,
      Flavour?: string, Language?: string, Name?: string, Publisher?: string,
      Tags?: string, Title?: string,
}


class ZimCreator {
  tmpDir: string;
  _creator: any;
  fileName: string;
  articleCounter: {[mimeType: string]: number} = {};

  constructor(opts: ZimCreatorOpts, metadata: ZimMetadata = {}) {
    // {fileName, welcome = '', fullTextIndexLanguage = '', minChunkSize =
    // 2048}, metadata = {}) {
    this.fileName = opts.fileName;
    this.articleCounter = {};
    this.tmpDir = this.fileName.split('.').slice(0, -1).join('.') + '.tmp';
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
      console.error(err);
    }

    const {
      welcome = '',
      fullTextIndexLanguage = '',
      minChunkSize = 2048,
    } = opts;
    this._creator = new ZimCreatorWrapper({
      fileName: this.fileName,
      mainPage: welcome,
      fullTextIndexLanguage,
      minChunkSize: minChunkSize,
    });

    this.setMetadata(metadata);
  }

  get isAlive() {
    return !!this._creator;
  }

  async addArticle(article: any) {
    if (!this.isAlive) throw new Error(`This Creator has been destroyed`);
    this._creator.addArticle(article);
    if (!article.redirectUrl) {
      this.articleCounter[article.mimeType] =
          this.articleCounter[article.mimeType] || 0;
      this.articleCounter[article.mimeType] += 1
    }
    return article;
  }

  async setMetadata(metadata: ZimMetadata) {
    if (!this.isAlive) throw new Error(`This Creator has been destroyed`);
    const keys = Object.keys(metadata).filter(key => (metadata as any)[key]);
    for (const key of keys) {
      const content = (metadata as any)[key];
      const article = new ZimArticle({url: key, data: content, ns: 'M'});
      await this.addArticle(article);
    }
  }

  async finalise() {
    if (!this.isAlive) throw new Error(`This Creator has been destroyed`);
    const counterString = Object.keys(this.articleCounter)
                              .map(mimeType => `${mimeType} =
                                $ { this.articleCounter[mimeType] }`)
                              .join(';');
    await this.setMetadata({Counter: counterString});
    this._creator.finalise();
    this._creator = null;
  }
};

export {ZimCreator};
