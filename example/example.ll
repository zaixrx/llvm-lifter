i64 122; ModuleID = 'x86_mov_example'
source_filename = "x86_mov_example"
target datalayout = "e-p:64:64-i64:64-i128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%State = type { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, %Flags, [6 x %Segment], %X87, [32 x [64 x i8]], [8 x i64], i32, i32 }
%Flags = type { i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8 }
%Segment = type { i64, i32, i16, i16 }
%X87 = type { [8 x [10 x i8]], i16, i16, i16, i16, i64, i64 }

define void @lifted_0x0(ptr noalias %state, ptr noalias %memory) {
initialize:
  %al = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 0
  %r8 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 8
  %bl = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 1
  %r9 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 9
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
  store i64 -1, ptr %r8, align 8
  br label %insn_0x1c

insn_0x1c:                                        ; preds = %insn_0x15
  %6 = load i64, ptr %al, align 8
  %7 = trunc i64 %6 to i8
  %8 = zext i8 %7 to i64
  %9 = load i64, ptr %bl, align 8
  %10 = and i64 %9, -256
  %11 = shl i64 %8, 0
  %12 = or i64 %10, %11
  store i64 %12, ptr %bl, align 8
  br label %insn_0x1e

insn_0x1e:                                        ; preds = %insn_0x1c
  %13 = load i64, ptr %al, align 8
  %14 = trunc i64 %13 to i16
  %15 = zext i16 %14 to i64
  %16 = load i64, ptr %bl, align 8
  %17 = and i64 %16, -65536
  %18 = shl i64 %15, 0
  %19 = or i64 %17, %18
  store i64 %19, ptr %bl, align 8
  br label %insn_0x21

insn_0x21:                                        ; preds = %insn_0x1e
  %20 = load i64, ptr %al, align 8
  %21 = trunc i64 %20 to i32
  %22 = zext i32 %21 to i64
  store i64 %22, ptr %bl, align 8
  br label %insn_0x23

insn_0x23:                                        ; preds = %insn_0x21
  %23 = load i64, ptr %al, align 8
  store i64 %23, ptr %bl, align 8
  br label %insn_0x26

insn_0x26:                                        ; preds = %insn_0x23
  %24 = load i64, ptr %r8, align 8
  store i64 %24, ptr %r9, align 8
  br label %insn_0x29

insn_0x29:                                        ; preds = %insn_0x26
  %25 = getelementptr i8, ptr %memory, i64 43
  store i8 66, ptr %25, align 1
  br label %insn_0x2c

insn_0x2c:                                        ; preds = %insn_0x29
  %26 = getelementptr i8, ptr %memory, i64 45
  store i16 4660, ptr %26, align 2
  br label %insn_0x32

insn_0x32:                                        ; preds = %insn_0x2c
  %27 = getelementptr i8, ptr %memory, i64 47
  store i32 305419896, ptr %27, align 4
  br label %insn_0x39

insn_0x39:                                        ; preds = %insn_0x32
  %28 = getelementptr i8, ptr %memory, i64 51
  store i64 -1, ptr %28, align 8
  br label %insn_0x41

insn_0x41:                                        ; preds = %insn_0x39
  %29 = load i64, ptr %al, align 8
  %30 = trunc i64 %29 to i8
  %31 = getelementptr i8, ptr %memory, i64 59
  store i8 %30, ptr %31, align 1
  br label %insn_0x44

insn_0x44:                                        ; preds = %insn_0x41
  %32 = load i64, ptr %al, align 8
  %33 = trunc i64 %32 to i16
  %34 = getelementptr i8, ptr %memory, i64 61
  store i16 %33, ptr %34, align 2
  br label %insn_0x48

insn_0x48:                                        ; preds = %insn_0x44
  %35 = load i64, ptr %al, align 8
  %36 = trunc i64 %35 to i32
  %37 = getelementptr i8, ptr %memory, i64 63
  store i32 %36, ptr %37, align 4
  br label %insn_0x4b

insn_0x4b:                                        ; preds = %insn_0x48
  %38 = load i64, ptr %al, align 8
  %39 = getelementptr i8, ptr %memory, i64 67
  store i64 %38, ptr %39, align 8
  br label %insn_0x4f

insn_0x4f:                                        ; preds = %insn_0x4b
  %40 = getelementptr i8, ptr %memory, i64 59
  %41 = load i8, ptr %40, align 1
  %42 = zext i8 %41 to i64
  %43 = load i64, ptr %cl, align 8
  %44 = and i64 %43, -256
  %45 = shl i64 %42, 0
  %46 = or i64 %44, %45
  store i64 %46, ptr %cl, align 8
  br label %insn_0x52

insn_0x52:                                        ; preds = %insn_0x4f
  %47 = getelementptr i8, ptr %memory, i64 61
  %48 = load i16, ptr %47, align 2
  %49 = zext i16 %48 to i64
  %50 = load i64, ptr %cl, align 8
  %51 = and i64 %50, -65536
  %52 = shl i64 %49, 0
  %53 = or i64 %51, %52
  store i64 %53, ptr %cl, align 8
  br label %insn_0x56

insn_0x56:                                        ; preds = %insn_0x52
  %54 = getelementptr i8, ptr %memory, i64 63
  %55 = load i32, ptr %54, align 4
  %56 = zext i32 %55 to i64
  store i64 %56, ptr %cl, align 8
  br label %insn_0x59

insn_0x59:                                        ; preds = %insn_0x56
  %57 = getelementptr i8, ptr %memory, i64 67
  %58 = load i64, ptr %57, align 8
  store i64 %58, ptr %cl, align 8
  br label %insn_0x5d

insn_0x5d:                                        ; preds = %insn_0x59
  store i64 2, ptr %edx, align 8
  br label %insn_0x62

insn_0x62:                                        ; preds = %insn_0x5d
  %59 = load i64, ptr %al, align 8
  %60 = getelementptr i8, ptr %memory, i64 379
  store i64 %59, ptr %60, align 8
  br label %insn_0x67

insn_0x67:                                        ; preds = %insn_0x62
  %61 = getelementptr i8, ptr %memory, i64 379
  %62 = load i64, ptr %61, align 8
  store i64 %62, ptr %r10, align 8
  br label %insn_0x6c

insn_0x6c:                                        ; preds = %insn_0x67
  %63 = load i64, ptr %al, align 8
  %64 = getelementptr i8, ptr %memory, i64 41
  store i64 %63, ptr %64, align 8
  br label %insn_0x73

insn_0x73:                                        ; preds = %insn_0x6c
  %65 = getelementptr i8, ptr %memory, i64 41
  %66 = load i64, ptr %65, align 8
  store i64 %66, ptr %r11, align 8
  br label %insn_0x7a

insn_0x7a:                                        ; preds = %insn_0x73
  ret void
}
