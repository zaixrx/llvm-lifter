; ModuleID = 'blog_example.o_lifted'
source_filename = "blog_example.o_lifted"
target datalayout = "e-m:e-p:64:64-i64:64-i128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

declare void @__lifter_jmp_handler(i64)

declare void @__lifter_call_handler(i64)

declare void @__lifter_ret_handler(i64)

; Function Attrs: nounwind willreturn memory(none)
declare i1 @__lifter_undef_af(i64) #0

define i64 @lifted_0x0(i64 %rax) #1 {
initialize_state:
  ret i64 0
}

attributes #0 = { nounwind willreturn memory(none) }
attributes #1 = { "lifter.memory_removed" "lifter.state_removed" }
