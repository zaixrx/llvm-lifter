; ModuleID = 'x86_mov_example'
source_filename = "x86_mov_example"
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
  %esp = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 7
  %esi = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 4
  %rax = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 0
  %rbx = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 1
  %edx = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 3
  %gs = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 17
  %0 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 18
  %eflags_1 = getelementptr inbounds nuw %Flags, ptr %0, i32 0, i32 1
  %1 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 18
  %eflags_3 = getelementptr inbounds nuw %Flags, ptr %1, i32 0, i32 3
  %2 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 18
  %eflags_4 = getelementptr inbounds nuw %Flags, ptr %2, i32 0, i32 4
  %3 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 18
  %eflags_0 = getelementptr inbounds nuw %Flags, ptr %3, i32 0, i32 0
  %4 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 18
  %eflags_5 = getelementptr inbounds nuw %Flags, ptr %4, i32 0, i32 5
  %5 = getelementptr inbounds nuw %State, ptr %state, i32 0, i32 18
  %eflags_2 = getelementptr inbounds nuw %Flags, ptr %5, i32 0, i32 2
  br label %insn_0x0

insn_0x0:                                         ; preds = %initialize
  store i64 131072, ptr %esp, align 8
  br label %insn_0x5

insn_0x5:                                         ; preds = %insn_0x0
  store i64 65536, ptr %esi, align 8
  br label %insn_0xa

insn_0xa:                                         ; preds = %insn_0x5
  store i64 -8613303245920329199, ptr %rax, align 8
  br label %insn_0x14

insn_0x14:                                        ; preds = %insn_0xa
  %6 = load i64, ptr %rax, align 8
  %7 = and i64 %6, -65536
  %8 = or i64 %7, 4660
  store i64 %8, ptr %rax, align 8
  br label %insn_0x18

insn_0x18:                                        ; preds = %insn_0x14
  %9 = load i64, ptr %rax, align 8
  %10 = and i64 %9, -65281
  %11 = or i64 %10, 32768
  store i64 %11, ptr %rax, align 8
  br label %insn_0x1a

insn_0x1a:                                        ; preds = %insn_0x18
  store i64 2147483648, ptr %rax, align 8
  br label %insn_0x1f

insn_0x1f:                                        ; preds = %insn_0x1a
  %12 = load i64, ptr %rax, align 8
  store i64 %12, ptr %rbx, align 8
  br label %insn_0x22

insn_0x22:                                        ; preds = %insn_0x1f
  %13 = load i64, ptr %esi, align 8
  %14 = getelementptr i8, ptr %memory, i64 %13
  store i8 1, ptr %14, align 1
  br label %insn_0x25

insn_0x25:                                        ; preds = %insn_0x22
  %15 = load i64, ptr %esi, align 8
  %16 = getelementptr i8, ptr %memory, i64 %15
  %17 = load i8, ptr %16, align 1
  %18 = zext i8 %17 to i64
  %19 = load i64, ptr %rax, align 8
  %20 = and i64 %19, -256
  %21 = shl i64 %18, 0
  %22 = or i64 %20, %21
  store i64 %22, ptr %rax, align 8
  br label %insn_0x27

insn_0x27:                                        ; preds = %insn_0x25
  %23 = load i64, ptr %esi, align 8
  %24 = add i64 %23, 2
  %25 = getelementptr i8, ptr %memory, i64 %24
  store i16 3, ptr %25, align 1
  br label %insn_0x2d

insn_0x2d:                                        ; preds = %insn_0x27
  %26 = load i64, ptr %esi, align 8
  %27 = add i64 %26, 2
  %28 = getelementptr i8, ptr %memory, i64 %27
  %29 = load i16, ptr %28, align 1
  %30 = zext i16 %29 to i64
  %31 = load i64, ptr %rax, align 8
  %32 = and i64 %31, -65536
  %33 = shl i64 %30, 0
  %34 = or i64 %32, %33
  store i64 %34, ptr %rax, align 8
  br label %insn_0x31

insn_0x31:                                        ; preds = %insn_0x2d
  %35 = load i64, ptr %esi, align 8
  %36 = add i64 %35, 4
  %37 = getelementptr i8, ptr %memory, i64 %36
  store i32 -1, ptr %37, align 1
  br label %insn_0x38

insn_0x38:                                        ; preds = %insn_0x31
  %38 = load i64, ptr %esi, align 8
  %39 = add i64 %38, 4
  %40 = getelementptr i8, ptr %memory, i64 %39
  %41 = load i32, ptr %40, align 1
  %42 = zext i32 %41 to i64
  store i64 %42, ptr %rax, align 8
  br label %insn_0x3b

