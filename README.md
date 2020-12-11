# tinydomxx

Simple DOM Generator for C++/WebAssembly.


# example
```
extern "C"
void
test_insert_link(jsref *p)
{
    JSObj o(*p);
    JSObj a = document.createElement("a");
    JSObj str = document.createTextNode("hello");

    a.setAttribute("href", "http://example.com/");
    a.appendChild(str);

    o.appendChild(a);
}
```

this program generate link to example.com and insert it to passed object.

# build

```
    $ mkdir build
    $ cd build
    $ emcmake cmake ..
    $ make
```

# use with emscripten

Link `libtinydomxx_emscripten.js` to your C++ code via emcc `--js-library` option.

```
 $ emcc --js-library libtinydomxx_emscripten.js your_c++_program.cpp
```

# pass JavaScript Object to C++ (with emscripten)

Use `tinydomxx_ccall` instead of `ccall`.

`tinydomxx_ccall` converts your javascript object to c++ reference pointer.

