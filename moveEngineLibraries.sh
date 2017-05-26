#!/bin/bash

if [ -f "Debug Library/Engine_debug.lib" ]; then
    cp -f Debug\ Library/Engine_debug.lib ScriptingProject/
    echo "Debug Engine Library copied!"
fi

if [ -f "Release Library/Engine_release.lib" ]; then
    cp -f Release\ Library/Engine_release.lib ScriptingProject/
    echo "Release Engine Library copied!"
fi