insn_0x3b:                                        ; preds = %insn_0x38
  store i64 2, ptr %edx, align 8
  br label %insn_0x40

insn_0x40:                                        ; preds = %insn_0x3b
  %43 = load i64, ptr %rax, align 8
  %44 = load i64, ptr %edx, align 8
  %45 = mul i64 %44, 8
  %46 = load i64, ptr %esi, align 8
  %47 = add i64 %45, %46
  %48 = add i64 %47, -8
  %49 = getelementptr i8, ptr %memory, i64 %48
  store i64 %43, ptr %49, align 1
  br label %insn_0x45

insn_0x45:                                        ; preds = %insn_0x40
  %50 = load i64, ptr %edx, align 8
  %51 = mul i64 %50, 8
  %52 = load i64, ptr %esi, align 8
  %53 = add i64 %51, %52
  %54 = add i64 %53, -8
  %55 = getelementptr i8, ptr %memory, i64 %54
  %56 = load i64, ptr %55, align 1
  store i64 %56, ptr %rbx, align 8
  br label %insn_0x4a

insn_0x4a:                                        ; preds = %insn_0x45
  %57 = load i64, ptr %rax, align 8
  %58 = getelementptr i8, ptr %memory, i64 151
  store i64 %57, ptr %58, align 1
  br label %insn_0x51

insn_0x51:                                        ; preds = %insn_0x4a
  %59 = getelementptr i8, ptr %memory, i64 151
  %60 = load i64, ptr %59, align 1
  store i64 %60, ptr %rbx, align 8
  br label %insn_0x58

insn_0x58:                                        ; preds = %insn_0x51
  %61 = load i64, ptr %edx, align 8
  %62 = trunc i64 %61 to i32
  %63 = mul i32 %62, 4
  %64 = load i64, ptr %esi, align 8
  %65 = trunc i64 %64 to i32
  %66 = add i32 %63, %65
  %67 = zext i32 %66 to i64
  %68 = load i64, ptr %gs, align 8
  %69 = add i64 %67, %68
  %70 = getelementptr i8, ptr %memory, i64 %69
  %71 = load i8, ptr %70, align 1
  %72 = zext i8 %71 to i64
  %73 = load i64, ptr %rax, align 8
  %74 = and i64 %73, -256
  %75 = shl i64 %72, 0
  %76 = or i64 %74, %75
  store i64 %76, ptr %rax, align 8
  br label %insn_0x5d

insn_0x5d:                                        ; preds = %insn_0x58
  %77 = getelementptr i8, ptr %memory, i64 0
  %78 = load i8, ptr %77, align 1
  %79 = zext i8 %78 to i64
  %80 = load i64, ptr %rax, align 8
  %81 = and i64 %80, -256
  %82 = shl i64 %79, 0
  %83 = or i64 %81, %82
  store i64 %83, ptr %rax, align 8
  br label %insn_0x64

insn_0x64:                                        ; preds = %insn_0x5d
  %84 = load i64, ptr %rax, align 8
  %85 = trunc i64 %84 to i8
  %86 = and i8 %85, -128
  %87 = zext i8 %86 to i64
  %88 = load i64, ptr %rax, align 8
  %89 = and i64 %88, -256
  %90 = shl i64 %87, 0
  %91 = or i64 %89, %90
  store i64 %91, ptr %rax, align 8
  %92 = lshr i8 %86, 4
  %93 = xor i8 %86, %92
  %94 = lshr i8 %93, 2
  %95 = xor i8 %93, %94
  %96 = lshr i8 %95, 1
  %97 = xor i8 %95, %96
  %98 = and i8 %97, 1
  %99 = icmp eq i8 %98, 0
  %100 = zext i1 %99 to i8
  store i8 %100, ptr %eflags_1, align 1
  %101 = icmp eq i8 %86, 0
  %102 = zext i1 %101 to i8
  store i8 %102, ptr %eflags_3, align 1
  %103 = lshr i8 %86, 7
  %104 = icmp eq i8 %103, 1
  %105 = zext i1 %104 to i8
  store i8 %105, ptr %eflags_4, align 1
  store i8 0, ptr %eflags_0, align 1
  store i8 0, ptr %eflags_5, align 1
  %106 = call i1 @__lifter_undef_af(i64 100)
  %107 = zext i1 %106 to i8
  store i8 %107, ptr %eflags_2, align 1
  br label %insn_0x66

