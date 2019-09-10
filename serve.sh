#!/bin/bash
port=8000
if [[ $# -ne 0 ]]; then port=$1
fi
php "-S" "localhost:$port" "-t" "wwwroot/"
