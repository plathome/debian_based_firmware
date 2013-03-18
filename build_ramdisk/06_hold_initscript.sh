#!/bin/bash

. `dirname $0`/config.sh

chroot ${DISTDIR} /usr/bin/aptitude hold initscripts
