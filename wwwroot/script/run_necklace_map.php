<?php
  /*
  Copyright 2019 Netherlands eScience Center and TU Eindhoven
  Licensed under the Apache License, version 2.0. See LICENSE for details.
  */

  $json = file_get_contents('php://input');
  $data = json_decode($json);

  // Check the input.
  if (!ctype_alnum($data->value) || !is_numeric($data->buffer_rad) || !is_numeric($data->aversion_ratio))
    throw new RuntimeException('Corrupt input.');

  if ($data->interval != "centroid" && $data->interval != "wedge")
    throw new RuntimeException('Corrupt input.');

  if ($data->order != "fixed" && $data->order != "any")
    throw new RuntimeException('Corrupt input.');

  // Make sure that the temporary directory exists.
  $tmp = sys_get_temp_dir();
  $tmpdir = "$tmp/geoviz/";
  if (!file_exists($tmpdir))
    mkdir($tmpdir);

  // Store the geometry and data files in the temporary directory.
  $geometry_tmp = tempnam($tmpdir, "geom");
  $data_tmp = tempnam($tmpdir, "data");

  $handle = fopen($geometry_tmp, "w");
  fwrite($handle, base64_decode($data->geometry_base64));
  fclose($handle);

  $handle = fopen($data_tmp, "w");
  fwrite($handle, base64_decode($data->data_base64));
  fclose($handle);

  $exec = "../bin/necklace_map_cla";
  $args =
    " --in_geometry_filename $geometry_tmp".
    " --in_data_filename $data_tmp".
    " --in_value_name $data->value".
    " --interval_type=$data->interval".
    ($data->ignore_point_regions ? " --ignore_point_regions" : "").
    " --order_type=$data->order".
    " --buffer_rad=$data->buffer_rad".
    " --aversion_ratio=$data->aversion_ratio".
    " --bead_opacity=0.5".
    " --draw_feasible_intervals".
    " --out_website".
    " --log_dir $tmpdir".
    " --minloglevel=2";
  $command = escapeshellcmd("$exec $args");
  //echo "<!--Execute: $command-->";
  $result = shell_exec($command);
  echo $result;

  unlink($geometry_tmp);
  unlink($data_tmp);
?>