insn_0x66:                                        ; preds = %insn_0x64
  %108 = load i64, ptr %rax, align 8
  %109 = trunc i64 %108 to i16
  %110 = or i16 %109, 3
  %111 = zext i16 %110 to i64
  %112 = load i64, ptr %rax, align 8
  %113 = and i64 %112, -65536
  %114 = shl i64 %111, 0
  %115 = or i64 %113, %114
  store i64 %115, ptr %rax, align 8
  %116 = trunc i16 %110 to i8
  %117 = lshr i8 %116, 4
  %118 = xor i8 %116, %117
  %119 = lshr i8 %118, 2
  %120 = xor i8 %118, %119
  %121 = lshr i8 %120, 1
  %122 = xor i8 %120, %121
  %123 = and i8 %122, 1
  %124 = icmp eq i8 %123, 0
  %125 = zext i1 %124 to i8
  store i8 %125, ptr %eflags_1, align 1
  %126 = icmp eq i16 %110, 0
  %127 = zext i1 %126 to i8
  store i8 %127, ptr %eflags_3, align 1
  %128 = lshr i16 %110, 15
  %129 = icmp eq i16 %128, 1
  %130 = zext i1 %129 to i8
  store i8 %130, ptr %eflags_4, align 1
  store i8 0, ptr %eflags_0, align 1
  store i8 0, ptr %eflags_5, align 1
  %131 = call i1 @__lifter_undef_af(i64 102)
  %132 = zext i1 %131 to i8
  store i8 %132, ptr %eflags_2, align 1
  br label %insn_0x6a

insn_0x6a:                                        ; preds = %insn_0x66
  %133 = load i64, ptr %rax, align 8
  %134 = trunc i64 %133 to i32
  %135 = load i64, ptr %rax, align 8
  %136 = trunc i64 %135 to i32
  %137 = xor i32 %134, %136
  %138 = zext i32 %137 to i64
  store i64 %138, ptr %rax, align 8
  %139 = trunc i32 %137 to i8
  %140 = lshr i8 %139, 4
  %141 = xor i8 %139, %140
  %142 = lshr i8 %141, 2
  %143 = xor i8 %141, %142
  %144 = lshr i8 %143, 1
  %145 = xor i8 %143, %144
  %146 = and i8 %145, 1
  %147 = icmp eq i8 %146, 0
  %148 = zext i1 %147 to i8
  store i8 %148, ptr %eflags_1, align 1
  %149 = icmp eq i32 %137, 0
  %150 = zext i1 %149 to i8
  store i8 %150, ptr %eflags_3, align 1
  %151 = lshr i32 %137, 31
  %152 = icmp eq i32 %151, 1
  %153 = zext i1 %152 to i8
  store i8 %153, ptr %eflags_4, align 1
  store i8 0, ptr %eflags_0, align 1
  store i8 0, ptr %eflags_5, align 1
  %154 = call i1 @__lifter_undef_af(i64 106)
  %155 = zext i1 %154 to i8
  store i8 %155, ptr %eflags_2, align 1
  br label %insn_0x6c

insn_0x6c:                                        ; preds = %insn_0x6a
  %156 = load i64, ptr %rbx, align 8
  %157 = and i64 %156, -1
  store i64 %157, ptr %rbx, align 8
  %158 = trunc i64 %157 to i8
  %159 = lshr i8 %158, 4
  %160 = xor i8 %158, %159
  %161 = lshr i8 %160, 2
  %162 = xor i8 %160, %161
  %163 = lshr i8 %162, 1
  %164 = xor i8 %162, %163
  %165 = and i8 %164, 1
  %166 = icmp eq i8 %165, 0
  %167 = zext i1 %166 to i8
  store i8 %167, ptr %eflags_1, align 1
  %168 = icmp eq i64 %157, 0
  %169 = zext i1 %168 to i8
  store i8 %169, ptr %eflags_3, align 1
  %170 = lshr i64 %157, 63
  %171 = icmp eq i64 %170, 1
  %172 = zext i1 %171 to i8
  store i8 %172, ptr %eflags_4, align 1
  store i8 0, ptr %eflags_0, align 1
  store i8 0, ptr %eflags_5, align 1
  %173 = call i1 @__lifter_undef_af(i64 108)
  %174 = zext i1 %173 to i8
  store i8 %174, ptr %eflags_2, align 1
  br label %insn_0x70

