cd $1
cd ..

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
rm -f env.zip
/c/Windows/System32/tar.exe acf env.zip $2.exe targets

mkdir -p releaseNotes
read -p "Enter tag name/release name: " tagname
echo tagname > releaseNotes/tagname

echo "Complete Release Notes"
echo "# Release Notes" > releaseNotes/notes
notepad releaseNotes/notes

git tag $tagname -s -m "$tagname"
git push --tags

gh release create $tagname env.zip $2.exe --notes-file releaseNotes/notes