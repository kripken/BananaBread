#!/bin/sh

rsync -avc --files-from=manifest.txt cube2/ output/
