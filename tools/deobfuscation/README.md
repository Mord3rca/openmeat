# Deobfuscation Tools

A set of tools to deobfuscate Deadmaze client code.

## bloader

Python script to break first obfuscation in **deadmaze.swf** which consist
of assembling back binary data to load its embedded SWF.

### Usage

```
export FFDEC_EXE="path/to/ffdec"
bloader
```

Will download **deadmeat.swf** in the current directory and create **reversed.swf**
if a reversing solution is found.

### Dropped method

For now, this reversing method is dropped since the AS3 code is
really well obfuscated. All classes / methods / functions are encoded in
weird UTF-8 caracter.

The only path which can be exploited is to debug library access in the AS code.
