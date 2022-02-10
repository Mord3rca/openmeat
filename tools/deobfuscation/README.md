# Break Loader

## Usage

Use `ffdec` to export binary data of `deadmeat.swf` and use the break loader script

```
wget https://www.deadmaze.com/alpha/deadmeat.swf
ffdec -export binarydata data/ deadmeat.swf
python3 break_loader.py data/
```

## Dropped method

For now, this reversing method is dropped since the AS3 code is
really well obfuscated. All classes / methods / functions are encoded in
weird UTF-8 caracter.

The only path which can be exploited is to debug library access in the AS code.
