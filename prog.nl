

2021-04-02 19:32                                                          Page 1


00010000 T begtext	   00013494 T _context_dump   00016310 T cwrite
00010000 T _start	   0001358b T _context_dump_a 0001632b T swritech
0001006c T __isr_restore   0001362d T _active_dump    0001634c T swrites
00010cc3 T __cio_setscroll 0001391f T _que_init	      00016379 T swrite
00010d65 T __cio_moveto	   00013976 T _que_alloc      00016394 T str2int
00010dc5 T __cio_putchar_a 000139dc T _que_free	      00016406 T strlen
00010e3d T __cio_putchar   00013a39 T _que_length     00016428 T strcpy
00010f3b T __cio_puts_at   00013a88 T _que_enque      00016452 T strcat
00010f7d T __cio_puts	   00013c1d T _que_deque      00016488 T strcmp
00010fae T __cio_write	   00013cd3 T _que_peek	      000164be T pad
00010fe6 T __cio_clearscro 00013d35 T _que_dump	      000164e0 T padstr
0001105f T __cio_clearscre 00013e3f T _sched_init     00016574 T sprint
000110cf T __cio_scroll	   00013ee4 T _schedule	      000167a5 T cvt_dec0
00011590 T __cio_printf_at 00013ff7 T _dispatch	      0001682e T cvt_dec
000115b0 T __cio_printf	   0001441a T _sio_init	      00016875 T cvt_hex
0001172d T __cio_getchar   000145ab T _sio_enable     00016904 T cvt_oct
000117c2 T __cio_gets	   00014622 T _sio_disable    0001698e T report
00011813 T __cio_input_que 00014699 T _sio_inq_length 000169f2 T exit
0001183e T __cio_init	   000146a3 T _sio_readc      000169fa T read
00011ae4 T __panic	   00014704 T _sio_reads      00016a02 T write
00011b00 T __init_interrup 0001479f T _sio_writec     00016a0a T getpid
00011b13 T __install_isr   0001480f T _sio_write      00016a12 T getppid
00011b38 T __delay	   000148b6 T _sio_puts	      00016a1a T gettime
00011c98 T _clk_init	   000148f3 T _sio_dump	      00016a22 T getprio
00011d8b T _init	   00014a70 T _stk_init	      00016a2a T setprio
00011f1c T _shell	   00014b00 T _stk_alloc      00016a32 T kill
000121e5 T __put_char_or_c 00014b4b T _stk_free	      00016a3a T sleep
0001221e T __bound	   00014b6d T _stk_dump	      00016a42 T spawn
00012242 T __memset	   000155b5 T _sys_init	      00016a4a T wait
00012266 T __memclr	   0001566b T _force_exit     00016a52 T bogus
00012287 T __memcpy	   0001578d T __pci_read16    00016a5a T exit_helper
000122b4 T __strlen	   00015837 T __pci_read32    0001a000 D __isr_stub_tabl
000122d6 T __strcmp	   000158bf T __pci_read8     0001a50c D __hexdigits
0001230c T __strcpy	   00015969 T __pci_write32   0001a51d B __bss_start
0001233c T __strcat	   000159f6 T __pci_find_devi 0001a51d D _edata
00012372 T __pad	   00015af5 T __eth_init      0001b6b4 B scroll_max_x
00012394 T __padstr	   00015cfa T __eth_disable_i 0001b6b8 B max_x
00012428 T __sprint	   00015d19 T __eth_enable_in 0001b6bc B curr_x
00012659 T __cvtdec0	   00015d38 T __eth_load_CU_b 0001b6c0 B min_y
000126e2 T __cvtdec	   00015df1 T __eth_load_RU_b 0001b6c4 B scroll_max_y
00012729 T __cvthex	   00015eb1 T __eth_CU_start  0001b6c8 B scroll_min_y
000127a8 T __cvtoct	   00015f25 T __eth_nop	      0001b6cc B min_x
00012836 T _kpanic	   00015ff4 T __pci_test      0001b6d0 B curr_y
00012972 T _km_init	   000160b0 T __inb	      0001b6d4 B max_y
00012b7a T _km_dump	   000160bb T __inw	      0001b6d8 B scroll_min_x
00012be1 T _km_page_alloc  000160c7 T __inl	      0001b6e0 B __isr_table
00012cd9 T _km_page_free   000160d2 T __outb	      0001bae0 B _sleeping
00012e8c T _km_slice_alloc 000160de T __outw	      0001bae4 B _system_time
00012f50 T _km_slice_free  000160eb T __outl	      0001bb00 B b512
0001306d T _pcb_alloc	   000160f7 T __get_flags     0001bd00 B _system_stack
000130bb T _pcb_free	   000160fa T __pause	      0001bd20 B b256
000130e3 T _pcb_find_pid   00016101 T __get_ra	      0001be20 B _system_esp
00013138 T _pcb_cleanup	   00016105 T init	      0001be40 B _next_pid
000131a5 T _proc_init	   0001622e T idle	      0001be44 B _active_procs
00013219 T _proc_create	   000162c2 T cwritech	      0001be60 B _ptable
00013370 T _pcb_dump	   000162e3 T cwrites	      0001bec4 B _ready







2021-04-02 19:32                                                          Page 2


0001bed4 B _current	   0001bee0 B eth	      0001c700 B eth_pci
0001bed8 B _reading	   0001bf00 B CBL	      0001c704 B _end



























































