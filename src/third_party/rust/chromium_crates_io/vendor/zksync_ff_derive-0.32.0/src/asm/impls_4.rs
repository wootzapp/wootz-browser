use super::super::get_temp_with_literal;

pub(crate) fn mul_impl(mont_inv: u64, modulus_static_prefix: &str) -> proc_macro2::TokenStream {
    let mut gen = proc_macro2::TokenStream::new();

    let m0 = get_temp_with_literal(modulus_static_prefix, 0);
    let m1 = get_temp_with_literal(modulus_static_prefix, 1);
    let m2 = get_temp_with_literal(modulus_static_prefix, 2);
    let m3 = get_temp_with_literal(modulus_static_prefix, 3);

    gen.extend(quote! {
        #[allow(clippy::too_many_lines)]
        #[inline(always)]
        #[cfg(target_arch = "x86_64")]
        // #[cfg(all(target_arch = "x86_64", target_feature = "adx"))]
        fn mont_mul_asm_adx_with_reduction(a: &[u64; 4], b: &[u64; 4]) -> [u64; 4] {
            // this is CIOS multiplication when top bit for top word of modulus is not set

            let mut r0: u64;
            let mut r1: u64;
            let mut r2: u64;
            let mut r3: u64;

            unsafe {
                core::arch::asm!(
                    // round 0
                    "mov rdx, qword ptr [{a_ptr} + 0]",
                    "xor r8d, r8d",
                    "mulx r14, r13, qword ptr [{b_ptr} + 0]",
                    "mulx r9, r8, qword ptr [{b_ptr} + 8]",
                    "mulx r10, r15, qword ptr [{b_ptr} + 16]",
                    "mulx r12, rdi, qword ptr [{b_ptr} + 24]",
                    "mov rdx, r13",
                    "mov r11, {inv}",
                    "mulx r11, rdx, r11",
                    "adcx r14, r8",
                    "adox r10, rdi",
                    "adcx r15, r9",
                    "mov r11, 0",
                    "adox r12, r11",
                    "adcx r10, r11",
                    "mulx r9, r8, qword ptr [rip + {q0_ptr}]",
                    "mulx r11, rdi, qword ptr [rip + {q1_ptr}]",
                    "adox r13, r8",
                    "adcx r14, rdi",
                    "adox r14, r9",
                    "adcx r15, r11",
                    "mulx r9, r8, qword ptr [rip + {q2_ptr}]",
                    "mulx r11, rdi, qword ptr [rip + {q3_ptr}]",
                    "adox r15, r8",
                    "adcx r10, rdi",
                    "adox r10, r9",
                    "adcx r12, r11",
                    "mov r9, 0",
                    "adox r12, r9",

                    // round 1
                    "mov rdx, qword ptr [{a_ptr} + 8]",
                    "mulx r9, r8, qword ptr [{b_ptr} + 0]",
                    "mulx r11, rdi, qword ptr [{b_ptr} + 8]",
                    "adcx r14, r8",
                    "adox r15, r9",
                    "mulx r9, r8, qword ptr [{b_ptr} + 16]",
                    "adcx r15, rdi",
                    "adox r10, r11",
                    "mulx r13, rdi, qword ptr [{b_ptr} + 24]",
                    "adcx r10, r8",
                    "adox r12, rdi",
                    "adcx r12, r9",
                    "mov rdi, 0",
                    "adox r13, rdi",
                    "adcx r13, rdi",
                    "mov rdx, r14",
                    "mov r8, {inv}",
                    "mulx r8, rdx, r8",
                    "mulx r9, r8, qword ptr [rip + {q0_ptr}]",
                    "mulx r11, rdi, qword ptr [rip + {q1_ptr}]",
                    "adox r14, r8",
                    "adcx r15, rdi",
                    "adox r15, r9",
                    "adcx r10, r11",
                    "mulx r9, r8, qword ptr [rip + {q2_ptr}]",
                    "mulx r11, rdi, qword ptr [rip + {q3_ptr}]",
                    "adox r10, r8",
                    "adcx r12, r9",
                    "adox r12, rdi",
                    "adcx r13, r11",
                    "mov rdi, 0",
                    "adox r13, rdi",

                    // round 2
                    "mov rdx, qword ptr [{a_ptr} + 16]",
                    "mulx r9, r8, qword ptr [{b_ptr} + 0]",
                    "mulx r11, rdi, qword ptr [{b_ptr} + 8]",
                    "adcx r15, r8",
                    "adox r10, r9",
                    "mulx r9, r8, qword ptr [{b_ptr} + 16]",
                    "adcx r10, rdi",
                    "adox r12, r11",
                    "mulx r14, rdi, qword ptr [{b_ptr} + 24]",
                    "adcx r12, r8",
                    "adox r13, r9",
                    "adcx r13, rdi",
                    "mov r9, 0",
                    "adox r14, r9",
                    "adcx r14, r9",
                    "mov rdx, r15",
                    "mov r8, {inv}",
                    "mulx r8, rdx, r8",
                    "mulx r9, r8, qword ptr [rip + {q0_ptr}]",
                    "mulx r11, rdi, qword ptr [rip + {q1_ptr}]",
                    "adox r15, r8",
                    "adcx r10, r9",
                    "adox r10, rdi",
                    "adcx r12, r11",
                    "mulx r9, r8, qword ptr [rip + {q2_ptr}]",
                    "mulx r11, rdi, qword ptr [rip + {q3_ptr}]",
                    "adox r12, r8",
                    "adcx r13, r9",
                    "adox r13, rdi",
                    "adcx r14, r11",
                    "mov rdi, 0",
                    "adox r14, rdi",

                    // round 3
                    "mov rdx, qword ptr [{a_ptr} + 24]",
                    "mulx r9, r8, qword ptr [{b_ptr} + 0]",
                    "mulx r11, rdi, qword ptr [{b_ptr} + 8]",
                    "adcx r10, r8",
                    "adox r12, r9",
                    "mulx r9, r8, qword ptr [{b_ptr} + 16]",
                    "adcx r12, rdi",
                    "adox r13, r11",
                    "mulx r15, rdi, qword ptr [{b_ptr} + 24]",
                    "adcx r13, r8",
                    "adox r14, r9",
                    "adcx r14, rdi",
                    "mov r9, 0",
                    "adox r15, r9",
                    "adcx r15, r9",
                    "mov rdx, r10",
                    "mov r8, {inv}",
                    "mulx r8, rdx, r8",
                    "mulx r9, r8, qword ptr [rip + {q0_ptr}]",
                    "mulx r11, rdi, qword ptr [rip + {q1_ptr}]",
                    "adox r10, r8",
                    "adcx r12, r9",
                    "adox r12, rdi",
                    "adcx r13, r11",
                    "mulx r9, r8, qword ptr [rip + {q2_ptr}]",
                    "mulx rdx, rdi, qword ptr [rip + {q3_ptr}]",
                    "adox r13, r8",
                    "adcx r14, r9",
                    "adox r14, rdi",
                    "adcx r15, rdx",
                    "mov rdi, 0",
                    "adox r15, rdi",
                    // reduction. We use sub/sbb

                    "mov r8, r12",
                    "mov rdx, qword ptr [rip + {q0_ptr}]",
                    "sub r8, rdx",
                    "mov r9, r13",
                    "mov rdx, qword ptr [rip + {q1_ptr}]",
                    "sbb r9, rdx",
                    "mov r10, r14",
                    "mov rdx, qword ptr [rip + {q2_ptr}]",
                    "sbb r10, rdx",
                    "mov r11, r15",
                    "mov rdx, qword ptr [rip + {q3_ptr}]",
                    "sbb r11, rdx",

                    // if CF == 1 then original result was ok (reduction wa not necessary)
                    // so if not carry (CMOVNQ) then we copy
                    "cmovnc r12, r8",
                    "cmovnc r13, r9",
                    "cmovnc r14, r10",
                    "cmovnc r15, r11",
                    // end of reduction
                    q0_ptr = sym #m0,
                    q1_ptr = sym #m1,
                    q2_ptr = sym #m2,
                    q3_ptr = sym #m3,
                    inv = const #mont_inv,
                    a_ptr = in(reg) a.as_ptr(),
                    b_ptr = in(reg) b.as_ptr(),
                    out("rdx") _,
                    out("rdi") _,
                    out("r8") _,
                    out("r9") _,
                    out("r10") _,
                    out("r11") _,
                    out("r12") r0,
                    out("r13") r1,
                    out("r14") r2,
                    out("r15") r3,
                    options(pure, readonly, nostack)
                );
            }

            [r0, r1, r2, r3]
        }

    });

    gen
}

