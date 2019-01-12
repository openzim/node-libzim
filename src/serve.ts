import * as fs from 'fs';
import * as url from 'url';
import * as path from 'path';
import * as http from 'http';

const codeRoot = __dirname;
const docRoot = path.resolve(__dirname, '../public');
const port = 8080;

const encoding = '; charset=utf-8';

const mimeTbl: {[extension: string]: string} = {
	css: 'text/css' + encoding,
	html: 'text/html' + encoding,
	js: 'text/javascript',
	mem: 'application/octet-stream',
	png: 'image/png'
};

function sendStatus(
	res: http.ServerResponse,
	status: number,
	header?: { [key: string]: string | number }
) {
	const body = new Buffer(status + '\n', 'utf-8');

	header = header || {};

	header['Content-Type'] = 'text/plain';
	header['Content-Length'] = body.length;

	res.writeHead(status, header);
	res.end(body);
}

function route(urlPath: string) {
	const ext = path.extname(urlPath);
	let root = docRoot;

	if(ext == '.js' || ext == '.mem') root = codeRoot;

	return(path.join(root, urlPath.substr(1)));
}

http.createServer(
	(req: http.IncomingMessage, res: http.ServerResponse) => {
		const urlParts = url.parse(req.url);

		// Paths must start with / and contain maximum one consecutive potentially dangerous
		// special character between alphanumeric characters.

		const pathParts = urlParts.pathname.match(/^\/([@_0-9A-Za-z]+[-./]?)*/);

		// Reject all invalid paths.

		if(!pathParts) return(sendStatus(res, 403));

		let urlPath = pathParts[0];

		// Silently redirect obvious directory paths (ending with a slash) to an index file.

		if(urlPath.match(/\/$/)) urlPath += 'index.html';

		try {
			// Drop initial slash from path and use platform specific path
			// separators (for Windows).

			const filePath = route(urlPath).replace(/\//g, path.sep);

			const stats = fs.statSync(filePath);

			// Redirect accesses to directories not marked as such; append a slash.

			if(stats.isDirectory()) return(sendStatus(res, 302, {
				'Location': urlPath + '/' + (urlParts.search || '')
			}));

			// OK, serve the file.

			const extension = urlPath.substr(urlPath.lastIndexOf('.') + 1);

			res.writeHead(200, {
				'Content-Type': mimeTbl[extension] || 'text/plain' + encoding,
				'Content-Length': stats.size,

				// Don't cache anything, to make sure reloading gives latest version.

				'Cache-Control': 'no-cache, no-store, must-revalidate',
				'Expires': 0
			});

			fs.createReadStream(filePath).pipe(res);
		} catch(err) {
			sendStatus(res, 404);
		}
	}
).listen(port, () => {
	console.log('Listening on http://localhost:' + port + '/');
});
