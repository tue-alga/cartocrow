<?php
  /*
  Copyright 2019 Netherlands eScience Center and TU Eindhoven
  Licensed under the Apache License, version 2.0. See LICENSE for details.
  */
    $args = isset($_REQUEST["args"]) ? $_REQUEST["args"] : "";
    $delim = isset($_REQUEST["delim"]) ? $_REQUEST["delim"] : "|";
    $assign = isset($_REQUEST["assign"]) ? $_REQUEST["assign"] : ":";
    $arg_str = str_replace($delim, " ", str_replace($assign, "=", $args));
    //TODO(tvl) replace shell_exec(...) by the variant that requires a local absolute path to the executable and an array of arguments.
    $result = shell_exec(escapeshellcmd("../bin/draw_logo_cla $arg_str"));
    echo $result;
?>
