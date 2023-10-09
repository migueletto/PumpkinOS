PATH=./bin:$PATH
export LD_LIBRARY_PATH=./bin

add_gpio() {
  if [ ! -L /sys/class/gpio/gpio$1 ]; then
    echo $1 > /sys/class/gpio/export
  fi
}

del_gpio() {
  if [ -L /sys/class/gpio/gpio$1 ]; then
    echo $1 > /sys/class/gpio/unexport
  fi
}

add_gpio 24
add_gpio 25
echo -e '\033[?17;0;0c' > /dev/tty1

#./pumpkin -d 1 -f pumpkin.log -s libscriptlua.so ./script/pumpkin_linux.lua
./pumpkin -d 1 -s libscriptlua.so ./script/pumpkin_linux.lua

del_gpio 24
del_gpio 25
