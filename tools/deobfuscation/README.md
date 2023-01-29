# Break Loader

## Usage

```
export FFDEC_EXE="path/to/ffdec"
python3 break_loader.py <output_filename>
```

## Dropped method

For now, this reversing method is dropped since the AS3 code is
really well obfuscated. All classes / methods / functions are encoded in
weird UTF-8 caracter.

The only path which can be exploited is to debug library access in the AS code.
