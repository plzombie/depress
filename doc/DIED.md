# Hierarchy

Document
- DocumentFlags
- Flags
- Pages
- - Page
- - - Flags
- - - Filename
- - - IllRect
- Outlines
- - Outline

## Document

Contains document flags, flags for all pages and element with pages.

## Pages

Contains individual pages.

## Page

Individual page. Contains Flags and Filename. Should always have closing tag.

## DocumentFlags

Describes document parameters. No body. Arguments:

* ptt - Page title type. If ptt = 0 then no authomatic page titles. If ptt = 1 then use filename as default page title.
* ptt_flags - Page title type flags. If ptt = 1 and ptt_flags = 1 then use short filename instead of full filename.

## Flags

Describes page flags. No body. Arguments:

* type - 0 for color images, 1 for bw images, 2 for layered images, 3 for palettized images
* param1
* * if type is 1 then 0 stands for threshold binarization, 1 stands for error diffusion, 2 stands for adaptive binarization.
* * if type is 2 then values from 1 describing foreground and background downsampling ratio.
* * if type is 3 then values from 2 to 256 describing number of colors.
* param2
* * if type is 2 then values from 1 describing foreground downsampling ratio (with respect to background downsampling ratio).
* * if type is 3 then value 0 stands for color quantization and value 1 stands for noteshrink algorithm.
* quality - image quality between 0 and 100.
* dpi - image dpi.
* title - page title.

## IllRect.

Rectangular illustrations. Define color regions on image. Can be only on black and white images. Arguments:

* x - x coordinate from 0 (left) to page width-1
* y - y coordinate from 0 (top) to page height-1
* width - width of rectangle
* height - height of rectangle

## Filename

Describes filename. Body is a filename.

## Outlines

Contains top level outlines.

## Outline

Describe single outline. Can contain child outlines in the body. Arguments:

* text - outline title
* page_id - page id (starting from 1)
