# Example minimal website with map and C++ backend.

This website is meant to be a minimal starting point for deploying a C++ back-end that generates data to be shown on a geographical map.

This website has been developed with design principles to promote responsiveness and good mobile usability:

- Mobile-first stylesheets.
- Minimize 'clutter'; maximize 'content'.
- Show something soon; indicate more is coming.
- Minimize reloading heavy content.

## Framework

Note that there are several widely used frameworks that can be used to make it easier to build and maintain the website, such as React, Vue, and Angular.
This website does not use these frameworks to keep it lightweight, but they are definately worth considering before the website becomes large.

## Style

The HTML and CSS webpage files follow the Google HTML/CSS style guide (https://google.github.io/styleguide/htmlcssguide.html) with the following changes:

- Ignore section 3.1.7 on optional elements: always keep the <head> and <body> tags for separation of concerns (e.g. separating metadata from data).
- Ignore section 4.1.8 on hexadecimal notation: always use 6- or 8-character notation. Shorter notations obfuscate the actual value.
- All style classes for general use within the group use the "myorg-" prefix. All style classes for specific use within a project use a short prefix based on the project name.

The JavaScript files follow the Google JavaScript style guide (https://google.github.io/styleguide/jsguide.html) with the following changes:

- Ignore section 9.3.2 on using clang-format. Instead, when using VS Code, use the Prettier plugin to format the code. One of the reasons is that clang-format violates section 4.1.2 on closing brace placement.
- Instead of using 4+ spaces fo continuation indentation (section 4.5.2), continuations are idented by 2 spaces compared to the original.
- Whenever spreading function parameters/arguments over multiple lines, always put the first one on a new line. Prefer to use exactly 1 parameter/argument per line.
- When naming elements in camel case, only capitalize the first letter of an abbreviation.

