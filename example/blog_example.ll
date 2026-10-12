; ModuleID = 'blog_example.o_lifted'
source_filename = "blog_example.o_lifted"
target datalayout = "e-p:64:64-i64:64-i128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

declare void @__lifter_jmp_handler(i64)

declare void @__lifter_call_handler(i64)

declare void @__lifter_ret_handler(i64)

declare i1 @__lifter_undef_af(i64)

define void @lifted_0x0(ptr noalias %state, ptr noalias %memory) {
insn_0x4:
  %0 = getelementptr inbounds nuw i8, ptr %state, i64 144
  %eflags_5 = getelementptr inbounds nuw i8, ptr %state, i64 149
  %eflags_1 = getelementptr inbounds nuw i8, ptr %state, i64 145
  %eflags_3 = getelementptr inbounds nuw i8, ptr %state, i64 147
  %eflags_4 = getelementptr inbounds nuw i8, ptr %state, i64 148
  %eflags_2 = getelementptr inbounds nuw i8, ptr %state, i64 146
  %1 = load i64, ptr %state, align 8
  %2 = icmp eq i64 %1, 0
  %3 = zext i1 %2 to i8
  store i8 %3, ptr %0, align 1
  store i8 0, ptr %eflags_5, align 1
  %4 = trunc i64 %1 to i8
  %5 = lshr i8 %4, 4
  %6 = xor i8 %5, %4
  %7 = lshr i8 %6, 2
  %8 = xor i8 %7, %6
  %9 = lshr i8 %8, 1
  %10 = xor i8 %9, %8
  %11 = and i8 %10, 1
  %12 = xor i8 %11, 1
  store i8 %12, ptr %eflags_1, align 1
  store i8 %3, ptr %eflags_3, align 1
  %.lobit = lshr i64 %1, 63
  %13 = trunc nuw nsw i64 %.lobit to i8
  store i8 %13, ptr %eflags_4, align 1
  %14 = tail call i1 @__lifter_undef_af(i64 0)
  %15 = zext i1 %14 to i8
  store i8 %15, ptr %eflags_2, align 1
  ret void
}
