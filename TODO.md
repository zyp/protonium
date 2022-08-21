# TODO

## Language features 
* `repeated`
  * Autodetect whether the field is packed when decoding.
* `optional`
* `package`
* `map`

## C++ API
* Length-constrained container class.
* Proper bytes/string support.
  * Static/dynamic/reference
* Namespacing
  * Packages
  * Nested types
  * `namespace protonium`
* Iterator constraints
* Async integration
* RPC client?

## Python API
* RPC server?

## Packaging
* Build `options.proto` and `rpc.proto` from `setup.py`

## CLI
* Add an option to copy headers to supplied directory.

## Documentation
* All of it

## Tests
* All of those as well

## Corner cases
* Messages may be used in a .proto before they are defined, and protoc supplies them in the definition order.
  This results in invalid C++ if we don't sort them before generating.
