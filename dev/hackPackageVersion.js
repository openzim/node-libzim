import { writeFileSync } from 'fs';

import pkg, { version } from '../package.json';
version = `${version}-${Date.now()}`;
writeFileSync('./package.json', JSON.stringify(pkg), 'utf8');