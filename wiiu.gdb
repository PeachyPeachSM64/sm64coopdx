set confirm off
set pagination off
set endian big
set architecture powerpc:common

set breakpoint pending on

set remotetimeout 20
set tcp auto-retry on

file /home/excellentgamer/render96dx/build/us_wiiu/render96dx.elf

target remote 127.0.0.1:1337

# WUT crt0 contains a trap instruction (twu) at 0x2000038 that can abort the title.
# Skip it when encountered.
break *0x2000038
commands
  silent
  set $pc = $pc + 4
  continue
end

break main
b sys_fatal
b exit
b abort

# catch very early init before main
b __init_wut
b __init_wut_newlib

# catch common fatal paths before remote disconnect
b OSFatal
b __assert_func

echo \nAttached and breakpoints set. Run: continue\n
