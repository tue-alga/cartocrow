=== GeoViz website ===

This directory contains the GeoViz website. The following descibes the subdirectory structure:

- bin: native applications.
- data: data stores like databases.
- page: referenced webpages.
- res: webpage resources like images and icons.
- script: javascript and other scripts.
- style: stylesheets.




Pros for using a css/layout framework

- Takes into account a lot of aspects of proper webdev.
- Mobile-first webdev.
- More likely to use correct practices/standards.
- Should take into account browser-specific quirks.
- Improve search engine support and media type support (other than screen).

Cons

- Less control over code.
- If framework changes, website changes (unless linking to sdpecific version of framework).
- May force online: must be downloaded every time.



I dislike w3.css because it heavily depends on floats, which do not always do what I want.
I dislike Boostrap slightly more, because it is a lot heavier (longer load times) and it depends on jQuery/JavaScript for layouting, which is depracated by html5.
