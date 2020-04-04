
import {ZimReaderWrapper} from './zim';

class ZimReader {
  _reader: any;

  constructor(fileName: string) {
    this._reader = new ZimReaderWrapper(fileName);
  }

  get isAlive() {
    return !!this._reader && this._reader.isOpen();
  }

  get fileName() {
    return this.isAlive ? this._reader.fileName : null;
  }

  destroy() {
    this._reader.close();
    this._reader = null;
  }

  async getCountArticles() {
    if (!this.isAlive) throw new Error(`This Reader has been destroyed`);
    return this._reader.getCountArticles();
  }

  async getArticleById(articleId: number) {
    if (!this.isAlive) throw new Error(`This Reader has been destroyed`);
    return this._reader.getArticleById(articleId);
  }

  async getArticleByUrl(articleUrl: string) {
    if (!this.isAlive) throw new Error(`This Reader has been destroyed`);
    return this._reader.getArticleByUrl(articleUrl);
  }

  async suggest(query: string) {
    if (!this.isAlive) throw new Error(`This Reader has been destroyed`);
    return this._reader.suggest(query);
  }

  async search(query: string) {
    if (!this.isAlive) throw new Error(`This Reader has been destroyed`);
    return this._reader.search(query);
  }
};

export {ZimReader}

