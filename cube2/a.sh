set -e
~/Dev/binaryen/bin/wasm-opt original.wat -O -o a.wasm -all
~/Dev/binaryen/bin/wasm-opt expermnt.wat -O -o b.wasm -all
ls -al a.wasm b.wasm

