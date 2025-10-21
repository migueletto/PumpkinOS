#!/bin/sh

# 2025-10-20 12:57:03.230595 I 06745 spacewar logtrap: 0x0006EB6C: trap 0xA055    DmNewRecord(ptr_0000E3C0, uint{0}, 2036) ...
# 2025-10-20 12:57:03.230672 I 06745 spacewar logtrap: 0x0006EB70: trap 0xA055    DmNewRecord(ptr_0000E3C0, uint{0}, 2036): ptr_0000EB00

pos=$1

if [ -z "$pos" ]; then
  pos=6
fi

awk -v pos=$pos '
BEGIN {
  next_ptr = 1;
  next_localid = 1;
}

$pos == "logtrap:" && $(pos+1) ~ /^0x/ {
  s = $(pos + 4);
  for (i = pos + 5; i <= NF; i++) {
    s = s " " $i;
  }

  for (;;) {
    i = index(s, "ptr_");
    if (i == 0) break;
    addr = substr(s, i + 4, 8);
    id = ptrs[addr];
    if (id == "") {
      id = sprintf("%06d", next_ptr);
      ptrs[addr] = id;
      next_ptr++;
    }
    s = substr(s, 1, i - 1) "ptr" id substr(s, i + 12);
  }

  for (;;) {
    i = index(s, "localid_");
    if (i == 0) break;
    addr = substr(s, i + 8, 8);
    if (addr == "00000000") {
      id = "000000";
    } else {
      id = localids[addr];
      if (id == "") {
        id = sprintf("%06d", next_localid);
        localids[addr] = id;
        next_localid++;
      }
    }
    s = substr(s, 1, i - 1) "localid" id substr(s, i + 16);
  }

  print s;
  next;
}
'

exit 0
