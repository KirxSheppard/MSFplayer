#!/bin/sh

echo "Creating DMG for MSF Player 1.2.0"

volname="MSF Player-macOS-1.2.0"
dmgname="$volname.dmg"
bundlename="MSF Player.app"

if [ ! -d "$bundlename" ]; then
        echo "Couldn’t find MSF Player Bundle"
        exit 2
fi

rm -f "$dmgname"
rm -rf "$volname"

mkdir "$volname" || exit 2
cp -R "$bundlename" "$volname" || exit 3
ln -s /Applications "$volname" || exit 4

hdiutil create -volname "$volname" -srcfolder "$volname" "$dmgname"
exitcode=$?

rm -r "$volname"
exit $exitcode
