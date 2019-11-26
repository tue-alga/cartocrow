# GeoViz code library

This library is a minimal (dummy) C++ library meant to demonstrate functionality that can be called from a website a    nd return new content for that website.
Note that this implementation depends on quick processing times. Heavier processing may take longer than the allowed     response delay, in which case more complex processing control may be required.

This library has been developed according to the following design principles to promote usability, sustainability, a    nd platform independence:

- Separation of functional code and user interface code.
- Project configuration using CMake.
- Separate libraries and applications per method.

## Code styles

Note that within the CMakeLists files, we follow a loose naming convention:
- All variables are UPPER_CASE, all methods are lower_case.
- All project-specific CMake build options have the GEOVIZ_ prefix.
- Similar to CMake, most variables are [project_name]_<target_name>_<type> (e.g. GEOVIZ_INSTALL_SOURCE_DIR or NECKLACE_MAP_TARGET)._
- Applications exposing the functionality of an individual library are suffixed by _CLA (for command-line application).

For the c++ code, we follow the Google style guide (https://google.github.io/styleguide/cppguide.html) with minor adjustments.
- Source files use the .cpp extension. Template implementations may be separated from their declaration in a file with the same name and the .inc extension.
- The order of #include directives is not strictly alphabetical: within each block, all files in a specific directory precede the files in sub-directories.
- Whenever a code block is spread over multiple lines, the opening character (e.g. brace) will be placed on the next line and the opening and closing characters have the same indentation.
  Why? This greatly increases ease of matching opening and closing characters.
- As a rule of thumb, closing braces are followed by a comment repeating the statement that opened them (e.g. "} // namespace geoviz").
  Why? This makes program flow more obvious.
- Whenever possible, define complex and composite types early (whether using typedef or using). This especially applies to templated types.
  Why? There are several reasons for this decision. For templated types, this may improve compiler error message readability (depending on the compiler). Giving a type an explicit (new) name or a shorter name, may increase readibility. Note that one type may be given several aliases to indicate the different roles in which that type is used. Defining types early makes changing those types easy (if required by some later changes to the code). Note that when the type used in one role may change, the other roles may keep the old type.
- Use the "auto" keyword extremely sparsely.
  Why? While the "auto" keyword may increase coding speed, it comes at the cost of readibility: it is not immediately apparant what the type actually is. While decent IDE's will provide the underlying type when hovering over the variable, this depends on the IDE maintaining a correct lookup table and in many cases this is very unstable/error-prone. Even when the type is given on the same line as the variable is declared, this will often no longer be the case when the variable is used. Generally, an "auto" type can be replaced by an explicit typedef or using statement that gives an informative name to the type.
- When defining a method with default parameters that was declared elsewhere, repeat the default value as a comment.
  Why? This will provide a good reminder of the value, as the declaration is often invisible (e.g. when using "go to declaration" to view the workings of the method).
- Prefer single line comments over short multi-line comments.
  Why? This support easily disabling code blocks temporarily, e.g. while debugging.
- When deciding on a name, prefer to start with more general parts before more specific parts (e.g. my_point_x as opposed to my_x_point).
- Add inline spaces if this would make similar parts of multiple lines line up better. For example, when setting multiple values in sequence, you may add spaces before the = character to line them up.
- When declaring pointer variables, always place the * adjacent to the type (e.g. const char* my_variable).
- Whenever spreading function parameters/arguments over multiple lines, always put the first one on a new line. Prefer to use exactly 1 parameter/argument per line.
- When naming elements in camel case, only capitalize the first letter of an abbreviation.

## Logging

We use the Google logging library (glog) for logging important information about the process including major command-line parameters and high level program flow. This library allows for logging at various 'levels' and adjusting the visible logging levels at runtime. We adhere to the following guidelines on logging levels:
* **FATAL:** terminate the process because an 'invalid state' has been reached. Further processing would produce invalid or undeterminable results. For example, some inconsistency in the data was detected. Note that logging at this severity level may terminate the program very abruptly without any deconstruction or memory deallocation, which can give serious issues related to file or stream/pipe flushing.
* **ERROR:** a recoverable 'invalid state' has been reached. For example, a file I/O operation failed or a connection was lost. Use FATAL instead if no (more) retries are attempted.
* **WARNING:** should never be used. Some people argue for using this severity level for unexpected results (but you expect them, because you're logging them: use INFO instead) or something that a developer should really have a look at (add a TODO comment instead and possibly log to INFO if you want to irritate the user).
* **INFO:** major program flow or progress. This is meant for the uninformed end user. For example, "currently at 60% progress", or "accessing file X". Try not to be spammy while also not leaving the user to wonder whether the program is still running.
* **1** minor program flow such as module progress. This is meant for the informed end user. For example, "computing average point density", or "95/1600 processes complete".
* **2+** program details. This is meant for a developer, with varying degrees of required detail. Try not to use too many levels: the developer will likely either want all the debug data (verbosity = infinity) or only know where things break down (verbosity = ~2).

Note that FLAGS_logtostderr logs to the standard error stream instead of file, FLAGS_stderrthreshold logs to standard error stream as well as file. You can set program-specific log directories using FLAGS_log_dir.

Use CHECK (and related method) liberally to verify important data or states. Take the logging message into account when choosing the CHECK method. For example, CHECK(my_string.empty()) will just log that line, while CHECK_EQ(my_string,"") will log that line and the two compared values (the contents of my_string and ""). Also be careful about comparing double values: you may want to replace equality checks by checks on a maximum difference or at least CHECK_DOUBLE_EQ().

## Documentation

Where possible, the DoxyGen documentation is placed in the source files. A structural exception is made for (non-DoxyGen) comments on how to interpret certain parameters. Note that the code should be mostly self-documenting in its naming choices and we assume that a library developer has the DoxyGen-generated documentation at hand and therefore does not need to see the documentation source.

This choice is motived by a few arguments:
* This keeps the headers small and to be used for a 'birds eye view' of the functionality.
* The documentation is close to the thing it describes, i.e. the functional code.
* A small change to the documentation does not require a full rebuild.

Where appropriate, the documentation should specify pre- and post-conditions and it should specify processing efficiency, e.g. O(n) notation.

Note that autobrief is disabled, because it can give unexpected results in rare cases where the documentation is in both header and source files.