pub(crate) fn sqr_impl(mont_inv: u64, modulus_static_prefix: &str) -> proc_macro2::TokenStream {
    let mut gen = proc_macro2::TokenStream::new();

    let m0 = get_temp_with_literal(modulus_static_prefix, 0);
    let m1 = get_temp_with_literal(modulus_static_prefix, 1);
    let m2 = get_temp_with_literal(modulus_static_prefix, 2);
    let m3 = get_temp_with_literal(modulus_static_prefix, 3);

    gen.extend(quote! {
        #[allow(clippy::too_many_lines)]
        #[inline(always)]
        #[cfg(target_arch = "x86_64")]
        // #[cfg(all(target_arch = "x86_64", target_feature = "adx"))]
        fn mont_sqr_asm_adx_with_reduction(a: &[u64; 4]) -> [u64; 4] {
            // this is CIOS multiplication when top bit for top word of modulus is not set

            let mut r0: u64;
            let mut r1: u64;
            let mut r2: u64;
            let mut r3: u64;

            unsafe {
                core::arch::asm!(
                    // round 0
                    "mov rdx, qword ptr [{a_ptr} + 0]",
                    "xor r8d, r8d",
                    "mulx r10, r9, qword ptr [{a_ptr} + 8]",
                    "mulx r15, r8, qword ptr [{a_ptr} + 16]",
                    "mulx r12, r11, qword ptr [{a_ptr} + 24]",
                    "adox r10, r8",
                    "adcx r11, r15",
                    "mov rdx, qword ptr [{a_ptr} + 8]",
                    "mulx r15, r8, qword ptr [{a_ptr} + 16]",
                    "mulx rcx, rdi, qword ptr [{a_ptr} + 24]",
                    "mov rdx, qword ptr [{a_ptr} + 16]",
                    "mulx r14, r13, qword ptr [{a_ptr} + 24]",
                    "adox r11, r8",
                    "adcx r12, rdi",
                    "adox r12, r15",
                    "adcx r13, rcx",
                    "mov r8, 0",
                    "adox r13, r8",
                    "adcx r14, r8",

                    // double
                    "adox r9, r9",
                    "adcx r12, r12",
                    "adox r10, r10",
                    "adcx r13, r13",
                    "adox r11, r11",
                    "adcx r14, r14",

                    // square contributions
                    "mov rdx, qword ptr [{a_ptr} + 0]",
                    "mulx rcx, r8, rdx",
                    "mov rdx, qword ptr [{a_ptr} + 16]",
                    "mulx rdi, rdx, rdx",
                    "adox r12, rdx",
                    "adcx r9, rcx",
                    "adox r13, rdi",
                    "mov rdx, qword ptr [{a_ptr} + 24]",
                    "mulx r15, rcx, rdx",
                    "mov rdx, qword ptr [{a_ptr} + 8]",
                    "mulx rdx, rdi, rdx",
                    "adcx r10, rdi",
                    "adox r14, rcx",
                    "mov rdi, 0",
                    "adcx r11, rdx",
                    "adox r15, rdi",

                    // reduction round 0
                    "mov rdx, r8",
                    "mov rdi, {inv}",
                    "mulx rdi, rdx, rdi",
                    "mulx rcx, rdi, qword ptr [rip + {q0_ptr}]",
                    "adox r8, rdi",
                    "mulx rdi, r8, qword ptr [rip + {q3_ptr}]",
                    "adcx r12, rdi",
                    "adox r9, rcx",
                    "mov rdi, 0",
                    "adcx r13, rdi",
                    "mulx rcx, rdi, qword ptr [rip + {q1_ptr}]",
                    "adox r10, rcx",
                    "adcx r9, rdi",
                    "adox r11, r8",
                    "mulx rcx, rdi, qword ptr [rip + {q2_ptr}]",
                    "adcx r10, rdi",
                    "adcx r11, rcx",

                    // reduction round 1
                    "mov rdx, r9",
                    "mov rdi, {inv}",
                    "mulx rdi, rdx, rdi",
                    "mulx rcx, rdi, qword ptr [rip + {q2_ptr}]",
                    "adox r12, rcx",
                    "mulx rcx, r8, qword ptr [rip + {q3_ptr}]",
                    "adcx r12, r8",
                    "adox r13, rcx",
                    "mov r8, 0",
                    "adcx r13, r8",
                    "adox r14, r8",
                    "mulx rcx, r8, qword ptr [rip + {q0_ptr}]",
                    "adcx r9, r8",
                    "adox r10, rcx",
                    "mulx rcx, r8, qword ptr [rip + {q1_ptr}]",
                    "adcx r10, r8",
                    "adox r11, rcx",
                    "adcx r11, rdi",

                    // reduction round 2
                    "mov rdx, r10",
                    "mov rdi, {inv}",
                    "mulx rdi, rdx, rdi",
                    "mulx rcx, rdi, qword ptr [rip + {q1_ptr}]",
                    "mulx r9, r8, qword ptr [rip + {q2_ptr}]",
                    "adox r12, rcx",
                    "adcx r12, r8",
                    "adox r13, r9",
                    "mulx r9, r8, qword ptr [rip + {q3_ptr}]",
                    "adcx r13, r8",
                    "adox r14, r9",
                    "mov r8, 0",
                    "adcx r14, r8",
                    "adox r15, r8",
                    "mulx r9, r8, qword ptr [rip + {q0_ptr}]",
                    "adcx r10, r8",
                    "adox r11, r9",
                    "mov r8, 0",
                    "adcx r11, rdi",
                    "adox r12, r8",

                    // reduction round 3
                    "mov rdx, r11",
                    "mov rdi, {inv}",
                    "mulx rdi, rdx, rdi",
                    "mulx rcx, rdi, qword ptr [rip + {q0_ptr}]",
                    "mulx r9, r8, qword ptr [rip + {q1_ptr}]",
                    "adox r11, rdi",
                    "adcx r12, r8",
                    "adox r12, rcx",
                    "adcx r13, r9",
                    "mulx r9, r8, qword ptr [rip + {q2_ptr}]",
                    "mulx r11, r10, qword ptr [rip + {q3_ptr}]",
                    "adox r13, r8",
                    "adcx r14, r10",
                    "mov r8, 0",
                    "adox r14, r9",
                    "adcx r15, r11",
                    "adox r15, r8",

                    // reduction. We use sub/sbb
                    "mov r8, r12",
                    "mov rdx, qword ptr [rip + {q0_ptr}]",
                    "sub r8, rdx",
                    "mov r9, r13",
                    "mov rdx, qword ptr [rip + {q1_ptr}]",
                    "sbb r9, rdx",
                    "mov r10, r14",
                    "mov rdx, qword ptr [rip + {q2_ptr}]",
                    "sbb r10, rdx",
                    "mov r11, r15",
                    "mov rdx, qword ptr [rip + {q3_ptr}]",
                    "sbb r11, rdx",

                    // if CF == 1 then original result was ok (reduction wa not necessary)
                    // so if not carry (CMOVNQ) then we copy
                    "cmovnc r12, r8",
                    "cmovnc r13, r9",
                    "cmovnc r14, r10",
                    "cmovnc r15, r11",
                    // end of reduction
                    q0_ptr = sym #m0,
                    q1_ptr = sym #m1,
                    q2_ptr = sym #m2,
                    q3_ptr = sym #m3,
                    inv = const #mont_inv,
                    a_ptr = in(reg) a.as_ptr(),
                    out("rcx") _,
                    out("rdx") _,
                    out("rdi") _,
                    out("r8") _,
                    out("r9") _,
                    out("r10") _,
                    out("r11") _,
                    out("r12") r0,
                    out("r13") r1,
                    out("r14") r2,
                    out("r15") r3,
                    options(pure, readonly, nostack)
                );
            }

            [r0, r1, r2, r3]
        }

    });

    gen
}

