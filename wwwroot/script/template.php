<!--
    Copyright 2019 Netherlands eScience Center and TU Eindhoven
    Licensed under the Apache License, version 2.0. See LICENSE for details.
-->
<!--Note that while we could have one PHP file that takes as input the executable to run, this would pose an enormous security risk.-->
<?php
    // Collect the command line arguments and separate them by spaces.
    $args = $_REQUEST["args"];
    $delim = isset($_REQUEST["delim"]) ? $_REQUEST["delim"] : "|";
    $arg_str = str_replace($delim, " ", $args);
    // Run the executable and return what it sent to the standard output stream.
    $result = shell_exec("../bin/executable $arg_str");
    echo $result;
?>