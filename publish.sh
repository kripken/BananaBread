#!/bin/sh

rsync -avz --exclude packages --exclude src cube2/* output
