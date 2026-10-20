"""Compile recovered IR and compare its results with lifted state execution."""

import ctypes
import pathlib
import re
import subprocess
import sys
import tempfile
import unittest


LIFTER, CLANG, NASM = sys.argv[1:4]
sys.argv[1:] = []
REGISTERS = "rax rbx rcx rdx rsi rdi rbp rsp r8 r9 r10 r11 r12 r13 r14 r15 rip gs".split()
FLAGS = "cf pf af zf sf of".split()


class State(ctypes.Structure):
    _fields_ = [(name, ctypes.c_uint64) for name in REGISTERS] + [
        (name, ctypes.c_uint8) for name in FLAGS
    ]


class OptimizerTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.directory = pathlib.Path(self.temporary.name)
        self.serial = 0

    def lift(self, assembly, *options):
        self.serial += 1
        stem = self.directory / str(self.serial)
        source = stem.with_suffix(".asm")
        binary = stem.with_suffix(".bin")
        source.write_text("bits 64\n" + assembly)
        subprocess.run([NASM, "-f", "bin", str(source), "-o", str(binary)], check=True)
        result = subprocess.run([LIFTER, str(binary), *options], check=True, capture_output=True, text=True)
        ir = result.stdout
        # Compiling the output also checks that diagnostics stay on stderr.
        output = stem.with_suffix(".ll")
        output.write_text(ir)
        helpers = self.directory / "helpers.c"
        helpers.write_text(
            "void __lifter_ret_handler(unsigned long x) {(void)x;}\n"
            "void __lifter_call_handler(unsigned long x) {(void)x;}\n"
            "void __lifter_jmp_handler(unsigned long x) {(void)x;}\n"
            "_Bool __lifter_undef_af(unsigned long x) {(void)x; return 0;}\n"
        )
        library = stem.with_suffix(".so")
        subprocess.run([CLANG, "-shared", "-fPIC", str(output), str(helpers), "-o", str(library)],
                       check=True, capture_output=True)
        loaded = ctypes.CDLL(str(library))
        function = loaded.lifted_0x0
        result_type, parameters = re.search(
            r"define [^\n]*?\b(void|i64) @lifted_0x0\(([^)]*)\)", ir
        ).groups()
        names, types = [], []
        for parameter in parameters.split(",") if parameters else []:
            names.append(re.search(r"%([\w.]+)", parameter).group(1))
            types.append(ctypes.c_void_p if parameter.strip().startswith("ptr") else
                         ctypes.c_uint8 if parameter.strip().startswith("i8 ") else ctypes.c_uint64)
        function.argtypes = types
        function.restype = ctypes.c_uint64 if result_type == "i64" else None
        function.library = loaded
        return ir, function, names

    def compare_scalar(self, assembly, cases, expected_names=None):
        _, original, _ = self.lift(assembly)
        ir, recovered, names = self.lift(assembly, "--remove-state", "--remove-memory")
        self.assertRegex(ir, r"define [^\n]*\bi64 @lifted_0x0")
        self.assertNotIn("state", names)
        self.assertNotIn("memory", names)
        self.assertNotIn("alloca", ir)
        if expected_names is not None:
            self.assertEqual(names, expected_names)
        for values in cases:
            state = State()
            memory = (ctypes.c_uint8 * 4096)()
            state.rsp = 2048
            for name, value in values.items():
                setattr(state, name, value)
            original(ctypes.byref(state), memory)
            result = recovered(*(values.get(name, 0) for name in names))
            self.assertEqual(result, state.rax, (values, ir))

    def test_argument_order_and_return(self):
        self.compare_scalar("mov rax, rdi\nadd rax, rsi\nret\n", [
            {"rdi": 0, "rsi": 0}, {"rdi": 12, "rsi": 30},
            {"rdi": (1 << 64) - 1, "rsi": 2}
        ], ["rdi", "rsi"])

    def test_unused_abi_prefix_is_preserved(self):
        self.compare_scalar("mov rax, r9\nret\n", [{"r9": 42}],
                            ["rdi", "rsi", "rdx", "rcx", "r8", "r9"])

    def test_constant_return(self):
        self.compare_scalar("mov eax, 42\nret\n", [{}], [])

    def test_incoming_non_abi_register(self):
        self.compare_scalar("add rax, rdi\nret\n", [
            {"rax": 40, "rdi": 2}, {"rax": (1 << 64) - 1, "rdi": 1}
        ], ["rdi", "rax"])

    def test_partial_register_write(self):
        self.compare_scalar("mov rax, rdi\nmov rcx, rsi\nmov ah, cl\nret\n", [
            {"rdi": 0x8877665544332211, "rsi": 0x80},
            {"rdi": (1 << 64) - 1, "rsi": 0}
        ], ["rdi", "rsi"])

    def test_zero_extension(self):
        self.compare_scalar("mov rax, rdi\nmov eax, esi\nret\n", [
            {"rdi": (1 << 64) - 1, "rsi": 0x80000000},
            {"rdi": 42, "rsi": (1 << 64) - 1}
        ], ["rdi", "rsi"])

    def test_branch_merge(self):
        self.compare_scalar("cmp rdi, 0\nje zero\nmov rax, rsi\njmp done\n"
                            "zero: mov eax, 7\ndone: ret\n", [
            {"rdi": 0, "rsi": 42}, {"rdi": 1, "rsi": 42},
            {"rdi": (1 << 64) - 1, "rsi": 99}
        ], ["rdi", "rsi"])

    def test_loop_merge(self):
        self.compare_scalar("mov eax, 0\ncmp rdi, 0\nje done\n"
                            "again: add rax, rdi\nsub rdi, 1\njne again\ndone: ret\n",
                            [{"rdi": x} for x in [0, 1, 2, 5, 20]], ["rdi"])

    def test_incoming_flag(self):
        self.compare_scalar("je zero\nmov eax, 42\nret\nzero: mov eax, 7\nret\n",
                            [{"zf": 0}, {"zf": 1}, {"zf": 255}], ["zf"])

    def test_native_memory(self):
        assembly = "mov rax, [rdi + 8]\nadd rax, rsi\nmov [rdi], rax\nret\n"
        _, original, _ = self.lift(assembly)
        ir, recovered, names = self.lift(assembly, "--remove-state", "--remove-memory")
        self.assertEqual(names, ["rdi", "rsi"])
        self.assertIn("inttoptr", ir)
        for initial, increment in [(40, 2), ((1 << 64) - 1, 3)]:
            original_memory = (ctypes.c_uint64 * 512)()
            native_memory = (ctypes.c_uint64 * 512)()
            original_memory[9] = native_memory[9] = initial
            state = State(rdi=64, rsi=increment, rsp=2048)
            original(ctypes.byref(state), original_memory)
            result = recovered(ctypes.addressof(native_memory) + 64, increment)
            self.assertEqual(result, state.rax)
            self.assertEqual(native_memory[8], original_memory[8])

    def test_state_remover_independently(self):
        ir, recovered, names = self.lift("mov rax, [rdi]\nret\n", "--remove-state")
        self.assertEqual(names, ["rdi", "memory"])
        self.assertNotIn("%state", ir)
        memory = (ctypes.c_uint64 * 16)()
        memory[3] = 123
        self.assertEqual(recovered(24, memory), 123)

    def test_memory_remover_independently(self):
        ir, recovered, names = self.lift("mov rax, [rdi]\nret\n", "--remove-memory")
        self.assertEqual(names, ["state"])
        self.assertNotIn("%memory", ir)
        value = ctypes.c_uint64(123)
        stack = ctypes.c_uint64(0)
        state = State(rdi=ctypes.addressof(value), rsp=ctypes.addressof(stack))
        recovered(ctypes.byref(state))
        self.assertEqual(state.rax, 123)

    def test_call_and_indirect_jump_keep_state(self):
        for assembly in ["call rdi\nret\n", "jmp rdi\n"]:
            ir, _, names = self.lift(assembly, "--remove-state")
            self.assertEqual(names, ["state", "memory"])
            self.assertIn("define void @lifted_0x0", ir)

    def test_incomplete_lifting_keeps_state(self):
        ir, _, names = self.lift("mov eax, 42\n", "--remove-state")
        self.assertEqual(names, ["state", "memory"])
        self.assertIn("define void @lifted_0x0", ir)

    def test_exhaust_option(self):
        ir, recovered, names = self.lift("mov rax, rdi\nadd rax, rsi\nret\n",
                                         "--remove-state", "--remove-memory", "--exhaust")
        self.assertEqual(names, ["rdi", "rsi"])
        self.assertEqual(recovered(40, 2), 42)
        self.assertNotIn("%state", ir)


if __name__ == "__main__":
    unittest.main()
