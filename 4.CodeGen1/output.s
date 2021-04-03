.text
_start_main:
sd ra, 0(sp)
sd fp, -8(sp)
add fp, sp, -8
add sp, sp, -16
la ra, _frameSize_main
lw ra, 0(ra)
sub, sp, sp, ra
sd t0, 8(sp)
sd t1, 16(sp)
sd t2, 24(sp)
sd t3, 32(sp)
sd t4, 40(sp)
sd t5, 48(sp)
sd t6, 56(sp)
sd s2, 64(sp)
sd s3, 72(sp)
sd s4, 80(sp)
sd s5, 88(sp)
sd s6, 96(sp)
sd s7, 104(sp)
sd s8, 112(sp)
sd s9, 120(sp)
sd s10, 128(sp)
sd s11, 136(sp)
sd fp, 144(sp)
fsw ft0, 152(sp)
fsw ft1, 156(sp)
fsw ft2, 160(sp)
fsw ft3, 164(sp)
fsw ft4, 168(sp)
fsw ft5, 172(sp)
fsw ft6, 176(sp)
fsw ft7, 180(sp)
li t0, 5
sw t0, 184(sp)
li ft0, 0x412e6666
fsw.s ft0, 188(sp)
.data
_CONSTANT_0: .word 0x9999999a
.align 3
.text
flw ft0, _CONSTANT_0
fmv.s fa0, ft0
jal _write_float
.data
_CONSTANT_1: .word 3
.align 3
.text
lw t0, _CONSTANT_1
mv a0, t0
jal _write_int
.data
_CONSTANT_2: .ascii "test\000"
.align 3
.text
la t0, _CONSTANT_2
mv a0, t0
jal _write_str
_end_main:
ld t0, 8(sp)
ld t1, 16(sp)
ld t2, 24(sp)
ld t3, 32(sp)
ld t4, 40(sp)
ld t5, 48(sp)
ld t6, 56(sp)
ld s2, 64(sp)
ld s3, 72(sp)
ld s4, 80(sp)
ld s5, 88(sp)
ld s6, 96(sp)
ld s7, 104(sp)
ld s8, 112(sp)
ld s9, 120(sp)
ld s10, 128(sp)
ld s11, 136(sp)
ld fp, 144(sp)
flw ft0, 152(sp)
flw ft1, 156(sp)
flw ft2, 160(sp)
flw ft3, 164(sp)
flw ft4, 168(sp)
flw ft5, 172(sp)
flw ft6, 176(sp)
flw ft7, 180(sp)
ld ra, 8(fp)
mv sp, fp
add sp, sp, 8
ld fp, 0(fp)
jr ra
.data_frameSize_main: .word 192
