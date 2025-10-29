# res2h - pack resources into a C header file

C23 has `#embed` but with res2h you can bundle multiple assets neatly
and have them compressed via [lzav](https://github.com/avaneev/lzav).

## Usage

* compile with `make res2h`
* invoke `./res2h resources.h file1 ... fileN`
* include in your C project `#include "resources.h"`
* get resource data via `get_resource("file1", &size, &data)`

res2h runtime "API" is the single function `get_resource` which takes the
file name, a pointer to a `size_t` and a pointer to `uint8*`.

res2h will `malloc` the appropriate amount of memory for the uncompressed
resource.

### Use different memory allocator

To use a different memory allocator than `malloc`, define `RES2H_ALLOC` macro
before including the resource header.

Example:
```
Arena arena = {.max = 1024*1024};

#define RES2H_ALLOC(x) arena_alloc(&arena, (x))
#include "resources.h"
```

If no memory allocator macro is defined, then regular stdlib `malloc` is used.
Note that there is no function to free the unpacked resource data.
