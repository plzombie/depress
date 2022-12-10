=Hierarchy

Document
- DocumentFlags
- Flags
- Pages
- - Page
- - - Flags
- - - Filename		

== Document

Contains document flags, flags for all pages and element with pages.

== Pages

Contains individual pages.

== Page

Individual page. Contains Flags and Filename.

== DocumentFlags

Describes document parameters. No body. Arguments:

* ptt - Page title type. If ptt = 0 then no authomatic page titles. If ptt = 1 then use filename as default page title.
* ptt_flags - Page title type flags. If ptt = 1 and ptt_flags = 1 then use short filename instead of full filename.

== Flags

Describes page flags. No body. Arguments:

* type - 0 for color images, 1 for bw images.
* param1 - if type is 1 then 0 stands for threshold binarization, 1 stands for error diffusion, 2 stands for adaptive binarization.
* quality - image quality between 0 and 100.
* dpi - image dpi.

== Filename

Describes filename. Body is a filename.


