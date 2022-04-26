# lemola_cc

inspired by https://www.sigbus.info/compilerbook

# How to compile your code
1. Write it to `src.c`
2. `make a.out`

## If you want to see debug print
`make -B src.s Debug=1` enables you to see debug print by C.

`make -B src.s RUSTD=1` eanbles you to see debug print by Rust.

# appendix
## x86_64 ABI

| Arch/ABI | arg1  | arg2  | arg3  | arg4  | arg5  | arg6  | arg7  | Notes |
| :------: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
|  x86-64  |  rdi  |  rsi  |  rdx  |  rcx  |  r8   |  r9   |  \-   |       |

Note: in syscall, instead of rcx, r10 is used


from man page<br>