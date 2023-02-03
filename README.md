# Pinapl 

Pinaple is not a programming language

## What is pinapl

Pinapl is the **study**-project for me to learn how compilers are made. I do not intend that anyone else would use it, but I cannot stop you.

## Build

To build the project you just run the build script:

```bash
$ ./build.sh
```

Possible script options include:
  - `clean` for deleting previously built binary files;
  - `rebuild` for rebuilding the whole project from scratch;
  - `build` the excessive option for adding to other options, if they happen to opt-out the build step, for example: `$ ./build.sh clean build`;
  - `nocrt` build without C runtime library (currently supported only for one specific architecture);

Currently the default build includes debug symbols and no optimization flags, since the project is not ready in any capacity and I do not intend you to use it anyway.
