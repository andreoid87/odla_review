#!/bin/bash
export PATH=".:${PATH}:${HOME}/Qt/5.15*/clang_64/bin"

# Copy in installer folder
cp -r ./build.qtc/ODLA.app ./installer-mac/packages/it.kemoniariver.odla/data
# Launch deploy script for Mac
macdeployqt ./installer-mac/packages/it.kemoniariver.odla/data/ODLA.app