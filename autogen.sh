#!/bin/sh

set -ex
touch ChangeLog NEWS README
autoreconf --install --force 
