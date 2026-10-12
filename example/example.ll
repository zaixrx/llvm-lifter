; ModuleID = 'example.o_lifted'
source_filename = "example.o_lifted"
target datalayout = "e-p:64:64-i64:64-i128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

declare void @__lifter_jmp_handler(i64)

declare void @__lifter_call_handler(i64)

declare void @__lifter_ret_handler(i64)

declare i1 @__lifter_undef_af(i64)

define void @lifted_0x0(ptr noalias %state, ptr noalias %memory) {
insn_0x5:
  %esp = getelementptr inbounds nuw i8, ptr %state, i64 56
  store i64 131072, ptr %esp, align 8
  ret void
}
