#!/bin/sh

if [ $# -lt 2 ]; then
  echo "usage: $0 <OS> <BITS> [ <TARGET> ]"
  echo " <OS> must be Msys, GNU/Linux, Darwin, or Serenity"
  echo " <BITS> must be 64 or 32"
  echo " <TARGET> must be empty or clean"
  exit 0
fi

OSNAME=$1
BITS=$2
TARGET=$3

DIR=`pwd`
ROOT=$DIR/..
PATH=$ROOT/bin:$PATH
export LD_LIBRARY_PATH=$ROOT/bin

if [ $OSNAME = "Msys" ]; then
  SDL2=
  GUI=windows
elif [ $OSNAME = "GNU/Linux" ]; then
  SDL2=liblsdl2
  GUI=linux
elif [ $OSNAME = "Serenity" ]; then
  if [ -z "$SERENITY" ]; then
    echo "Environment variable SERENITY must point to SerenityOS home directory"
    exit 1
  fi
  SDL2=liblsdl2
  GUI=linux
elif [ $OSNAME = "Darwin" ]; then
  SDL2=liblsdl2
  GUI=linux
else
  echo "Invalid OS parameter"
  exit 1
fi

for dir in bin lib tools vfs/app_card/PALM/Programs vfs/app_install vfs/app_storage registry
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
    make ROOT=$ROOT BITS=$BITS $TARGET
    cd $DIR
  fi
done

for dir in libpit lua $SDL2 libpumpkin libos libshell $GUI BOOT Launcher Preferences Command Edit LuaSyntax MemoPad AddressBook ToDoList DateBook
do
  if [ -d $dir ]; then
    cd $dir
    make ROOT=$ROOT OSNAME=$OSNAME BITS=$BITS SERENITY=$SERENITY $TARGET
    cd $DIR
  fi
done

exit 0
