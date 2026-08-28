#!/bin/sh

# 2026-08-28 10:29:17.430890 T 01357 ChemTabl logmem: alloc heap Window 12864 40
# 2026-08-28 12:03:14.571628 T 02433 TealPain logmem: realloc heap HandlePtr 63680 63680 40
# 2026-08-28 10:29:17.595650 T 01357 ChemTabl logmem: free heap Window 12864
# 2026-08-28 10:29:17.477988 T 01357 ChemTabl logmem: write 486754 1
# 2026-08-28 13:40:11.864217 T 06964 PalmSoko logmem: lock heap Bitmap 395108 228
# 2026-08-28 13:40:11.864239 T 06964 PalmSoko logmem: unlock heap Bitmap 395108 228

if [ -z "$1" ]; then
  echo "usage: $0 <label>"
  exit 0
fi

gawk -vlabel=$1 '
BEGIN {
  idx = 0;
  struct["Control"] = 1;
  struct["Form"] = 1;
  struct["Gadget"] = 1;
  struct["Label"] = 1;
  struct["List"] = 1;
  struct["Menu"] = 1;
  struct["MenuItem"] = 1;
  struct["Popup"] = 1;
  struct["ScrollBar"] = 1;
  struct["Title"] = 1;
  struct["Window"] = 1;
}
$5 == label && $6 == "logmem:" && ($7 == "alloc" || $7 == "lock") {
  addr = 0 + $10;
  len = 0 + $11;
  type = $9;
  table_idx[idx] = addr;
  table_line[idx] = NR;
  table_addr[addr] = 1;
  table_size[addr] = len;
  table_type[addr] = type;
  for (i = 0; i < len; i++) {
    table_mem[addr + i] = idx;
  }
  idx++;
  next;
}
$5 == label && $6 == "logmem:" && $7 == "realloc" {
  addr = 0 + $10;
  if (table_addr[addr]) {
    len = table_size[addr];
    for (i = 0; i < len; i++) {
      table_mem[addr + i] = "";
    }
    table_addr[addr] = "";
    table_size[addr] = "";
    table_type[addr] = "";

    addr = 0 + $11;
    len = 0 + $12;
    type = $9;
    table_idx[idx] = addr;
    table_line[idx] = NR;
    table_addr[addr] = 1;
    table_size[addr] = len;
    table_type[addr] = type;
    for (i = 0; i < len; i++) {
      table_mem[addr + i] = idx;
    }
    idx++;
  }
  next;
}
$5 == label && $6 == "logmem:" && ($7 == "free" || $7 == "unlock") {
  addr = 0 + $10;
  if (table_addr[addr]) {
    len = table_size[addr];
    for (i = 0; i < len; i++) {
      table_mem[addr + i] = "";
    }
    table_addr[addr] = "";
    table_size[addr] = "";
    table_type[addr] = "";
  } else {
    print NR ": free unused addr " addr;
  }
  next;
}
$5 == label && $6 == "logmem:" && $7 == "write" {
  mem_addr = 0 + $8;
  i = table_mem[mem_addr];
  if (i) {
    addr = table_idx[i];
    if (addr) {
      type = table_type[addr];
      offset = mem_addr - addr;
      if (struct[type]) {
        print NR ": write to " type " structure at " addr " addr " mem_addr " offset " offset " (" table_line[i] ")";
      }
    }
  }
  next;
}
'

exit 0
