import { binding, lib } from './binding';

import * as mime from 'mime';

class ZimArticle {
    ns: string;
    aid: string;
    url: string;
    title: string;
    mimeType: string;
    redirectAid: string;
    fileName: string;
    bufferData: Buffer;

    constructor(
        url: string,
        data: Buffer | string,
        ns: string = '.',
        mimeType?: string,
        title: string = '',
        redirectAid: string = '',
        aid: string = `${ns}/${url}`,
        fileName: string = '',
    ) {
        if (!mimeType) mimeType = mime.getType(url) || 'text/plain';

        this.ns = ns;
        this.aid = aid;
        this.url = url;
        this.title = title;
        this.mimeType = mimeType;
        this.redirectAid = redirectAid;
        this.fileName = fileName;
        this.bufferData = data && Buffer.from(data as any);
    }

    fromJS(output: any) {
        output(this.ns,
            this.aid,
            this.url,
            this.title,
            this.mimeType,
            this.redirectAid,
            this.fileName,
            this.bufferData);
    }
}

binding.bind('Article', ZimArticle);

export { ZimArticle }