insn_0x70:                                        ; preds = %insn_0x6c
  %175 = load i64, ptr %esi, align 8
  %176 = add i64 %175, 4
  %177 = getelementptr i8, ptr %memory, i64 %176
  %178 = load i32, ptr %177, align 1
  %179 = load i64, ptr %rax, align 8
  %180 = trunc i64 %179 to i32
  %181 = or i32 %178, %180
  %182 = load i64, ptr %esi, align 8
  %183 = add i64 %182, 4
  %184 = getelementptr i8, ptr %memory, i64 %183
  store i32 %181, ptr %184, align 1
  %185 = trunc i32 %181 to i8
  %186 = lshr i8 %185, 4
  %187 = xor i8 %185, %186
  %188 = lshr i8 %187, 2
  %189 = xor i8 %187, %188
  %190 = lshr i8 %189, 1
  %191 = xor i8 %189, %190
  %192 = and i8 %191, 1
  %193 = icmp eq i8 %192, 0
  %194 = zext i1 %193 to i8
  store i8 %194, ptr %eflags_1, align 1
  %195 = icmp eq i32 %181, 0
  %196 = zext i1 %195 to i8
  store i8 %196, ptr %eflags_3, align 1
  %197 = lshr i32 %181, 31
  %198 = icmp eq i32 %197, 1
  %199 = zext i1 %198 to i8
  store i8 %199, ptr %eflags_4, align 1
  store i8 0, ptr %eflags_0, align 1
  store i8 0, ptr %eflags_5, align 1
  %200 = call i1 @__lifter_undef_af(i64 112)
  %201 = zext i1 %200 to i8
  store i8 %201, ptr %eflags_2, align 1
  br label %insn_0x73

insn_0x73:                                        ; preds = %insn_0x70
  %202 = load i64, ptr %rbx, align 8
  %203 = load i64, ptr %esi, align 8
  %204 = add i64 %203, 8
  %205 = getelementptr i8, ptr %memory, i64 %204
  %206 = load i64, ptr %205, align 1
  %207 = xor i64 %202, %206
  store i64 %207, ptr %rbx, align 8
  %208 = trunc i64 %207 to i8
  %209 = lshr i8 %208, 4
  %210 = xor i8 %208, %209
  %211 = lshr i8 %210, 2
  %212 = xor i8 %210, %211
  %213 = lshr i8 %212, 1
  %214 = xor i8 %212, %213
  %215 = and i8 %214, 1
  %216 = icmp eq i8 %215, 0
  %217 = zext i1 %216 to i8
  store i8 %217, ptr %eflags_1, align 1
  %218 = icmp eq i64 %207, 0
  %219 = zext i1 %218 to i8
  store i8 %219, ptr %eflags_3, align 1
  %220 = lshr i64 %207, 63
  %221 = icmp eq i64 %220, 1
  %222 = zext i1 %221 to i8
  store i8 %222, ptr %eflags_4, align 1
  store i8 0, ptr %eflags_0, align 1
  store i8 0, ptr %eflags_5, align 1
  %223 = call i1 @__lifter_undef_af(i64 115)
  %224 = zext i1 %223 to i8
  store i8 %224, ptr %eflags_2, align 1
  br label %insn_0x77

insn_0x77:                                        ; preds = %insn_0x73
  %225 = load i64, ptr %esp, align 8
  %226 = sub i64 %225, 8
  store i64 %226, ptr %esp, align 8
  %227 = getelementptr i8, ptr %memory, i64 %226
  store i64 124, ptr %227, align 1
  call void @__lifter_call_handler(i64 144)
  br label %insn_0x7c

insn_0x7c:                                        ; preds = %insn_0x77
  %228 = load i64, ptr %rbx, align 8
  %229 = load i64, ptr %esp, align 8
  %230 = sub i64 %229, 8
  store i64 %230, ptr %esp, align 8
  %231 = getelementptr i8, ptr %memory, i64 %230
  store i64 126, ptr %231, align 1
  call void @__lifter_call_handler(i64 %228)
  br label %insn_0x7e

insn_0x7e:                                        ; preds = %insn_0x7c
  %232 = load i64, ptr %esi, align 8
  %233 = add i64 %232, 8
  %234 = getelementptr i8, ptr %memory, i64 %233
  %235 = load i64, ptr %234, align 1
  %236 = load i64, ptr %esp, align 8
  %237 = sub i64 %236, 8
  store i64 %237, ptr %esp, align 8
  %238 = getelementptr i8, ptr %memory, i64 %237
  store i64 129, ptr %238, align 1
  call void @__lifter_call_handler(i64 %235)
  br label %insn_0x81

