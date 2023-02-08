# Depress ( [en](README.md) )

Программа для автоматизации создания djvu файлов.

## How to use

* Соберите программу (или скачайте бинарники по [ссылке](https://github.com/plzombie/depress/releases))
* Установите [djvulibre](https://sourceforge.net/projects/djvu/files/)
* Скопируйте `depress.exe` в папку djvulibre (например "c:\Program Files (x86)\DjVuLibre\")
* Запустите `depress.exe pictures_list.txt output.djvu`

Смотрите [документацию](doc/DEPRESS.md).

## Лицензия

Программа сама по себе распространяется по лицензии [BSD](https://github.com/plzombie/depress/blob/master/LICENSE). Если вы используете её вместе с djvulibre, это будет считаться комбинированной работой с djvulibre. Таким образом, она будет под лицензией GNU GPL.

### Зависимости

* [djvulibre](https://djvu.sourceforge.net/) - [GNU GPL 2](https://opensource.org/licenses/GPL-2.0)
* [stb](https://github.com/nothings/stb) - [Unlicense/MIT](https://github.com/nothings/stb/blob/master/LICENSE)
* [djvul](https://github.com/ImageProcessing-ElectronicPublications/stb-image-djvul) - [Unlicense](https://github.com/ImageProcessing-ElectronicPublications/stb-image-djvul/blob/main/LICENSE)
