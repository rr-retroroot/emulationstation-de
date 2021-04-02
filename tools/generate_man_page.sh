#!/usr/bin/bash
#
# generate_man_page.sh
# Generate a Unix manual page for EmulationStation Desktop Edition.
#
# The script takes no arguments and replaces the man page file es-app/assets/emulationstation.6.gz
# It has to be run from within the tools directory.
#
# The command help2man must be installed, or the script will fail.
#
# This script is intended to only be used on Linux systems.
#
# Leon Styhre
# 2020-07-16
#

if [ ! -f ../es-app/CMakeLists.txt ]; then
  echo "You need to run this script from within the tools directory."
  exit
fi

ESBINARY=../emulationstation

TEMPFILE_INPUT=tempfile_input_$(date +%H%M%S)
TEMPFILE_OUTPUT=tempfile_output_$(date +%H%M%S)
TARGET_FILENAME=emulationstation.6

MAN_INCLUDE="
[NAME]
emulationstation - EmulationStation Desktop Edition

[DESCRIPTION]
EmulationStation Desktop Edition is a front-end for browsing and launching games from your multi-platform game collection.

It's intended to be used in conjunction with emulators such as the RetroArch cores.

[AUTHOR]
Leon Styhre (Desktop Edition fork) <https://es-de.org/>

RetroPie community (RetroPie fork)

Alec \"Aloshi\" Lofquist (original version)

[SEE ALSO]
Full documentation is available at: <https://gitlab.com/leonstyhre/emulationstation-de/>
"

echo "${MAN_INCLUDE}" > $TEMPFILE_INPUT

help2man --section 6 --no-info --include $TEMPFILE_INPUT $ESBINARY > $TEMPFILE_OUTPUT

# Manual string replacements, these may need to be modified if changes are made to the
# command line --help output.
cat $TEMPFILE_OUTPUT | sed s/"EmulationStation Desktop Edition, Emulator Front\\\-end"/""/g | \
sed s/"Set to at least"/".br\nSet to at least"/ > $TARGET_FILENAME

gzip -9 $TARGET_FILENAME
mv ${TARGET_FILENAME}.gz ../es-app/assets/

echo "The man page was generated and saved to ../es-app/assets/emulationstation.6.gz"

rm $TEMPFILE_INPUT
rm $TEMPFILE_OUTPUT
