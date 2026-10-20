  %121 = load i64, ptr %120, align 1; ModuleID = 'blog_example.o_lifted'
source_filename = "blog_example.o_lifted"
target datalayout = "e-p:64:64-i64:64-i128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%State = type { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, %Flags }
%Flags = type { i8, i8, i8, i8, i8, i8 }

declare void @__lifter_jmp_handler(i64)

declare void @__lifter_call_handler(i64)

declare void @__lifter_ret_handler(i64)

declare i1 @__lifter_undef_af(i64)

define void @lifted_0x0(ptr noalias %state, ptr noalias %memory) {
initialize:
  %rax = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 0
  %0 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 18
  %eflags_0 = getelementptr inbounds nuw %Flags, ptr %0, i32 0, i32 0
  %1 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 18
  %eflags_5 = getelementptr inbounds nuw %Flags, ptr %1, i32 0, i32 5
  %2 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 18
  %eflags_1 = getelementptr inbounds nuw %Flags, ptr %2, i32 0, i32 1
  %3 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 18
  %eflags_3 = getelementptr inbounds nuw %Flags, ptr %3, i32 0, i32 3
  %4 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 18
  %eflags_4 = getelementptr inbounds nuw %Flags, ptr %4, i32 0, i32 4
  %5 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 18
  %eflags_2 = getelementptr inbounds nuw %Flags, ptr %5, i32 0, i32 2
  %rsp = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 7
  br label %insn_0x0

insn_0x0:                                         ; preds = %initialize
  %6 = load i64, ptr %rax, align 8
  %7 = sub i64 %6, 0
  %8 = icmp ule i64 %6, 0
  %9 = zext i1 %8 to i8
  store i8 %9, ptr %eflags_0, align 1
  %10 = xor i64 %6, 0
  %11 = xor i64 %6, %7
  %12 = and i64 %10, %11
  %13 = and i64 %12, -2147483648
  %14 = icmp eq i64 %13, 1
  %15 = zext i1 %14 to i8
  store i8 %15, ptr %eflags_5, align 1
  %16 = trunc i64 %7 to i8
  %17 = lshr i8 %16, 4
  %18 = xor i8 %16, %17
  %19 = lshr i8 %18, 2
  %20 = xor i8 %18, %19
  %21 = lshr i8 %20, 1
  %22 = xor i8 %20, %21
  %23 = and i8 %22, 1
  %24 = icmp eq i8 %23, 0
  %25 = zext i1 %24 to i8
  store i8 %25, ptr %eflags_1, align 1
  %26 = icmp eq i64 %7, 0
  %27 = zext i1 %26 to i8
  store i8 %27, ptr %eflags_3, align 1
  %28 = lshr i64 %7, 63
  %29 = icmp eq i64 %28, 1
  %30 = zext i1 %29 to i8
  store i8 %30, ptr %eflags_4, align 1
  %31 = call i1 @__lifter_undef_af(i64 0)
  %32 = zext i1 %31 to i8
  store i8 %32, ptr %eflags_2, align 1
  br label %insn_0x4

insn_0x4:                                         ; preds = %insn_0x0
  %33 = load i8, ptr %eflags_3, align 1
  %34 = icmp ne i8 %33, 0
  br i1 %34, label %insn_0xc, label %insn_0x6

insn_0xc:                                         ; preds = %insn_0x4
  %35 = load i64, ptr %rax, align 8
  %36 = add i64 %35, 2
  store i64 %36, ptr %rax, align 8
  %37 = icmp ule i64 %36, 2
  %38 = zext i1 %37 to i8
  store i8 %38, ptr %eflags_0, align 1
  %39 = xor i64 %35, %36
  %40 = xor i64 2, %36
  %41 = and i64 %39, %40
  %42 = and i64 %41, -2147483648
  %43 = icmp eq i64 %42, 1
  %44 = zext i1 %43 to i8
  store i8 %44, ptr %eflags_5, align 1
  %45 = trunc i64 %36 to i8
  %46 = lshr i8 %45, 4
  %47 = xor i8 %45, %46
  %48 = lshr i8 %47, 2
  %49 = xor i8 %47, %48
  %50 = lshr i8 %49, 1
  %51 = xor i8 %49, %50
  %52 = and i8 %51, 1
  %53 = icmp eq i8 %52, 0
  %54 = zext i1 %53 to i8
  store i8 %54, ptr %eflags_1, align 1
  %55 = icmp eq i64 %36, 0
  %56 = zext i1 %55 to i8
  store i8 %56, ptr %eflags_3, align 1
  %57 = lshr i64 %36, 63
  %58 = icmp eq i64 %57, 1
  %59 = zext i1 %58 to i8
  store i8 %59, ptr %eflags_4, align 1
  %60 = call i1 @__lifter_undef_af(i64 12)
  %61 = zext i1 %60 to i8
  store i8 %61, ptr %eflags_2, align 1
  br label %insn_0x10

insn_0x6:                                         ; preds = %insn_0x4
  %62 = load i64, ptr %rax, align 8
  %63 = add i64 %62, 1
  store i64 %63, ptr %rax, align 8
  %64 = icmp ule i64 %63, 1
  %65 = zext i1 %64 to i8
  store i8 %65, ptr %eflags_0, align 1
  %66 = xor i64 %62, %63
  %67 = xor i64 1, %63
  %68 = and i64 %66, %67
  %69 = and i64 %68, -2147483648
  %70 = icmp eq i64 %69, 1
  %71 = zext i1 %70 to i8
  store i8 %71, ptr %eflags_5, align 1
  %72 = trunc i64 %63 to i8
  %73 = lshr i8 %72, 4
  %74 = xor i8 %72, %73
  %75 = lshr i8 %74, 2
  %76 = xor i8 %74, %75
  %77 = lshr i8 %76, 1
  %78 = xor i8 %76, %77
  %79 = and i8 %78, 1
  %80 = icmp eq i8 %79, 0
  %81 = zext i1 %80 to i8
  store i8 %81, ptr %eflags_1, align 1
  %82 = icmp eq i64 %63, 0
  %83 = zext i1 %82 to i8
  store i8 %83, ptr %eflags_3, align 1
  %84 = lshr i64 %63, 63
  %85 = icmp eq i64 %84, 1
  %86 = zext i1 %85 to i8
  store i8 %86, ptr %eflags_4, align 1
  %87 = call i1 @__lifter_undef_af(i64 6)
  %88 = zext i1 %87 to i8
  store i8 %88, ptr %eflags_2, align 1
  br label %insn_0xa

insn_0xa:                                         ; preds = %insn_0x6
  br label %insn_0x10

insn_0x10:                                        ; preds = %insn_0x14, %insn_0xa, %insn_0xc
  %89 = load i64, ptr %rax, align 8
  %90 = add i64 %89, 1
  store i64 %90, ptr %rax, align 8
  %91 = icmp ule i64 %89, 1
  %92 = zext i1 %91 to i8
  store i8 %92, ptr %eflags_0, align 1
  %93 = xor i64 %89, 1
  %94 = xor i64 %89, %90
  %95 = and i64 %93, %94
  %96 = and i64 %95, -2147483648
  %97 = icmp eq i64 %96, 1
  %98 = zext i1 %97 to i8
  store i8 %98, ptr %eflags_5, align 1
  %99 = trunc i64 %90 to i8
  %100 = lshr i8 %99, 4
  %101 = xor i8 %99, %100
  %102 = lshr i8 %101, 2
  %103 = xor i8 %101, %102
  %104 = lshr i8 %103, 1
  %105 = xor i8 %103, %104
  %106 = and i8 %105, 1
  %107 = icmp eq i8 %106, 0
  %108 = zext i1 %107 to i8
  store i8 %108, ptr %eflags_1, align 1
  %109 = icmp eq i64 %90, 0
  %110 = zext i1 %109 to i8
  store i8 %110, ptr %eflags_3, align 1
  %111 = lshr i64 %90, 63
  %112 = icmp eq i64 %111, 1
  %113 = zext i1 %112 to i8
  store i8 %113, ptr %eflags_4, align 1
  %114 = call i1 @__lifter_undef_af(i64 16)
  %115 = zext i1 %114 to i8
  store i8 %115, ptr %eflags_2, align 1
  br label %insn_0x14

insn_0x14:                                        ; preds = %insn_0x10
  %116 = load i8, ptr %eflags_3, align 1
  %117 = icmp ne i8 %116, 0
  %118 = xor i1 %117, true
  br i1 %118, label %insn_0x10, label %insn_0x16

insn_0x16:                                        ; preds = %insn_0x14
  %119 = load i64, ptr %rsp, align 8
  %120 = getelementptr i8, ptr %memory, i64 %119
  %121 = load i64, ptr %120, align 1
  %122 = add i64 %119, 8
  store i64 %122, ptr %rsp, align 8
  call void @__lifter_ret_handler(i64 %121)
  ret void
}
GEP register rax index: 0
GEP register rsp index: 7
non-constant branch 16 ->
