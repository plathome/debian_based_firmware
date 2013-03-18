#!/bin/bash

. `dirname $0`/config.sh

chroot ${SQUEEZEDIR} /usr/bin/aptitude hold initscripts
