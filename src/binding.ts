import * as nbind from 'nbind';

const binding: nbind.Binding<any> = nbind.init(__dirname);
const lib = binding.lib;

export {
    binding,
    lib
}