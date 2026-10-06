i64 115; ModuleID = 'x86_mov_example'
source_filename = "x86_mov_example"
target datalayout = "e-p:64:64-i64:64-i128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%State = type { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, %Flags }
%Flags = type { i8, i8, i8, i8, i8, i8 }

declare i1 @__lifter_undef_af(i64)

define void @lifted_0x0(ptr noalias %state, ptr noalias %memory) {
initialize:
  %al = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 0
  %bl = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 1
  %r8 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 8
  %r9 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 9
  %rsi = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 4
  %cl = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 2
  %edx = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 3
  %r10 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 10
  %r11 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 11
  br label %insn_0x0

insn_0x0:                                         ; preds = %initialize
  %0 = load i64, ptr %al, align 8
  %1 = and i64 %0, -256
  %2 = or i64 %1, 66
  store i64 %2, ptr %al, align 8
  br label %insn_0x2

insn_0x2:                                         ; preds = %insn_0x0
  %3 = load i64, ptr %al, align 8
  %4 = and i64 %3, -65536
  %5 = or i64 %4, 4660
  store i64 %5, ptr %al, align 8
  br label %insn_0x6

insn_0x6:                                         ; preds = %insn_0x2
  store i64 305419896, ptr %al, align 8
  br label %insn_0xb

insn_0xb:                                         ; preds = %insn_0x6
  store i64 1234605616436508552, ptr %al, align 8
  br label %insn_0x15

insn_0x15:                                        ; preds = %insn_0xb
  %6 = load i64, ptr %al, align 8
  %7 = trunc i64 %6 to i8
  %8 = zext i8 %7 to i64
  %9 = load i64, ptr %bl, align 8
  %10 = and i64 %9, -256
  %11 = shl i64 %8, 0
  %12 = or i64 %10, %11
  store i64 %12, ptr %bl, align 8
  br label %insn_0x17

insn_0x17:                                        ; preds = %insn_0x15
  %13 = load i64, ptr %al, align 8
  %14 = trunc i64 %13 to i16
  %15 = zext i16 %14 to i64
  %16 = load i64, ptr %bl, align 8
  %17 = and i64 %16, -65536
  %18 = shl i64 %15, 0
  %19 = or i64 %17, %18
  store i64 %19, ptr %bl, align 8
  br label %insn_0x1a

insn_0x1a:                                        ; preds = %insn_0x17
  %20 = load i64, ptr %al, align 8
  %21 = trunc i64 %20 to i32
  %22 = zext i32 %21 to i64
  store i64 %22, ptr %bl, align 8
  br label %insn_0x1c

insn_0x1c:                                        ; preds = %insn_0x1a
  %23 = load i64, ptr %al, align 8
  store i64 %23, ptr %bl, align 8
  br label %insn_0x1f

insn_0x1f:                                        ; preds = %insn_0x1c
  %24 = load i64, ptr %r8, align 8
  store i64 %24, ptr %r9, align 8
  br label %insn_0x22

insn_0x22:                                        ; preds = %insn_0x1f
  %25 = load i64, ptr %rsi, align 8
  %26 = add i64 %25, 16
  %27 = getelementptr i8, ptr %memory, i64 %26
  %28 = load i8, ptr %27, align 1
  %29 = zext i8 %28 to i64
  %30 = load i64, ptr %cl, align 8
  %31 = and i64 %30, -256
  %32 = shl i64 %29, 0
  %33 = or i64 %31, %32
  store i64 %33, ptr %cl, align 8
  br label %insn_0x25

insn_0x25:                                        ; preds = %insn_0x22
  %34 = load i64, ptr %rsi, align 8
  %35 = add i64 %34, 18
  %36 = getelementptr i8, ptr %memory, i64 %35
  %37 = load i16, ptr %36, align 1
  %38 = zext i16 %37 to i64
  %39 = load i64, ptr %cl, align 8
  %40 = and i64 %39, -65536
  %41 = shl i64 %38, 0
  %42 = or i64 %40, %41
  store i64 %42, ptr %cl, align 8
  br label %insn_0x29

insn_0x29:                                        ; preds = %insn_0x25
  %43 = load i64, ptr %rsi, align 8
  %44 = add i64 %43, 20
  %45 = getelementptr i8, ptr %memory, i64 %44
  %46 = load i32, ptr %45, align 1
  %47 = zext i32 %46 to i64
  store i64 %47, ptr %cl, align 8
  br label %insn_0x2c

insn_0x2c:                                        ; preds = %insn_0x29
  %48 = load i64, ptr %rsi, align 8
  %49 = add i64 %48, 24
  %50 = getelementptr i8, ptr %memory, i64 %49
  %51 = load i64, ptr %50, align 1
  store i64 %51, ptr %cl, align 8
  br label %insn_0x30

insn_0x30:                                        ; preds = %insn_0x2c
  %52 = load i64, ptr %rsi, align 8
  %53 = getelementptr i8, ptr %memory, i64 %52
  store i8 66, ptr %53, align 1
  br label %insn_0x33

insn_0x33:                                        ; preds = %insn_0x30
  %54 = load i64, ptr %rsi, align 8
  %55 = add i64 %54, 2
  %56 = getelementptr i8, ptr %memory, i64 %55
  store i16 4660, ptr %56, align 1
  br label %insn_0x39

insn_0x39:                                        ; preds = %insn_0x33
  %57 = load i64, ptr %rsi, align 8
  %58 = add i64 %57, 4
  %59 = getelementptr i8, ptr %memory, i64 %58
  store i32 305419896, ptr %59, align 1
  br label %insn_0x40

insn_0x40:                                        ; preds = %insn_0x39
  %60 = load i64, ptr %rsi, align 8
  %61 = add i64 %60, 8
  %62 = getelementptr i8, ptr %memory, i64 %61
  store i64 -1, ptr %62, align 1
  br label %insn_0x48

insn_0x48:                                        ; preds = %insn_0x40
  %63 = load i64, ptr %al, align 8
  %64 = trunc i64 %63 to i8
  %65 = load i64, ptr %rsi, align 8
  %66 = add i64 %65, 16
  %67 = getelementptr i8, ptr %memory, i64 %66
  store i8 %64, ptr %67, align 1
  br label %insn_0x4b

insn_0x4b:                                        ; preds = %insn_0x48
  %68 = load i64, ptr %al, align 8
  %69 = trunc i64 %68 to i16
  %70 = load i64, ptr %rsi, align 8
  %71 = add i64 %70, 18
  %72 = getelementptr i8, ptr %memory, i64 %71
  store i16 %69, ptr %72, align 1
  br label %insn_0x4f

insn_0x4f:                                        ; preds = %insn_0x4b
  %73 = load i64, ptr %al, align 8
  %74 = trunc i64 %73 to i32
  %75 = load i64, ptr %rsi, align 8
  %76 = add i64 %75, 20
  %77 = getelementptr i8, ptr %memory, i64 %76
  store i32 %74, ptr %77, align 1
  br label %insn_0x52

insn_0x52:                                        ; preds = %insn_0x4f
  %78 = load i64, ptr %al, align 8
  %79 = load i64, ptr %rsi, align 8
  %80 = add i64 %79, 24
  %81 = getelementptr i8, ptr %memory, i64 %80
  store i64 %78, ptr %81, align 1
  br label %insn_0x56

insn_0x56:                                        ; preds = %insn_0x52
  store i64 2, ptr %edx, align 8
  br label %insn_0x5b

insn_0x5b:                                        ; preds = %insn_0x56
  %82 = load i64, ptr %al, align 8
  %83 = load i64, ptr %edx, align 8
  %84 = mul i64 %83, 8
  %85 = load i64, ptr %rsi, align 8
  %86 = add i64 %84, %85
  %87 = add i64 %86, 16
  %88 = getelementptr i8, ptr %memory, i64 %87
  store i64 %82, ptr %88, align 1
  br label %insn_0x60

insn_0x60:                                        ; preds = %insn_0x5b
  %89 = load i64, ptr %edx, align 8
  %90 = mul i64 %89, 8
  %91 = load i64, ptr %rsi, align 8
  %92 = add i64 %90, %91
  %93 = add i64 %92, 16
  %94 = getelementptr i8, ptr %memory, i64 %93
  %95 = load i64, ptr %94, align 1
  store i64 %95, ptr %r10, align 8
  br label %insn_0x65

insn_0x65:                                        ; preds = %insn_0x60
  %96 = load i64, ptr %al, align 8
  %97 = getelementptr i8, ptr %memory, i64 108
  store i64 %96, ptr %97, align 1
  br label %insn_0x6c

insn_0x6c:                                        ; preds = %insn_0x65
  %98 = getelementptr i8, ptr %memory, i64 115
  %99 = load i64, ptr %98, align 1
  store i64 %99, ptr %r11, align 8
  br label %insn_0x73

insn_0x73:                                        ; preds = %insn_0x6c
  ret void
}
out branch to instruction @73
