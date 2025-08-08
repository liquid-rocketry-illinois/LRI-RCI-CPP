cd $3

which gh > /dev/null
if [ $? -ne 0 ]; then
  echo "Github CLI is required for this auto-release."
  exit 1
fi

read -p "Are all changes pushed? (Y/N): " confirm && [[ $confirm == [yY] || $confirm == [yY][eE][sS] ]] || exit 1
git add . &> /dev/null && git diff --quiet && git diff --cached --quiet
if [ $? -ne 0 ]; then
  read -p "You have uncommitted changes. Still continue? (Y/N): " confirm && [[ $confirm == [yY] || $confirm == [yY][eE][sS] ]] || exit 1
fi

cd $1

mkdir -p releaseNotes
tagname=$(cat $3/VERSION)
echo "Using version $tagname"
echo $tagname > releaseNotes/tagname

portablename=RCI-$tagname-win32-x64-portable.zip

rm -f env.zip
/c/Windows/System32/tar.exe acf $portablename $2.exe targets

msiname=RCI-$tagname-win32-x64.msi

wix build -src $3/wix/RCI_installer.xml -o $1/$msiname -d VERSION="$tagname" -b $1

echo "Complete Release Notes"
echo "# Release Notes" > releaseNotes/notes
notepad releaseNotes/notes

git tag $tagname -s -m "$tagname"
git push --tags

gh release create $tagname $portablename $2.exe $msiname --notes-file releaseNotes/notes