pub(crate) fn add_impl(modulus_static_prefix: &str) -> proc_macro2::TokenStream {
    let mut gen = proc_macro2::TokenStream::new();

    let m0 = get_temp_with_literal(modulus_static_prefix, 0);
    let m1 = get_temp_with_literal(modulus_static_prefix, 1);
    let m2 = get_temp_with_literal(modulus_static_prefix, 2);
    let m3 = get_temp_with_literal(modulus_static_prefix, 3);

    gen.extend(quote! {
        #[allow(clippy::too_many_lines)]
        #[inline(always)]
        #[cfg(target_arch = "x86_64")]
        // #[cfg(all(target_arch = "x86_64", target_feature = "adx"))]
        fn add_asm_adx_with_reduction(a: &[u64; 4], b: &[u64; 4]) -> [u64; 4] {
            let mut r0: u64;
            let mut r1: u64;
            let mut r2: u64;
            let mut r3: u64;

            unsafe {
                core::arch::asm!(
                    // we sum (a+b) using addition chain with OF
                    // and sum (a+b) - p using addition chain with CF
                    // if (a+b) does not overflow the modulus
                    // then sum (a+b) will produce CF
                    "xor r12d, r12d",
                    "mov r12, qword ptr [{a_ptr} + 0]",
                    "mov r13, qword ptr [{a_ptr} + 8]",
                    "mov r14, qword ptr [{a_ptr} + 16]",
                    "mov r15, qword ptr [{a_ptr} + 24]",
                    "adox r12, qword ptr [{b_ptr} + 0]",
                    "mov r8, r12",
                    "adcx r8, qword ptr [rip + {q0_ptr}]",
                    "adox r13, qword ptr [{b_ptr} + 8]",
                    "mov r9, r13",
                    "adcx r9, qword ptr [rip + {q1_ptr}]",
                    "adox r14, qword ptr [{b_ptr} + 16]",
                    "mov r10, r14",
                    "adcx r10, qword ptr [rip + {q2_ptr}]",
                    "adox r15, qword ptr [{b_ptr} + 24]",
                    "mov r11, r15",
                    "adcx r11, qword ptr [rip + {q3_ptr}]",

                    // if CF = 0 then take value (a+b) from [r12, .., r15]
                    // otherwise take (a+b) - p

                    "cmovc r12, r8",
                    "cmovc r13, r9",
                    "cmovc r14, r10",
                    "cmovc r15, r11",

                    q0_ptr = sym #m0,
                    q1_ptr = sym #m1,
                    q2_ptr = sym #m2,
                    q3_ptr = sym #m3,
                    // end of reduction
                    a_ptr = in(reg) a.as_ptr(),
                    b_ptr = in(reg) b.as_ptr(),
                    out("r8") _,
                    out("r9") _,
                    out("r10") _,
                    out("r11") _,
                    out("r12") r0,
                    out("r13") r1,
                    out("r14") r2,
                    out("r15") r3,
                    options(pure, readonly, nostack)
                );
            }

            // unsafe {
            //     core::arch::asm!(
            //         "xor r12d, r12d",
            //         "mov r12, qword ptr [{a_ptr} + 0]",
            //         "mov r13, qword ptr [{a_ptr} + 8]",
            //         "mov r14, qword ptr [{a_ptr} + 16]",
            //         "mov r15, qword ptr [{a_ptr} + 24]",
            //         "add r12, qword ptr [{b_ptr} + 0]",
            //         "adc r13, qword ptr [{b_ptr} + 8]",
            //         "adc r14, qword ptr [{b_ptr} + 16]",
            //         "adc r15, qword ptr [{b_ptr} + 24]",

            //         "mov r8, r12",
            //         "mov rdx, qword ptr [rip + {q0_ptr}]",
            //         "sub r8, rdx",
            //         "mov r9, r13",
            //         "mov rdx, qword ptr [rip + {q1_ptr}]",
            //         "sbb r9, rdx",
            //         "mov r10, r14",
            //         "mov rdx, qword ptr [rip + {q2_ptr}]",
            //         "sbb r10, rdx",
            //         "mov r11, r15",
            //         "mov rdx, qword ptr [rip + {q3_ptr}]",
            //         "sbb r11, rdx",

            //         "cmovnc r12, r8",
            //         "cmovnc r13, r9",
            //         "cmovnc r14, r10",
            //         "cmovnc r15, r11",

            //         q0_ptr = sym #m0,
            //         q1_ptr = sym #m1,
            //         q2_ptr = sym #m2,
            //         q3_ptr = sym #m3,
            //         // end of reduction
            //         a_ptr = in(reg) a.as_ptr(),
            //         b_ptr = in(reg) b.as_ptr(),
            //         out("rdx") _,
            //         out("r8") _,
            //         out("r9") _,
            //         out("r10") _,
            //         out("r11") _,
            //         out("r12") r0,
            //         out("r13") r1,
            //         out("r14") r2,
            //         out("r15") r3,
            //         options(pure, readonly, nostack)
            //     );
            // }

            [r0, r1, r2, r3]
        }
    });

    gen
}

