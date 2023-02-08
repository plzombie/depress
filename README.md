# Depress ( [ru](README.ru.md) )

Software to automatize creation of djvu files.

## How to use

* Compile software (or download binaries from [release page](https://github.com/plzombie/depress/releases))
* Install [djvulibre](https://sourceforge.net/projects/djvu/files/)
* Copy `depress.exe` to djvulibre folder (i.e. "c:\Program Files (x86)\DjVuLibre\")
* Run `depress.exe pictures_list.txt output.djvu`

See [documentation](doc/DEPRESS.md).

## License

The software itself licensed under [BSD license](https://github.com/plzombie/depress/blob/master/LICENSE). If you use it with djvulibre it will be considered as combined work with djvulibre. So it will be licensed under GNU GPL.

### Dependencies

* [djvulibre](https://djvu.sourceforge.net/) - [GNU GPL 2](https://opensource.org/licenses/GPL-2.0)
* [stb](https://github.com/nothings/stb) - [Unlicense/MIT](https://github.com/nothings/stb/blob/master/LICENSE)
* [djvul](https://github.com/ImageProcessing-ElectronicPublications/stb-image-djvul) - [Unlicense](https://github.com/ImageProcessing-ElectronicPublications/stb-image-djvul/blob/main/LICENSE)
