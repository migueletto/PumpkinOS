#!/bin/sh

DIR=`pwd`
ROOT=$DIR/..
PATH=$ROOT/bin:$PATH
export LD_LIBRARY_PATH=$ROOT/bin

OSNAME=`uname -o`
if [ $OSNAME = "GNU/Linux" ]; then
SDL2=liblsdl2
GUI=linux
else
SDL2=
GUI=windows
fi

for dir in bin lib vfs/app_card/PALM/Programs vfs/app_install vfs/app_storage registry
do
  if [ ! -d $ROOT/$dir ]; then
    echo "creating directory $dir"
    mkdir -p $ROOT/$dir
  fi
done

for dir in pilrc prcbuild
do
  if [ -d $dir ]; then
    cd $dir
    make ROOT=$ROOT BITS=$1 $2
    cd $DIR
  fi
done

for dir in libpit lua-5.3.5 $SDL2 libpumpkin libos $GUI BOOT Launcher Preferences Command libshell Edit LuaSyntax MemoPad AddressBook ToDoList DateBook
do
  if [ -d $dir ]; then
    cd $dir
    make ROOT=$ROOT OSNAME=serenity BITS=$1 $2
    #make ROOT=$ROOT BITS=$1 $2
    cd $DIR
  fi
done

exit 0
