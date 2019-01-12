nbind-example-universal
=======================

[![build status](https://travis-ci.org/charto/nbind-example-universal.svg?branch=master)](http://travis-ci.org/charto/nbind-example-universal)

This is an example of using [nbind](https://github.com/charto/nbind)
to distribute a native Node.js addon with an asm.js fallback,
also usable from web browsers.

The **optional** native addon is installed if you have one of the following C++ compilers:

- GCC 4.8 or above
- Clang 3.6 or above
- Visual Studio 2015 ([The Community version](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx) is fine)

Otherwise the asm.js fallback is used.

Usage
-----

First install:

```bash
git clone https://github.com/charto/nbind-example-universal.git
cd nbind-example-universal
npm install
```

Then run the example in Node.js with `npm test` or in the browser by running
`npm start` and navigating to [http://localhost:8080/](http://localhost:8080/).

Note that you shouldn't add the compiled asm.js code in version control.
In this example it's present in the `dist` directory simply to allow installing
directly from Github without having Emscripten available.

License
=======

This example is public domain.
