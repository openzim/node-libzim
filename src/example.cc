#include <string>
#include <iostream>

#include "nbind/api.h"

class Example {

public:

	static std::string test() {
#		if defined(BUILDING_NODE_EXTENSION)
			return("native");
#		elif defined(EMSCRIPTEN)
			return("asm.js");
#		else
			return("unknown");
#		endif
	}

};

#include "nbind/nbind.h"

#ifdef NBIND_CLASS

NBIND_CLASS(Example) {
	method(test);
}

#endif