pub(crate) fn double_impl(modulus_static_prefix: &str) -> proc_macro2::TokenStream {
    let mut gen = proc_macro2::TokenStream::new();

    let m0 = get_temp_with_literal(modulus_static_prefix, 0);
    let m1 = get_temp_with_literal(modulus_static_prefix, 1);
    let m2 = get_temp_with_literal(modulus_static_prefix, 2);
    let m3 = get_temp_with_literal(modulus_static_prefix, 3);

    gen.extend(quote! {
        #[allow(clippy::too_many_lines)]
        #[inline(always)]
        #[cfg(target_arch = "x86_64")]
        // #[cfg(all(target_arch = "x86_64", target_feature = "adx"))]
        fn double_asm_adx_with_reduction(a: &[u64; 4]) -> [u64; 4] {
            let mut r0: u64;
            let mut r1: u64;
            let mut r2: u64;
            let mut r3: u64;

            unsafe {
                core::arch::asm!(
                    // we sum (a+b) using addition chain with OF
                    // and sum (a+b) - p using addition chain with CF
                    // if (a+b) does not overflow the modulus
                    // then sum (a+b) will produce CF
                    "xor r12d, r12d",
                    "mov r12, qword ptr [{a_ptr} + 0]",
                    "adox r12, r12",
                    "mov r13, qword ptr [{a_ptr} + 8]",
                    "adox r13, r13",
                    "mov r14, qword ptr [{a_ptr} + 16]",
                    "adox r14, r14",
                    "mov r15, qword ptr [{a_ptr} + 24]",
                    "adox r15, r15",

                    "mov r8, r12",
                    "adcx r8, qword ptr [rip + {q0_ptr}]",
                    "mov r9, r13",
                    "adcx r9, qword ptr [rip + {q1_ptr}]",
                    "mov r10, r14",
                    "adcx r10, qword ptr [rip + {q2_ptr}]",
                    "mov r11, r15",
                    "adcx r11, qword ptr [rip + {q3_ptr}]",

                    // if CF = 0 then take value (a+b) from [r12, .., r15]
                    // otherwise take (a+b) - p

                    "cmovc r12, r8",
                    "cmovc r13, r9",
                    "cmovc r14, r10",
                    "cmovc r15, r11",

                    q0_ptr = sym #m0,
                    q1_ptr = sym #m1,
                    q2_ptr = sym #m2,
                    q3_ptr = sym #m3,
                    // end of reduction
                    a_ptr = in(reg) a.as_ptr(),
                    out("r8") _,
                    out("r9") _,
                    out("r10") _,
                    out("r11") _,
                    out("r12") r0,
                    out("r13") r1,
                    out("r14") r2,
                    out("r15") r3,
                    options(pure, readonly, nostack)
                );
            }

            // unsafe {
            //     core::arch::asm!(
            //         "xor r12d, r12d",
            //         "mov r12, qword ptr [{a_ptr} + 0]",
            //         "mov r13, qword ptr [{a_ptr} + 8]",
            //         "mov r14, qword ptr [{a_ptr} + 16]",
            //         "mov r15, qword ptr [{a_ptr} + 24]",
            //         "add r12, r12",
            //         "adc r13, r13",
            //         "adc r14, r14",
            //         "adc r15, r15",

            //         "mov r8, r12",
            //         "mov rdx, qword ptr [rip + {q0_ptr}]",
            //         "sub r8, rdx",
            //         "mov r9, r13",
            //         "mov rdx, qword ptr [rip + {q1_ptr}]",
            //         "sbb r9, rdx",
            //         "mov r10, r14",
            //         "mov rdx, qword ptr [rip + {q2_ptr}]",
            //         "sbb r10, rdx",
            //         "mov r11, r15",
            //         "mov rdx, qword ptr [rip + {q3_ptr}]",
            //         "sbb r11, rdx",

            //         "cmovnc r12, r8",
            //         "cmovnc r13, r9",
            //         "cmovnc r14, r10",
            //         "cmovnc r15, r11",

            //         q0_ptr = sym #m0,
            //         q1_ptr = sym #m1,
            //         q2_ptr = sym #m2,
            //         q3_ptr = sym #m3,
            //         // end of reduction
            //         a_ptr = in(reg) a.as_ptr(),
            //         out("rdx") _,
            //         out("r8") _,
            //         out("r9") _,
            //         out("r10") _,
            //         out("r11") _,
            //         out("r12") r0,
            //         out("r13") r1,
            //         out("r14") r2,
            //         out("r15") r3,
            //         options(pure, readonly, nostack)
            //     );
            // }

            [r0, r1, r2, r3]
        }
    });

    gen
}

