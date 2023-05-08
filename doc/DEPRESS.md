# DEPRESS ( [ru](DEPRESS.ru.md) )

``` shell
depress [options] inputfile.txt outputfile.djvu
```

## Options

* `-bw` - create black and white document.
* `-errdiff` - use error diffusion (in combination with `-bw`).
* `-adaptive` - use adaptive threshold (in combination with `-bw`).
* `-layered` - create layered document (separate layers for backgroud and foreground).
* `-laydownall n` - sets downsampling ratio for background and foreground layers (in combination with `-layered`). Defaults to 3.
* `-laydownfg n` - sets further foreground downsampling ratio (`-laydownall 3` and `-laydownfg 2` gets foreground downsampling ratio 6). Defaults to 2.
* `-palettized` - create palettized document.
* `-palcolors n` - number of colors between 2 and 256 (defaults to 8).
* `-quant` - use quantization for palettized document.
* `-noteshrink` - use noteshrink for palettized document.
* `-pta` - Generates page title from full file name.
* `-shortfntitle` - Uses only file name (without path and extension) for page title (in combination with `-pta`).
* `-temp path` - defines temporary directory.
* `-quality n` - defines quality from 1 to 100 (defaults to 100).
* `-dpi n` - defines dpi (defaults to 100).

## Example

Create a text file (for example "C:\My Album\album.txt"). Put images names you want to have in your document into text file. Each line of file should have one image name. Image file name can be relative to text file path.

Run following command to create document:

``` shell
depress "C:\My Album\album.txt" "C:\My Album\album.djvu"
```

Let's imagine you want to create a textbook with illustrations scanned at 600 dpi. You can use following command:

``` shell
depress -quality 70 -dpi 600 -layered -laydownall 2 -laydownfg 2 "C:\My Album\album.txt" "C:\My Album\album.djvu"
```
