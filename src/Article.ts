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
    shouldIndex: boolean;
    bufferData: Buffer;

    constructor(props: {
        url: string,
        data: Buffer | string,
        ns?: string,
        mimeType?: string,
        title?: string,
        redirectAid?: string,
        aid?: string,
        fileName?: string,
        shouldIndex?: boolean
    }, ...args: any[]) {
        if (args.length) {
            // CPP hack
            const url = props
            props = { url } as any
            const keys = ['data', 'ns', 'mimeType', 'title', 'redirectAid', 'aid', 'fileName', 'shouldIndex']
            for (let i in args) {
                (props as any)[keys[i]] = args[i]
            }
        }
        props.ns = props.ns || '.'
        props.title = props.title || ''
        props.fileName = props.fileName || ''
        props.shouldIndex = props.shouldIndex || false
        props.redirectAid = props.redirectAid || ''
        props.aid = props.aid || `${props.ns}/${props.url}`

        if (!props.mimeType) props.mimeType = mime.getType(props.url) || 'text/plain';

        this.ns = props.ns;
        this.aid = props.aid;
        this.url = props.url;
        this.title = props.title;
        this.mimeType = props.mimeType;
        this.redirectAid = props.redirectAid;
        this.fileName = props.fileName;
        this.shouldIndex = props.shouldIndex;
        this.bufferData = props.data && Buffer.from(props.data as any);
    }

    fromJS(output: any) {
        output(this.ns,
            this.aid,
            this.url,
            this.title,
            this.mimeType,
            this.redirectAid,
            this.fileName,
            this.shouldIndex,
            this.bufferData);
    }
}

binding.bind('Article', ZimArticle);

export { ZimArticle }