pub(crate) fn sub_impl(modulus_static_prefix: &str) -> proc_macro2::TokenStream {
    let mut gen = proc_macro2::TokenStream::new();

    let m0 = get_temp_with_literal(modulus_static_prefix, 0);
    let m1 = get_temp_with_literal(modulus_static_prefix, 1);
    let m2 = get_temp_with_literal(modulus_static_prefix, 2);
    let m3 = get_temp_with_literal(modulus_static_prefix, 3);

    gen.extend(quote! {
        #[allow(clippy::too_many_lines)]
        #[inline(always)]
        #[cfg(target_arch = "x86_64")]
        // #[cfg(all(target_arch = "x86_64", target_feature = "adx"))]
        fn sub_asm_adx_with_reduction(a: &[u64; 4], b: &[u64; 4]) -> [u64; 4] {
            let mut r0: u64;
            let mut r1: u64;
            let mut r2: u64;
            let mut r3: u64;

            unsafe {
                core::arch::asm!(
                    "xor r12d, r12d",
                    "mov r12, qword ptr [{a_ptr} + 0]",
                    "sub r12, qword ptr [{b_ptr} + 0]",
                    "mov r13, qword ptr [{a_ptr} + 8]",
                    "sbb r13, qword ptr [{b_ptr} + 8]",
                    "mov r14, qword ptr [{a_ptr} + 16]",
                    "sbb r14, qword ptr [{b_ptr} + 16]",
                    "mov r15, qword ptr [{a_ptr} + 24]",
                    "sbb r15, qword ptr [{b_ptr} + 24]",

                    // duplicate (a-b) into [r8, r9, r10, r11]

                    // now make [r12, .., r15] + modulus;
                    // if (a-b) did underflow then 2^256 + (a-b) < modulus,
                    // so below we will get an overflow

                    "mov r8, r12",
                    "add r12, qword ptr [rip + {q0_ptr}]",
                    // "adox r12, qword ptr [rip + {q0_ptr}]",
                    "mov r9, r13",
                    "adc r13, qword ptr [rip + {q1_ptr}]",
                    // "adox r13, qword ptr [rip + {q1_ptr}]",
                    "mov r10, r14",
                    "adc r14, qword ptr [rip + {q2_ptr}]",
                    // "adox r14, qword ptr [rip + {q2_ptr}]",
                    "mov r11, r15",
                    "adc r15, qword ptr [rip + {q3_ptr}]",
                    // "adox r15, qword ptr [rip + {q3_ptr}]",

                    "cmovnc r12, r8",
                    "cmovnc r13, r9",
                    "cmovnc r14, r10",
                    "cmovnc r15, r11",

                    q0_ptr = sym #m0,
                    q1_ptr = sym #m1,
                    q2_ptr = sym #m2,
                    q3_ptr = sym #m3,
                    // end of reduction
                    a_ptr = in(reg) a.as_ptr(),
                    b_ptr = in(reg) b.as_ptr(),
                    out("r8") _,
                    out("r9") _,
                    out("r10") _,
                    out("r11") _,
                    out("r12") r0,
                    out("r13") r1,
                    out("r14") r2,
                    out("r15") r3,
                    options(pure, readonly, nostack)
                );
            }

            [r0, r1, r2, r3]
        }
    });

    gen
}
