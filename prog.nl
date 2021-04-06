

2021-04-06 15:41                                                          Page 1


00010000 T begtext	   000134a4 T _context_dump   00016400 T cwrites
00010000 T _start	   0001359b T _context_dump_a 0001642d T cwrite
0001006c T __isr_restore   0001363d T _active_dump    00016448 T swritech
00010cc3 T __cio_setscroll 0001392f T _que_init	      00016469 T swrites
00010d65 T __cio_moveto	   00013986 T _que_alloc      00016496 T swrite
00010dc5 T __cio_putchar_a 000139ec T _que_free	      000164b1 T str2int
00010e3d T __cio_putchar   00013a49 T _que_length     00016523 T strlen
00010f3b T __cio_puts_at   00013a98 T _que_enque      00016545 T strcpy
00010f7d T __cio_puts	   00013c2d T _que_deque      0001656f T strcat
00010fae T __cio_write	   00013ce3 T _que_peek	      000165a5 T strcmp
00010fe6 T __cio_clearscro 00013d45 T _que_dump	      000165db T pad
0001105f T __cio_clearscre 00013e4f T _sched_init     000165fd T padstr
000110cf T __cio_scroll	   00013ef4 T _schedule	      00016691 T sprint
00011590 T __cio_printf_at 00014007 T _dispatch	      000168c2 T cvt_dec0
000115b0 T __cio_printf	   0001442a T _sio_init	      0001694b T cvt_dec
0001172d T __cio_getchar   000145bb T _sio_enable     00016992 T cvt_hex
000117c2 T __cio_gets	   00014632 T _sio_disable    00016a21 T cvt_oct
00011813 T __cio_input_que 000146a9 T _sio_inq_length 00016aab T report
0001183e T __cio_init	   000146b3 T _sio_readc      00016b0f T exit
00011ae4 T __panic	   00014714 T _sio_reads      00016b17 T read
00011b00 T __init_interrup 000147af T _sio_writec     00016b1f T write
00011b13 T __install_isr   0001481f T _sio_write      00016b27 T getpid
00011b38 T __delay	   000148c6 T _sio_puts	      00016b2f T getppid
00011c98 T _clk_init	   00014903 T _sio_dump	      00016b37 T gettime
00011d8b T _init	   00014a80 T _stk_init	      00016b3f T getprio
00011f2c T _shell	   00014b10 T _stk_alloc      00016b47 T setprio
000121f5 T __put_char_or_c 00014b5b T _stk_free	      00016b4f T kill
0001222e T __bound	   00014b7d T _stk_dump	      00016b57 T sleep
00012252 T __memset	   000155c5 T _sys_init	      00016b5f T spawn
00012276 T __memclr	   0001567b T _force_exit     00016b67 T wait
00012297 T __memcpy	   0001579d T __pci_read16    00016b6f T bogus
000122c4 T __strlen	   00015847 T __pci_read32    00016b77 T exit_helper
000122e6 T __strcmp	   000158cf T __pci_read8     0001a000 D __isr_stub_tabl
0001231c T __strcpy	   00015979 T __pci_write32   0001a50c D __hexdigits
0001234c T __strcat	   00015a06 T __pci_find_devi 0001a51d B __bss_start
00012382 T __pad	   00015add T __eth_loadaddr  0001a51d D _edata
000123a4 T __padstr	   00015b8e T __eth_init      0001b6b4 B scroll_max_x
00012438 T __sprint	   00015e01 T __eth_disable_i 0001b6b8 B max_x
00012669 T __cvtdec0	   00015e20 T __eth_enable_in 0001b6bc B curr_x
000126f2 T __cvtdec	   00015e3f T __eth_load_CU_b 0001b6c0 B min_y
00012739 T __cvthex	   00015eca T __eth_load_RU_b 0001b6c4 B scroll_max_y
000127b8 T __cvtoct	   00015f6a T __eth_CU_start  0001b6c8 B scroll_min_y
00012846 T _kpanic	   00015ffa T __eth_nop	      0001b6cc B min_x
00012982 T _km_init	   00016111 T __pci_test      0001b6d0 B curr_y
00012b8a T _km_dump	   000161cd T __inb	      0001b6d4 B max_y
00012bf1 T _km_page_alloc  000161d8 T __inw	      0001b6d8 B scroll_min_x
00012ce9 T _km_page_free   000161e4 T __inl	      0001b6e0 B __isr_table
00012e9c T _km_slice_alloc 000161ef T __outb	      0001bae0 B _sleeping
00012f60 T _km_slice_free  000161fb T __outw	      0001bae4 B _system_time
0001307d T _pcb_alloc	   00016208 T __outl	      0001bb00 B b512
000130cb T _pcb_free	   00016214 T __get_flags     0001bd00 B _system_stack
000130f3 T _pcb_find_pid   00016217 T __pause	      0001bd20 B b256
00013148 T _pcb_cleanup	   0001621e T __get_ra	      0001be20 B _system_esp
000131b5 T _proc_init	   00016222 T init	      0001be40 B _next_pid
00013229 T _proc_create	   0001634b T idle	      0001be44 B _active_procs
00013380 T _pcb_dump	   000163df T cwritech	      0001be60 B _ptable







2021-04-06 15:41                                                          Page 2


0001bec4 B _ready	   0001bee0 B CBL_end	      0001bf00 B CBL_start
0001bed4 B _current	   0001bee4 B eth	      0001c700 B eth_pci
0001bed8 B _reading	   0001beec B CBL	      0001c704 B _end


























































