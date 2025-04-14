cfg_if! {
    if #[cfg(
        all(
            any(target_arch = "x86", target_arch = "x86_64"),
            any(target_feature = "sse", target_feature = "sse4.1", target_feature = "avx", target_feature = "avx2")
        )
    )] {
        #[inline(always)]
        pub(crate) fn prefetch_l1_pointer<T: Sized>(pointer: *const T) {
            #[cfg(target_arch = "x86")]
            use std::arch::x86::_mm_prefetch;
            #[cfg(target_arch = "x86_64")]
            use std::arch::x86_64::_mm_prefetch;


            unsafe {
                #[cfg(target_arch = "x86_64")]
                _mm_prefetch(pointer as *const i8, core::arch::x86_64::_MM_HINT_T0);
                #[cfg(target_arch = "x86")]
                _mm_prefetch(pointer as *const i8, core::arch::x86::_MM_HINT_T0);
            }
        }

        #[inline(always)]
        pub(crate) fn prefetch_l2_pointer<T: Sized>(pointer: *const T) {
            #[cfg(target_arch = "x86")]
            use std::arch::x86::_mm_prefetch;
            #[cfg(target_arch = "x86_64")]
            use std::arch::x86_64::_mm_prefetch;


            unsafe {
                #[cfg(target_arch = "x86_64")]
                _mm_prefetch(pointer as *const i8, core::arch::x86_64::_MM_HINT_T1);
                #[cfg(target_arch = "x86")]
                _mm_prefetch(pointer as *const i8, core::arch::x86::_MM_HINT_T1);
            }
        }

        #[inline(always)]
        pub(crate) fn prefetch_l3_pointer<T: Sized>(pointer: *const T) {
            #[cfg(target_arch = "x86")]
            use std::arch::x86::_mm_prefetch;
            #[cfg(target_arch = "x86_64")]
            use std::arch::x86_64::_mm_prefetch;


            unsafe {
                #[cfg(target_arch = "x86_64")]
                _mm_prefetch(pointer as *const i8, core::arch::x86_64::_MM_HINT_T2);
                #[cfg(target_arch = "x86")]
                _mm_prefetch(pointer as *const i8, core::arch::x86::_MM_HINT_T2);
            }
        }

        #[inline(always)]
        pub(crate) fn prefetch_l1<T: Sized>(reference: &T) {
            #[cfg(target_arch = "x86")]
            use std::arch::x86::_mm_prefetch;
            #[cfg(target_arch = "x86_64")]
            use std::arch::x86_64::_mm_prefetch;


            unsafe {
                #[cfg(target_arch = "x86_64")]
                _mm_prefetch(reference as *const T as *const i8, core::arch::x86_64::_MM_HINT_T0);
                #[cfg(target_arch = "x86")]
                _mm_prefetch(reference as *const T as *const i8, core::arch::x86::_MM_HINT_T0);
            }
        }

        #[inline(always)]
        pub(crate) fn prefetch_l2<T: Sized>(reference: &T) {
            #[cfg(target_arch = "x86")]
            use std::arch::x86::_mm_prefetch;
            #[cfg(target_arch = "x86_64")]
            use std::arch::x86_64::_mm_prefetch;


            unsafe {
                #[cfg(target_arch = "x86_64")]
                _mm_prefetch(reference as *const T as *const i8, core::arch::x86_64::_MM_HINT_T1);
                #[cfg(target_arch = "x86")]
                _mm_prefetch(reference as *const T as *const i8, core::arch::x86::_MM_HINT_T1);
            }
        }

        #[inline(always)]
        pub(crate) fn prefetch_l3<T: Sized>(reference: &T) {
            #[cfg(target_arch = "x86")]
            use std::arch::x86::_mm_prefetch;
            #[cfg(target_arch = "x86_64")]
            use std::arch::x86_64::_mm_prefetch;


            unsafe {
                #[cfg(target_arch = "x86_64")]
                _mm_prefetch(reference as *const T as *const i8, core::arch::x86_64::_MM_HINT_T2);
                #[cfg(target_arch = "x86")]
                _mm_prefetch(reference as *const T as *const i8, core::arch::x86::_MM_HINT_T2);
            }
        }
    } else {
        #[inline(always)]
        pub(crate) fn prefetch_l1_pointer<T: Sized>(pointer: *const T) {}

        #[inline(always)]
        pub(crate) fn prefetch_l2_pointer<T: Sized>(pointer: *const T) {}

        #[inline(always)]
        pub(crate) fn prefetch_l3_pointer<T: Sized>(pointer: *const T) {}

        #[inline(always)]
        pub(crate) fn prefetch_l1<T: Sized>(reference: &T) {}

        #[inline(always)]
        pub(crate) fn prefetch_l2<T: Sized>(reference: &T) {}

        #[inline(always)]
        pub(crate) fn prefetch_l3<T: Sized>(reference: &T) {}
    }
}
