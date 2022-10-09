export RESOURCE=$PWD/app/src/main/assets/resource.bfs

# clean resource
rm $RESOURCE

echo RESOURCES
cd resource
bfs $RESOURCE blobSet readme.txt
bfs $RESOURCE blobSet BarlowSemiCondensed-Regular-64.json
cd ..

echo VKK UI
cd app/src/main/cpp/libvkk/ui/resource
./build-resource.sh $RESOURCE
cd ../../../../../../..

echo VKK VG
cd app/src/main/cpp/libvkk/vg/resource
./build-resource.sh $RESOURCE
cd ../../../../../../..

echo CONTENTS
bfs $RESOURCE blobList
