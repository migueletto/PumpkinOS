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

export LANG=en_US.UTF-8

add_gpio 536  # 512 + 24
add_gpio 537  # 512 + 25
echo -e '\033[?17;0;0c' > /dev/tty1

gdb --args ./pumpkin -d 1 -f pumpkin.log -s libscriptlua ./script/pumpkin_rpi.lua

del_gpio 536
del_gpio 537