insn_0x81:                                        ; preds = %insn_0x7e
  %239 = load i64, ptr %rax, align 8
  %240 = trunc i64 %239 to i32
  %241 = load i64, ptr %rax, align 8
  %242 = trunc i64 %241 to i32
  %243 = xor i32 %240, %242
  %244 = zext i32 %243 to i64
  store i64 %244, ptr %rax, align 8
  %245 = trunc i32 %243 to i8
  %246 = lshr i8 %245, 4
  %247 = xor i8 %245, %246
  %248 = lshr i8 %247, 2
  %249 = xor i8 %247, %248
  %250 = lshr i8 %249, 1
  %251 = xor i8 %249, %250
  %252 = and i8 %251, 1
  %253 = icmp eq i8 %252, 0
  %254 = zext i1 %253 to i8
  store i8 %254, ptr %eflags_1, align 1
  %255 = icmp eq i32 %243, 0
  %256 = zext i1 %255 to i8
  store i8 %256, ptr %eflags_3, align 1
  %257 = lshr i32 %243, 31
  %258 = icmp eq i32 %257, 1
  %259 = zext i1 %258 to i8
  store i8 %259, ptr %eflags_4, align 1
  store i8 0, ptr %eflags_0, align 1
  store i8 0, ptr %eflags_5, align 1
  %260 = call i1 @__lifter_undef_af(i64 129)
  %261 = zext i1 %260 to i8
  store i8 %261, ptr %eflags_2, align 1
  br label %insn_0x83

insn_0x83:                                        ; preds = %insn_0x81
  %262 = load i8, ptr %eflags_3, align 1
  %263 = icmp ne i8 %262, 0
  br i1 %263, label %insn_0x90, label %insn_0x85

insn_0x90:                                        ; preds = %insn_0x83
  %264 = load i64, ptr %esp, align 8
  %265 = getelementptr i8, ptr %memory, i64 %264
  %266 = load i64, ptr %265, align 1
  %267 = add i64 %264, 8
  store i64 %267, ptr %esp, align 8
  call void @__lifter_ret_handler(i64 %266)
  ret void

insn_0x85:                                        ; preds = %insn_0x83
  store i64 1, ptr %rax, align 8
  br label %insn_0x8a

insn_0x8a:                                        ; preds = %insn_0x85
  %268 = load i64, ptr %rax, align 8
  %269 = trunc i64 %268 to i32
  %270 = load i64, ptr %rax, align 8
  %271 = trunc i64 %270 to i32
  %272 = or i32 %269, %271
  %273 = zext i32 %272 to i64
  store i64 %273, ptr %rax, align 8
  %274 = trunc i32 %272 to i8
  %275 = lshr i8 %274, 4
  %276 = xor i8 %274, %275
  %277 = lshr i8 %276, 2
  %278 = xor i8 %276, %277
  %279 = lshr i8 %278, 1
  %280 = xor i8 %278, %279
  %281 = and i8 %280, 1
  %282 = icmp eq i8 %281, 0
  %283 = zext i1 %282 to i8
  store i8 %283, ptr %eflags_1, align 1
  %284 = icmp eq i32 %272, 0
  %285 = zext i1 %284 to i8
  store i8 %285, ptr %eflags_3, align 1
  %286 = lshr i32 %272, 31
  %287 = icmp eq i32 %286, 1
  %288 = zext i1 %287 to i8
  store i8 %288, ptr %eflags_4, align 1
  store i8 0, ptr %eflags_0, align 1
  store i8 0, ptr %eflags_5, align 1
  %289 = call i1 @__lifter_undef_af(i64 138)
  %290 = zext i1 %289 to i8
  store i8 %290, ptr %eflags_2, align 1
  br label %insn_0x8c

insn_0x8c:                                        ; preds = %insn_0x8a
  %291 = load i8, ptr %eflags_3, align 1
  %292 = icmp ne i8 %291, 0
  br i1 %292, label %insn_0x91, label %insn_0x8e

insn_0x91:                                        ; preds = %insn_0x8c
  %293 = load i64, ptr %esp, align 8
  %294 = getelementptr i8, ptr %memory, i64 %293
  %295 = load i64, ptr %294, align 1
  %296 = add i64 %293, 8
  store i64 %296, ptr %esp, align 8
  %297 = load i64, ptr %esp, align 8
  %298 = add i64 %297, 16
  store i64 %298, ptr %esp, align 8
  call void @__lifter_ret_handler(i64 %295)
  ret void

insn_0x8e:                                        ; preds = %insn_0x8c
  br label %insn_0x94

insn_0x94:                                        ; preds = %insn_0x8e
  %299 = load i64, ptr %esi, align 8
  %300 = add i64 %299, 8
  %301 = getelementptr i8, ptr %memory, i64 %300
  %302 = load i64, ptr %301, align 1
  call void @__lifter_jmp_handler(i64 %302)
  ret void
}
