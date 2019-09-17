=== GeoViz code library ===

This directory contains the GeoViz C++ library. The following descibes the subdirectory structure:

- cmake : configuration and template files for CMake.
- console : command-line applications to expose the functionality of the library.
- geoviz : all the functional code. Note that I/O methods can be found with the relevant applications.

This library has been developed according to the following design principles to promote usability, sustainability, and platform independence:

- Separation of functional code and user interface code.
- Project configuration using CMake.
- Separate libraries and applications per method.

Note that within the CMakeLists files, we follow a loose naming convention:
- All variables are UPPER_CASE, all methods are lower_case.
- All project-specific CMake build options have the GEOVIZ_ prefix.
- Similar to CMake, most variables are [project_name]_<target_name>_<type> (e.g. GEOVIZ_INSTALL_SOURCE_DIR or NECKLACE_MAP_TARGET)._
- Applications exposing the functionality of an individual library are suffixed by _CLA (for command-line application).

For the c++ code, we follow the Google style guide (https://google.github.io/styleguide/cppguide.html) with minor adjustments.
- Source files use the .cpp extension. Template implementations may be separated from their declaration in a file with the same name and the .inc extension.
- The order of #include directives is not strictly alphabetical: within each block, all files in a specific directory precede the files in sub-directories.
- Whenever a code block is spread over multiple lines, the opening character (e.g. brace) will be placed on the next line and the opening and closing characters have the same indentation.
  ? This greatly increases ease of matching opening and closing characters.
- As a rule of thumb, closing braces are followed by a comment repeating the statement that opened them (e.g. "} // namespace geoviz").
  ? This makes program flow more obvious.
- Whenever possible, define complex and composite types early (whether using typedef or using). This especially applies to templated types.
  ? There are several reasons for this decision. For templated types, this may improve compiler error message readability (depending on the compiler). Giving a type an explicit (new) name or a shorter name, may increase readibility. Note that one type may be given several aliases to indicate the different roles in which that type is used. Defining types early makes changing those types easy (if required by some later changes to the code). Note that when the type used in one role may change, the other roles may keep the old type.
- Use the "auto" keyword extremely sparsely.
  ? While the "auto" keyword may increase coding speed, it comes at the cost of readibility: it is not immediately apparant what the type actually is. While decent IDE's will provide the underlying type when hovering over the variable, this depends on the IDE maintaining a correct lookup table and in many cases this is very unstable/error-prone. Even when the type is given on the same line as the variable is declared, this will often no longer be the case when the variable is used. Generally, an "auto" type can be replaced by an explicit typedef or using statement that gives an informative name to the type.
- When defining a method with default parameters that was declared elsewhere, repeat the default value as a comment.
  ? This will provide a good reminder of the value, as the declaration is often invisible (e.g. when using "go to declaration" to view the workings of the method).
- Prefer single line comments over short multi-line comments.
  ? This support easily disabling code blocks temporarily, e.g. while debugging.
- When deciding on a name, prefer to start with more general parts before more specific parts (e.g. my_point_x as opposed to my_x_point).
- Add inline spaces if this would make similar parts of multiple lines line up better. For example, when setting multiple values in sequence, you may add spaces before the = character to line them up.
- When declaring pointer variables, always place the * adjacent to the type (e.g. const char* my_variable).

