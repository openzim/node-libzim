/* Binding to libzim */
#include <node.h>
#include <v8.h>
#include "nan.h"

// Tell node about our module!
NAN_MODULE_INIT(RegisterModule) {
  Nan::HandleScope scope;

  // XXX DO SOMETHING USEFUL HERE XXX
}

NODE_MODULE(zim, RegisterModule)
